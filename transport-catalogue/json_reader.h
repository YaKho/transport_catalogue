#pragma once

#include <iostream>
#include <optional>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

namespace json_reader {

class JSONReader final {
public:
	JSONReader(std::istream& input);

	bool LoadDataToTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) const;

	void PrintResponses(const request_handler::RequestHandler& handler, std::ostream& output) const;

	std::optional<map_renderer::RenderSettings> GetRenderSettings() const;

	std::optional<transport_router::RouterSettings> GetRouterSettings() const;

	std::optional<serializer::SerializerSettings> GetSerializerSettings() const;

private:
	json::Document document_;

	void LoadStops(transport_catalogue::TransportCatalogue& catalogue, const json::Array& data) const;
	void LoadBuses(transport_catalogue::TransportCatalogue& catalogue, const json::Array& data) const;

	static bool IsStop(const json::Node& item);
	static bool IsBus(const json::Node& item);

	const json::Document GetResponses(const request_handler::RequestHandler& handler,
		const json::Array& data) const;

	static bool IsBusRequest(const json::Node& item);
	static bool IsStopRequest(const json::Node& item);
	static bool IsMapRequest(const json::Node& item);
	static bool IsRouteRequest(const json::Node& item);

	static json::Dict GetBusResponse(const request_handler::RequestHandler& handler,
		const json::Dict& data);

	static json::Dict GetStopResponse(const request_handler::RequestHandler& handler,
		const json::Dict& data);

	json::Dict GetMapResponse(const request_handler::RequestHandler& handler,
		const json::Dict& data) const;

	json::Dict GetRouteResponse(const request_handler::RequestHandler& handler,
		const json::Dict& data) const;

	static map_renderer::RenderSettings BuildRenderSettings(const json::Dict& data);
	static transport_router::RouterSettings BuildRouterSettings(const json::Dict& data);

	static svg::Color GetColor(const json::Node& item);
	static svg::Point GetOffset(const json::Array& item);

};

json::Dict ErrorResponse(int id);

}