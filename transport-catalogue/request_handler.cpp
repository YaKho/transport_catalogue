#include <map>

#include "request_handler.h"

namespace request_handler {

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, 
							   const map_renderer::MapRenderer& renderer,
							   const transport_router::TransportRouter& router)
	: db_(db)
	, renderer_(renderer)
	, router_(router) {}

domain::BusStat RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return db_.GetBusInfo(std::string(bus_name));
}

const std::set<std::string_view>& RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return db_.GetBusesAtStop(std::string(stop_name));
}

svg::Document RequestHandler::RenderMap() const {
	svg::Document res;

	const auto& buses = db_.GetAllBuses();
	const auto& stops = db_.GetAllStops();

	map_renderer::Buses ordered_buses;
	map_renderer::Stops used_stops;

	for (const auto& bus : buses) {
		ordered_buses.emplace(bus);
	}
	for (const auto& stop : stops) {
		if (!db_.GetBusesAtStop(stop.second->name).empty()) {
			used_stops.emplace(stop);
		}
	}

	renderer_.RenderMap(used_stops, ordered_buses, res);

	return res;
}

std::optional<RequestHandler::Route> RequestHandler::BuildRoute(const std::string& from, const std::string& to) const {
	return router_.BuildRoute(from, to);
}

const transport_router::TransportRouter& RequestHandler::GetTransportRouter() const {
	return router_;
}

} // namespace request_handler