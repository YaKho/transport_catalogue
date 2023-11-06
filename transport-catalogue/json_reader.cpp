#include <string>
#include <vector>
#include <sstream>

#include "json_reader.h"

using namespace std::literals;

namespace json_reader {

JSONReader::JSONReader(std::istream& input) : document_(json::Load(input)) {}

bool JSONReader::LoadDataToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const {
	if (document_.GetRoot().IsMap() && document_.GetRoot().AsMap().count("base_requests"s)) {
		const auto& data = document_.GetRoot().AsMap().at("base_requests"s);
		if (data.IsArray()) {
			LoadStops(catalogue, data.AsArray());
			LoadBuses(catalogue, data.AsArray());

			return true;
		}
	}
	return false;
}

void JSONReader::PrintResponses(const request_handler::RequestHandler& handler, std::ostream& output) const {
	if (document_.GetRoot().IsMap() && document_.GetRoot().AsMap().count("stat_requests"s)) {
		const auto& data = document_.GetRoot().AsMap().at("stat_requests"s);
		if (data.IsArray()) {
			const auto responses = GetResponses(handler, data.AsArray());

			json::Print(responses, output);
		}
	}
}

std::optional<map_renderer::RenderSettings> JSONReader::GetRenderSettings() const {
	if (document_.GetRoot().IsMap() && document_.GetRoot().AsMap().count("render_settings"s)) {
		const auto& data = document_.GetRoot().AsMap().at("render_settings"s);
		if (data.IsMap()) {
			return BuildRenderSettings(data.AsMap());
		}
	}
	return std::nullopt;
}

std::optional<transport_router::RouterSettings> JSONReader::GetRouterSettings() const {
	if (document_.GetRoot().IsMap() && document_.GetRoot().AsMap().count("routing_settings"s)) {
		const auto& data = document_.GetRoot().AsMap().at("routing_settings"s);
		if (data.IsMap()) {
			return BuildRouterSettings(data.AsMap());
		}
	}
	return std::nullopt;
}

std::optional<serializer::SerializerSettings> JSONReader::GetSerializerSettings() const {
	if (document_.GetRoot().IsMap() && document_.GetRoot().AsMap().count("serialization_settings"s)) {
		const auto& data = document_.GetRoot().AsMap().at("serialization_settings"s);
		if (data.IsMap() && data.AsMap().count("file"s)) {
			serializer::SerializerSettings settings;

			settings.path = data.AsMap().at("file"s).AsString();

			return settings;
		}
	}
	return std::nullopt;
}

void JSONReader::LoadStops(transport_catalogue::TransportCatalogue& catalogue, const json::Array& data) const {
	for (const auto& item : data) {
		if (IsStop(item)) {
			catalogue.AddStop(item.AsMap().at("name"s).AsString(),
				{ item.AsMap().at("latitude"s).AsDouble(), item.AsMap().at("longitude"s).AsDouble() });
		}
	}
	for (const auto& item : data) {
		if (IsStop(item)) {
			const auto& distances = item.AsMap().at("road_distances"s).AsMap();
			const auto& from = item.AsMap().at("name"s).AsString();
			for (const auto& [to, distance] : distances) {
				if (distance.IsInt()) {
					catalogue.SetDistance(from, to, distance.AsInt());
				}
			}
		}
	}
}

void JSONReader::LoadBuses(transport_catalogue::TransportCatalogue& catalogue, const json::Array& data) const {
	for (const auto& item : data) {
		if (IsBus(item)) {
			std::vector<std::string> stops;
			for (const auto& stop : item.AsMap().at("stops"s).AsArray()) {
				if (stop.IsString()) {
					stops.push_back(stop.AsString());
				}
			}
			catalogue.AddBus(item.AsMap().at("name"s).AsString(), stops, item.AsMap().at("is_roundtrip"s).AsBool());
		}
	}
}

bool JSONReader::IsStop(const json::Node& item) {
	if (item.IsMap()) {
		const auto& stop = item.AsMap();
		if (stop.count("type"s) && stop.at("type"s) == "Stop"s &&
			stop.count("name"s) && stop.at("name"s).IsString() &&
			stop.count("latitude"s) && stop.at("latitude"s).IsDouble() &&
			stop.count("longitude"s) && stop.at("longitude"s).IsDouble() &&
			stop.count("road_distances"s) && stop.at("road_distances"s).IsMap()) {
			return true;
		}
	}
	return false;
}

bool JSONReader::IsBus(const json::Node& item) {
	if (item.IsMap()) {
		const auto& bus = item.AsMap();
		if (bus.count("type"s) && bus.at("type"s) == "Bus"s &&
			bus.count("name"s) && bus.at("name"s).IsString() &&
			bus.count("is_roundtrip"s) && bus.at("is_roundtrip"s).IsBool() &&
			bus.count("stops"s) && bus.at("stops"s).IsArray()) {
			return true;
		}
	}
	return false;
}

const json::Document JSONReader::GetResponses(const request_handler::RequestHandler& handler,
	const json::Array& data) const {
	json::Array res;
	
	for (const auto& req : data) {
		if (IsBusRequest(req)) {
			res.push_back(GetBusResponse(handler, req.AsMap()));
		} else if (IsStopRequest(req)) {
			res.push_back(GetStopResponse(handler, req.AsMap()));
		} else if (IsMapRequest(req)) {
			res.push_back(GetMapResponse(handler, req.AsMap()));
		} else if (IsRouteRequest(req)) {
			res.push_back(GetRouteResponse(handler, req.AsMap()));
		}
	}

	return { res };
}

bool JSONReader::IsBusRequest(const json::Node& item) {
	if (item.IsMap()) {
		const auto& req = item.AsMap();
		if (req.count("type"s) && req.at("type"s) == "Bus"s &&
			req.count("id"s) && req.at("id"s).IsInt() &&
			req.count("name"s) && req.at("name"s).IsString()) {
			return true;
		}
	}
	return false;
}

bool JSONReader::IsStopRequest(const json::Node& item) {
	if (item.IsMap()) {
		const auto& req = item.AsMap();
		if (req.count("type"s) && req.at("type"s) == "Stop"s &&
			req.count("id"s) && req.at("id"s).IsInt() &&
			req.count("name"s) && req.at("name"s).IsString()) {
			return true;
		}
	}
	return false;
}

bool JSONReader::IsMapRequest(const json::Node& item) {
	if (item.IsMap()) {
		const auto& req = item.AsMap();
		if (req.count("type"s) && req.at("type"s) == "Map"s &&
			req.count("id"s) && req.at("id"s).IsInt()) {
			return true;
		}
	}
	return false;
}

bool JSONReader::IsRouteRequest(const json::Node& item) {
	if (item.IsMap()) {
		const auto& req = item.AsMap();
		if (req.count("type"s) && req.at("type"s) == "Route"s &&
			req.count("id"s) && req.at("id"s).IsInt() &&
			req.count("from"s) && req.at("from"s).IsString() &&
			req.count("to"s) && req.at("to"s).IsString()) {
			return true;
		}
	}
	return false;
}

json::Dict JSONReader::GetBusResponse(const request_handler::RequestHandler& handler,
	const json::Dict& data) {
	int id = data.at("id"s).AsInt();
	const auto& name = data.at("name"s).AsString();
	try {
		auto stat = handler.GetBusStat(name);
		json::Dict res = json::Builder{}
							.StartDict()
								.Key("request_id").Value(id)
								.Key("curvature").Value(stat.curvature)
								.Key("route_length").Value(stat.length)
								.Key("stop_count").Value(stat.stops)
								.Key("unique_stop_count").Value(stat.unic_stops)
							.EndDict()
						.Build().AsMap();
		return res;
	} catch (std::out_of_range&) {
		return ErrorResponse(id);
	}
}

json::Dict JSONReader::GetStopResponse(const request_handler::RequestHandler& handler,
	const json::Dict& data) {
	int id = data.at("id"s).AsInt();
	const auto& name = data.at("name"s).AsString();
	try {
		auto stat = handler.GetBusesByStop(name);
		json::Array buses;
		for (const auto& bus : stat) {
			buses.push_back(std::string(bus));
		}
		json::Dict res = json::Builder{}
							.StartDict()
								.Key("request_id"s).Value(id)
								.Key("buses"s).Value(buses)
							.EndDict()
						.Build().AsMap();
		return res;
	} catch (std::out_of_range&) {
		return ErrorResponse(id);
	}
}

json::Dict JSONReader::GetMapResponse(const request_handler::RequestHandler& handler,
	const json::Dict& data) const {

	int id = data.at("id"s).AsInt();
	std::ostringstream out;
	try {		
		const auto map = handler.RenderMap();

		map.Render(out);

		json::Dict res = json::Builder{}
							.StartDict()
								.Key("request_id"s).Value(id)
								.Key("map"s).Value(out.str())
							.EndDict()
						.Build().AsMap();
		return res;
	} catch (std::out_of_range&) {
		return ErrorResponse(id);
	}
}

json::Dict JSONReader::GetRouteResponse(const request_handler::RequestHandler& handler,
	const json::Dict& data) const {

	int id = data.at("id"s).AsInt();
		
	try {
		const auto route = handler.BuildRoute(data.at("from"s).AsString(), data.at("to"s).AsString());

		if (!route.has_value()) {
			return ErrorResponse(id);
		}
		json::Array items;

		int wait_time = handler.GetTransportRouter().GetRouterSettings().bus_wait_time;

		for (const auto& item : route->route) {
			json::Dict wait_item = json::Builder{}
										.StartDict()
											.Key("stop_name"s).Value(std::string(item.from))																								
											.Key("time"s).Value(wait_time)
											.Key("type"s).Value("Wait"s)
										.EndDict()
									.Build().AsMap();
			items.push_back(wait_item);
			json::Dict go_item = json::Builder{}
									.StartDict()											
										.Key("bus"s).Value(std::string(item.bus_name))
										.Key("span_count"s).Value(item.span_count)
										.Key("time"s).Value(item.total_time - wait_time)
										.Key("type"s).Value("Bus"s)
									.EndDict()
								.Build().AsMap();				
			items.push_back(go_item);
		}
		json::Dict res = json::Builder{}
							.StartDict()
								.Key("request_id"s).Value(id)
								.Key("total_time"s).Value(route->total_time)
								.Key("items"s).Value(items)
							.EndDict()
						.Build().AsMap();
		return res;
		
	} catch (std::out_of_range&) {
		return ErrorResponse(id);
	}
}

map_renderer::RenderSettings JSONReader::BuildRenderSettings(const json::Dict& data) {
	map_renderer::RenderSettings res;

	if (data.count("width"s) && data.at("width"s).IsDouble()) {
		res.width = data.at("width"s).AsDouble();
	}
	if (data.count("height"s) && data.at("height"s).IsDouble()) {
		res.height = data.at("height"s).AsDouble();
	}
	if (data.count("padding"s) && data.at("padding"s).IsDouble()) {
		res.padding = data.at("padding"s).AsDouble();
	}
	if (data.count("line_width"s) && data.at("line_width"s).IsDouble()) {
		res.line_width = data.at("line_width"s).AsDouble();
	}
	if (data.count("stop_radius"s) && data.at("stop_radius"s).IsDouble()) {
		res.stop_radius = data.at("stop_radius"s).AsDouble();
	}
	if (data.count("bus_label_font_size"s) && data.at("bus_label_font_size"s).IsInt()) {
		res.bus_label_font_size = data.at("bus_label_font_size"s).AsInt();
	}
	if (data.count("bus_label_offset"s) && data.at("bus_label_offset"s).IsArray()) {
		res.bus_label_offset = GetOffset(data.at("bus_label_offset"s).AsArray());
	}
	if (data.count("stop_label_font_size"s) && data.at("stop_label_font_size"s).IsInt()) {
		res.stop_label_font_size = data.at("stop_label_font_size"s).AsInt();
	}
	if (data.count("stop_label_offset"s) && data.at("stop_label_offset"s).IsArray()) {
		res.stop_label_offset = GetOffset(data.at("stop_label_offset"s).AsArray());
	}
	if (data.count("underlayer_color"s)) {
		res.underlayer_color = GetColor(data.at("underlayer_color"s));
	}
	if (data.count("underlayer_width"s) && data.at("underlayer_width"s).IsDouble()) {
		res.underlayer_width = data.at("underlayer_width"s).AsDouble();
	}
	if (data.count("color_palette"s) && data.at("color_palette"s).IsArray()) {
		for (auto& color : data.at("color_palette"s).AsArray()) {
			res.color_palette.push_back(GetColor(color));
		}
	}

	return res;
}

transport_router::RouterSettings JSONReader::BuildRouterSettings(const json::Dict& data) {
	constexpr static double FACTOR = 1000.0 / 60.0; // km per hour to meters per minute

	transport_router::RouterSettings res;

	if (data.count("bus_wait_time"s) && data.at("bus_wait_time"s).IsInt()) {
		res.bus_wait_time = data.at("bus_wait_time"s).AsInt();
	}
	if (data.count("bus_velocity"s) && data.at("bus_velocity"s).IsDouble()) {
		res.bus_velocity = data.at("bus_velocity"s).AsDouble() * FACTOR;
	}

	return res;
}

svg::Color JSONReader::GetColor(const json::Node& item) {
	if (item.IsString()) {
		return item.AsString();
	} else if (item.IsArray() && item.AsArray().size() == 3) {
		return svg::Rgb(static_cast<uint8_t>(item.AsArray().at(0).AsInt()),
			static_cast<uint8_t>(item.AsArray().at(1).AsInt()),
			static_cast<uint8_t>(item.AsArray().at(2).AsInt()));
	} else if (item.IsArray() && item.AsArray().size() == 4) {
		return svg::Rgba(static_cast<uint8_t>(item.AsArray().at(0).AsInt()),
			static_cast<uint8_t>(item.AsArray().at(1).AsInt()),
			static_cast<uint8_t>(item.AsArray().at(2).AsInt()),
			item.AsArray().at(3).AsDouble());
	} else {
		return svg::NoneColor;
	}
}

svg::Point JSONReader::GetOffset(const json::Array& item) {
	svg::Point res;
	if (item.size() > 1) {
		if (item.at(0).IsDouble()) {
			res.x = item.at(0).AsDouble();
		}
		if (item.at(1).IsDouble()) {
			res.y = item.at(1).AsDouble();
		}
	}
	return res;
}

json::Dict ErrorResponse(int id) {
	return json::Builder{}
			.StartDict()
				.Key("request_id"s).Value(id)
				.Key("error_message"s).Value("not found"s)
			.EndDict()
		.Build().AsMap();
}

} // namespace json_reader