#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <unordered_map>

namespace transport_router {

struct RouterSettings {
	int bus_wait_time = 0;
	double bus_velocity = 0.0;
};

struct EdgeWeight {
	std::string_view bus_name;
	std::string_view from;
	std::string_view to;
	double total_time;
	int span_count = 0;
};

EdgeWeight operator+(const EdgeWeight& lhs, const EdgeWeight& rhs);
bool operator<(const EdgeWeight& lhs, const EdgeWeight& rhs);
bool operator>(const EdgeWeight& lhs, const EdgeWeight& rhs);

struct TransportRoute {
	double total_time;
	std::vector<EdgeWeight> route;
};

class TransportRouter {
public:
	
	using Graph = graph::DirectedWeightedGraph<EdgeWeight>;
	using Router = graph::Router<EdgeWeight>;

	TransportRouter(const RouterSettings settings = {}) : settings_(settings) {}

	void InitializeRouterWithCatalogue(const transport_catalogue::TransportCatalogue& catalogue);

	std::optional<TransportRoute> BuildRoute(const std::string &from, const std::string &to) const;
	
	void SetRouterSettings(RouterSettings settings);

	const RouterSettings& GetRouterSettings() const;

	Graph& GetGraph();

	std::unique_ptr<Router>& GetRouter();

	std::unordered_map<std::string_view, graph::VertexId>& GetStopsIdByName();

private:
	RouterSettings settings_ = {};
		
	Graph graph_;
	std::unique_ptr<Router> router_;

	std::unordered_map<std::string_view, graph::VertexId> stop_id_by_name_;

	void BuildGraphBasedOnCatalogue(const transport_catalogue::TransportCatalogue& catalogue);

	void BuildEdge(EdgeWeight edge);

	graph::VertexId AssignStopId(std::string_view stop);
};

} // namespace transport_router