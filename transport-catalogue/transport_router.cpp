#include "transport_router.h"

namespace transport_router {

std::optional<TransportRoute> TransportRouter::BuildRoute(const std::string& from, const std::string& to) const {
	if (from == to) {
		return TransportRoute{};
	}
	
	const auto route = router_->BuildRoute(stop_id_by_name_.at(from), stop_id_by_name_.at(to));
		
	if (!route.has_value()) {
		return std::nullopt;
	}
	TransportRoute res;
	res.total_time = route->weight.total_time;

	for (const auto& id : route->edges) {
		const auto& edge = graph_.GetEdge(id);		
		res.route.push_back(edge.weight);
	}

	return res;
}

void TransportRouter::InitializeRouterWithCatalogue(const transport_catalogue::TransportCatalogue& catalogue) {
	BuildGraphBasedOnCatalogue(catalogue);

	router_ = std::make_unique<Router>(graph_, true);
}

void TransportRouter::BuildGraphBasedOnCatalogue(const transport_catalogue::TransportCatalogue& catalogue) {
	graph::DirectedWeightedGraph<EdgeWeight> graph(catalogue.GetAllStops().size());
	graph_ = std::move(graph);

	for (const auto& [name, bus] : catalogue.GetAllBuses()) {
		int stops_cnt = static_cast<int>(bus->route.size());
		for (int from = 0; from < stops_cnt; ++from) {
			double route_time_forward = settings_.bus_wait_time;
			double route_time_backward = settings_.bus_wait_time;
			for (int to = from + 1; to < stops_cnt; ++to) {
				route_time_forward += catalogue.GetDistance(bus->route[to-1]->name, bus->route[to]->name) / settings_.bus_velocity;
				BuildEdge(EdgeWeight{
					name,
					bus->route[from]->name,
					bus->route[to]->name,
					route_time_forward,
					to - from
					});

				if (!bus->ring_route) {
					route_time_backward += catalogue.GetDistance(bus->route[to]->name, bus->route[to-1]->name) / settings_.bus_velocity;
					BuildEdge(EdgeWeight{
						name,
						bus->route[to]->name,
						bus->route[from]->name,
						route_time_backward,
						to - from
						});
				}
			}
		}
	}
}

void TransportRouter::BuildEdge(EdgeWeight edge) {
	auto id_from = AssignStopId(edge.from);
	auto id_to = AssignStopId(edge.to);

	graph_.AddEdge(graph::Edge<EdgeWeight>{id_from, id_to, std::move(edge)});
}

graph::VertexId TransportRouter::AssignStopId(std::string_view stop) {
	auto stop_id = stop_id_by_name_.size();
	if (!stop_id_by_name_.count(stop)) {
		stop_id_by_name_[stop] = stop_id;
	}
	return stop_id_by_name_.at(stop);
}

EdgeWeight operator+(const EdgeWeight& lhs, const EdgeWeight& rhs) {
	EdgeWeight tmp;
	tmp.total_time = lhs.total_time + rhs.total_time;
	return tmp;
}

bool operator<(const EdgeWeight& lhs, const EdgeWeight& rhs) {
	return lhs.total_time < rhs.total_time;
}

bool operator>(const EdgeWeight& lhs, const EdgeWeight& rhs) {
	return rhs < lhs;
}

void TransportRouter::SetRouterSettings(RouterSettings settings) {
	settings_ = settings;
}

const RouterSettings& TransportRouter::GetRouterSettings() const {
	return settings_;
}

TransportRouter::Graph& TransportRouter::GetGraph() {
	return graph_;
}

std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() {
	return router_;
}

std::unordered_map<std::string_view, graph::VertexId>& TransportRouter::GetStopsIdByName() {
	return stop_id_by_name_;
}


} // namespace transport_router