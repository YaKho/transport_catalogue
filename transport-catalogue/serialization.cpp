#include <fstream>

#include "serialization.h"

namespace serializer {

void Serializer::Serialize() {
	std::ofstream ofs(settings_.path, std::ios::binary);
	if (!ofs.is_open()) {
		return;
	}
	ProtoCatalogue proto_catalogue;

	SerializeStops(proto_catalogue);
	SerializeDistances(proto_catalogue);
	SerializeBuses(proto_catalogue);

	SerializeRenderSettings(proto_catalogue);

	SerializeTransportRouter(proto_catalogue);

	proto_catalogue.SerializeToOstream(&ofs);
}

void Serializer::Deserialize() {
	std::ifstream ifs(settings_.path, std::ios::binary);
	if (!ifs.is_open()) {
		return;
	}
	ProtoCatalogue proto_catalogue;

	proto_catalogue.ParseFromIstream(&ifs);

	DeserializeStops(proto_catalogue);
	DeserializeDistances(proto_catalogue);
	DeserializeBuses(proto_catalogue);

	DeserializeRenderSettings(proto_catalogue);

	DeserializeTransportRouter(proto_catalogue);
}

void Serializer::SerializeStops(ProtoCatalogue& proto_catalogue) {
	for (const auto& [name, stop] : catalogue_.GetAllStops()) {
		proto_transport_catalogue::Stop proto_stop;
		proto_stop.set_name(stop->name);
		proto_stop.mutable_coordinates()->set_lat(stop->coordinates.lat);
		proto_stop.mutable_coordinates()->set_lng(stop->coordinates.lng);

		*proto_catalogue.add_stops() = proto_stop;
	}
}

void Serializer::SerializeDistances(ProtoCatalogue& proto_catalogue) {
	for (const auto& [dir, dist] : catalogue_.GetAllDistances()) {
		proto_transport_catalogue::Distance proto_dist;
		proto_dist.set_from(dir.first->name);
		proto_dist.set_to(dir.second->name);
		proto_dist.set_distance(dist);

		*proto_catalogue.add_distances() = proto_dist;
	}
}

void Serializer::SerializeBuses(ProtoCatalogue& proto_catalogue) {
	for (const auto& [name, bus] : catalogue_.GetAllBuses()) {
		proto_transport_catalogue::Bus proto_bus;
		proto_bus.set_name(bus->name);
		for (const auto* stop : bus->route) {
			*proto_bus.mutable_route()->Add() = stop->name;
		}
		proto_bus.set_ring_route(bus->ring_route);

		*proto_catalogue.add_buses() = proto_bus;
	}
}

void Serializer::SerializeRenderSettings(ProtoCatalogue& proto_catalogue) {
	const auto& render_settings = renderer_.GetSettings();
	proto_map_renderer::RenderSettings proto_render_settings;

	proto_render_settings.set_width(render_settings.width);
	proto_render_settings.set_height(render_settings.height);

	proto_render_settings.set_padding(render_settings.padding);

	proto_render_settings.set_stop_radius(render_settings.stop_radius);
	proto_render_settings.set_line_width(render_settings.line_width);

	proto_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
	*proto_render_settings.mutable_bus_label_offset() = SerializePoint(render_settings.bus_label_offset);

	proto_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
	*proto_render_settings.mutable_stop_label_offset() = SerializePoint(render_settings.stop_label_offset);

	*proto_render_settings.mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color);
	proto_render_settings.set_underlayer_width(render_settings.underlayer_width);

	for (const auto& color : render_settings.color_palette) {
		*proto_render_settings.add_color_palette() = SerializeColor(color);
	}
	
	*proto_catalogue.mutable_render_settings() = proto_render_settings;
}

proto_svg::Point Serializer::SerializePoint(const svg::Point& point) {
	proto_svg::Point proto_point;

	proto_point.set_x(point.x);
	proto_point.set_y(point.y);

	return proto_point;
}

proto_svg::Color Serializer::SerializeColor(const svg::Color& color) {
	proto_svg::Color proto_color;

	if (std::holds_alternative<std::string>(color)) {
		proto_color.set_color(std::get<std::string>(color));
	} else if (std::holds_alternative<svg::Rgb>(color)) {
		*proto_color.mutable_rgb() = SerializeRgb(std::get<svg::Rgb>(color));
	} else if (std::holds_alternative<svg::Rgba>(color)) {
		*proto_color.mutable_rgba() = SerializeRgba(std::get<svg::Rgba>(color));
	}

	return proto_color;
}

proto_svg::Rgb Serializer::SerializeRgb(const svg::Rgb& rgb) {
	proto_svg::Rgb proto_rgb;

	proto_rgb.set_red(rgb.red);
	proto_rgb.set_green(rgb.green);
	proto_rgb.set_blue(rgb.blue);

	return proto_rgb;
}

proto_svg::Rgba Serializer::SerializeRgba(const svg::Rgba& rgba) {
	proto_svg::Rgba proto_rgba;

	proto_rgba.set_red(rgba.red);
	proto_rgba.set_green(rgba.green);
	proto_rgba.set_blue(rgba.blue);
	proto_rgba.set_opacity(rgba.opacity);

	return proto_rgba;
}

void Serializer::SerializeTransportRouter(ProtoCatalogue& proto_catalogue) {
	SerializeRouterSettings(proto_catalogue);
	SerializeStopIdByName(proto_catalogue);
	SerializeGraph(proto_catalogue);
	SerializeRouter(proto_catalogue);
}

void Serializer::SerializeRouterSettings(ProtoCatalogue& proto_catalogue) {
	const auto& router_settings = router_.GetRouterSettings();
	proto_transport_router::RouterSettings proto_router_settings;

	proto_router_settings.set_wait_time(router_settings.bus_wait_time);
	proto_router_settings.set_velocity(router_settings.bus_velocity);

	*proto_catalogue.mutable_router()->mutable_settings() = proto_router_settings;
}

void Serializer::SerializeStopIdByName(ProtoCatalogue& proto_catalogue) {
	auto proto_stop_id_by_name = proto_catalogue.mutable_router()->mutable_stop_id_by_name();

	for (auto [name, id] : router_.GetStopsIdByName()) {
		proto_transport_router::StopIdByName stop_id_by_name;
		stop_id_by_name.set_name(std::string(name));
		stop_id_by_name.set_id(static_cast<uint32_t>(id));
		*proto_stop_id_by_name->Add() = std::move(stop_id_by_name);
	}
}

void Serializer::SerializeGraph(ProtoCatalogue& proto_catalogue) {
	auto proto_graph = proto_catalogue.mutable_router()->mutable_graph();

	for (auto& edge : router_.GetGraph().GetEdges()) {
		proto_graph::Edge proto_edge;
		proto_edge.set_from(static_cast<uint32_t>(edge.from));
		proto_edge.set_to(static_cast<uint32_t>(edge.to));
		*proto_edge.mutable_weight() = SerializeEdgeWeight(edge.weight);
		*proto_graph->add_edges() = std::move(proto_edge);
	}

	for (auto& list : router_.GetGraph().GetIncidenceLists()) {
		auto proto_list = proto_graph->add_incidence_lists();
		for (auto id : list) {
			proto_list->add_edge_id(static_cast<uint32_t>(id));
		}
	}
}

void Serializer::SerializeRouter(ProtoCatalogue& proto_catalogue) {
	auto proto_router = proto_catalogue.mutable_router()->mutable_router();

	for (const auto& data : router_.GetRouter()->GetRoutesInternalData()) {
		proto_graph::RoutesInternalData proto_data;
		for (const auto& internal : data) {
			proto_graph::OptionalRouteInternalData proto_internal;
			if (internal.has_value()) {
				auto& value = internal.value();
				auto proto_value = proto_internal.mutable_route_internal_data();
				proto_value->set_total_time(value.weight.total_time);
				if (value.prev_edge.has_value()) {
					proto_value->set_prev_edge(static_cast<uint32_t>(value.prev_edge.value()));
				}
			}
			*proto_data.add_routes_internal_data() = std::move(proto_internal);
		}
		*proto_router->add_routes_internal_data() = std::move(proto_data);
	}
}

proto_graph::EdgeWeight
Serializer::SerializeEdgeWeight(const transport_router::EdgeWeight& weight) const {
	proto_graph::EdgeWeight proto_weight;

	proto_weight.set_bus_name(std::string(weight.bus_name));
	proto_weight.set_from(std::string(weight.from));
	proto_weight.set_to(std::string(weight.to));
	proto_weight.set_span_count(weight.span_count);
	proto_weight.set_total_time(weight.total_time);

	return proto_weight;
}

void Serializer::DeserializeStops(ProtoCatalogue& proto_catalogue) {
	for (int i = 0; i < proto_catalogue.stops_size(); ++i) {
		const proto_transport_catalogue::Stop& proto_stop = proto_catalogue.stops(i);
		catalogue_.AddStop(proto_stop.name(), 
			{ proto_stop.coordinates().lat(), proto_stop.coordinates().lng() });
	}
}

void Serializer::DeserializeDistances(ProtoCatalogue& proto_catalogue) {
	for (int i = 0; i < proto_catalogue.distances_size(); ++i) {
		const proto_transport_catalogue::Distance& proto_dist = proto_catalogue.distances(i);

		catalogue_.SetDistance(proto_dist.from(), proto_dist.to(), proto_dist.distance());
	}
}

void Serializer::DeserializeBuses(ProtoCatalogue& proto_catalogue) {
	for (int i = 0; i < proto_catalogue.buses_size(); ++i) {
		const proto_transport_catalogue::Bus& proto_bus = proto_catalogue.buses(i);
		std::vector<std::string> route(proto_bus.route_size());
		for (int j = 0; j < proto_bus.route_size(); ++j) {
			route[j] = proto_bus.route(j);
		}
		catalogue_.AddBus(proto_bus.name(), route, proto_bus.ring_route());
	}
}

void Serializer::DeserializeRenderSettings(ProtoCatalogue& proto_catalogue) {
	const proto_map_renderer::RenderSettings& proto_render_settings = proto_catalogue.render_settings();
	map_renderer::RenderSettings render_settings;

	render_settings.width = proto_render_settings.width();
	render_settings.height = proto_render_settings.height();

	render_settings.padding = proto_render_settings.padding();

	render_settings.stop_radius = proto_render_settings.stop_radius();
	render_settings.line_width = proto_render_settings.line_width();

	render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
	render_settings.bus_label_offset = DeserializePoint(proto_render_settings.bus_label_offset());

	render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
	render_settings.stop_label_offset = DeserializePoint(proto_render_settings.stop_label_offset());

	render_settings.underlayer_color = DeserializeColor(proto_render_settings.underlayer_color());
	render_settings.underlayer_width = proto_render_settings.underlayer_width();

	for (int i = 0; i < proto_render_settings.color_palette_size(); ++i) {
		render_settings.color_palette.push_back(DeserializeColor(proto_render_settings.color_palette(i)));
	}

	renderer_.SetSettings(render_settings);
}

svg::Point Serializer::DeserializePoint(const proto_svg::Point& proto_point) {
	return { proto_point.x(), proto_point.y() };
}

svg::Color Serializer::DeserializeColor(const proto_svg::Color& proto_color) {
	if (proto_color.has_rgb()) {
		return DeserializeRgb(proto_color.rgb());
	} else if (proto_color.has_rgba()) {
		return DeserializeRgba(proto_color.rgba());
	} else {
		return proto_color.color();
	}
}

svg::Rgb Serializer::DeserializeRgb(const proto_svg::Rgb& proto_rgb) {
	return { static_cast<uint8_t>(proto_rgb.red()),
			static_cast<uint8_t>(proto_rgb.green()),
			static_cast<uint8_t>(proto_rgb.blue()) };
}

svg::Rgba Serializer::DeserializeRgba(const proto_svg::Rgba& proto_rgba) {
	return { static_cast<uint8_t>(proto_rgba.red()),
			static_cast<uint8_t>(proto_rgba.green()),
			static_cast<uint8_t>(proto_rgba.blue()),
			proto_rgba.opacity() };
}

void Serializer::DeserializeTransportRouter(ProtoCatalogue& proto_catalogue) {
	DeserializeRouterSettings(proto_catalogue);
	DeserializeStopIdByName(proto_catalogue);
	DeserializeGraph(proto_catalogue);
	DeserializeRouter(proto_catalogue);
}

void Serializer::DeserializeRouterSettings(ProtoCatalogue& proto_catalogue) {
	const proto_transport_router::RouterSettings proto_router_settings = proto_catalogue.router().settings();
	transport_router::RouterSettings router_settings;

	router_settings.bus_wait_time = proto_router_settings.wait_time();
	router_settings.bus_velocity = proto_router_settings.velocity();

	router_.SetRouterSettings(router_settings);
}

void Serializer::DeserializeStopIdByName(ProtoCatalogue& proto_catalogue) {
	auto& proto_router = proto_catalogue.router();
	auto stops_count = proto_router.stop_id_by_name_size();

	for (auto i = 0; i < stops_count; ++i) {
		auto& proto_stop_id_by_name = proto_router.stop_id_by_name(i);
		auto stop = catalogue_.GetAllStops().at(proto_stop_id_by_name.name());
		router_.GetStopsIdByName().insert({ stop->name,
									proto_stop_id_by_name.id() });
	}
}

void Serializer::DeserializeGraph(ProtoCatalogue& proto_catalogue) {
	auto& proto_graph = proto_catalogue.router().graph();
	auto edge_count = proto_graph.edges_size();
	auto& graph = router_.GetGraph();

	for (auto i = 0; i < edge_count; ++i) {
		graph::Edge<transport_router::EdgeWeight> edge;
		auto& proto_edge = proto_graph.edges(i);
		edge.from = proto_edge.from();
		edge.to = proto_edge.to();
		edge.weight = DeserializeEdgeWeight(proto_edge.weight());
		graph.GetEdges().push_back(edge);
	}
	auto incidence_lists_count = proto_graph.incidence_lists_size();

	for (auto i = 0; i < incidence_lists_count; ++i) {
		std::vector<graph::EdgeId> list;
		auto& proto_list = proto_graph.incidence_lists(i);
		auto proto_list_count = proto_list.edge_id_size();

		for (auto j = 0; j < proto_list_count; ++j) {
			list.push_back(proto_list.edge_id(j));
		}
		graph.GetIncidenceLists().push_back(list);
	}
}

void Serializer::DeserializeRouter(ProtoCatalogue& proto_catalogue) {
	router_.GetRouter() = std::make_unique<transport_router::TransportRouter::Router>(router_.GetGraph(), false);	

	auto& proto_router = proto_catalogue.router().router();
	auto& routes_internal_data = router_.GetRouter()->GetRoutesInternalData();
	auto proto_rid_count = proto_router.routes_internal_data_size();

	for (auto i = 0; i < proto_rid_count; ++i) {
		auto& proto_internal_data = proto_router.routes_internal_data(i);
		auto proto_id_count = proto_internal_data.routes_internal_data_size();

		for (auto j = 0; j < proto_id_count; ++j) {
			auto& proto_optional_data = proto_internal_data.routes_internal_data(j);

			if (proto_optional_data.optional_route_internal_data_case() ==
				proto_graph::OptionalRouteInternalData::kRouteInternalData) {

				transport_router::TransportRouter::Router::RouteInternalData data;
				auto& proto_data = proto_optional_data.route_internal_data();
				data.weight.total_time = proto_data.total_time();

				if (proto_data.optional_prev_edge_case() == proto_graph::RouteInternalData::kPrevEdge) {
					data.prev_edge = proto_data.prev_edge();
				} else {
					data.prev_edge = std::nullopt;
				}
				routes_internal_data[i][j] = std::move(data);

			} else {
				routes_internal_data[i][j] = std::nullopt;
			}
		}
	}
}

transport_router::EdgeWeight 
Serializer::DeserializeEdgeWeight(const proto_graph::EdgeWeight& proto_weight) const {
	transport_router::EdgeWeight weight;

	weight.bus_name = catalogue_.GetAllBuses().at(proto_weight.bus_name())->name;
	weight.from = catalogue_.GetAllStops().at(proto_weight.from())->name;
	weight.to = catalogue_.GetAllStops().at(proto_weight.to())->name;
	weight.span_count = proto_weight.span_count();
	weight.total_time = proto_weight.total_time();

	return weight;
}

} // serializer