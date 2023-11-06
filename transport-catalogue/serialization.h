#pragma once

#include <string>
#include <filesystem>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"
#include "graph.pb.h"

namespace serializer {

struct SerializerSettings {
	std::filesystem::path path;
};

class Serializer {
public:
	using ProtoCatalogue = proto_transport_catalogue::TransportCatalogue;

	Serializer(const SerializerSettings& settings,
		transport_catalogue::TransportCatalogue& catalogue,
		map_renderer::MapRenderer& renderer,
		transport_router::TransportRouter& router)
		: settings_(settings)
		, catalogue_(catalogue)
		, renderer_(renderer)
		, router_(router) {}

	void Serialize();

	void Deserialize();

private:
	SerializerSettings settings_;

	transport_catalogue::TransportCatalogue& catalogue_;
	map_renderer::MapRenderer& renderer_;
	transport_router::TransportRouter& router_;

	void SerializeStops(ProtoCatalogue& proto_catalogue);
	void SerializeDistances(ProtoCatalogue& proto_catalogue);
	void SerializeBuses(ProtoCatalogue& proto_catalogue);

	void SerializeRenderSettings(ProtoCatalogue& proto_catalogue);
	proto_svg::Point SerializePoint(const svg::Point& point);
	proto_svg::Color SerializeColor(const svg::Color& color);
	proto_svg::Rgb SerializeRgb(const svg::Rgb& rgb);
	proto_svg::Rgba SerializeRgba(const svg::Rgba& rgba);

	void SerializeTransportRouter(ProtoCatalogue& proto_catalogue);
	void SerializeRouterSettings(ProtoCatalogue& proto_catalogue);
	void SerializeStopIdByName(ProtoCatalogue& proto_catalogue);
	void SerializeGraph(ProtoCatalogue& proto_catalogue);
	void SerializeRouter(ProtoCatalogue& proto_catalogue);
	proto_graph::EdgeWeight SerializeEdgeWeight(const transport_router::EdgeWeight& weight) const;

	void DeserializeStops(ProtoCatalogue& proto_catalogue);
	void DeserializeDistances(ProtoCatalogue& proto_catalogue);
	void DeserializeBuses(ProtoCatalogue& proto_catalogue);

	void DeserializeRenderSettings(ProtoCatalogue& proto_catalogue);
	svg::Point DeserializePoint(const proto_svg::Point& proto_point);
	svg::Color DeserializeColor(const proto_svg::Color& proto_color);
	svg::Rgb DeserializeRgb(const proto_svg::Rgb& proto_rgb);
	svg::Rgba DeserializeRgba(const proto_svg::Rgba& proto_rgba);

	void DeserializeTransportRouter(ProtoCatalogue& proto_catalogue);
	void DeserializeRouterSettings(ProtoCatalogue& proto_catalogue);
	void DeserializeStopIdByName(ProtoCatalogue& proto_catalogue);
	void DeserializeGraph(ProtoCatalogue& proto_catalogue);
	void DeserializeRouter(ProtoCatalogue& proto_catalogue);
	transport_router::EdgeWeight DeserializeEdgeWeight(const proto_graph::EdgeWeight& proto_weight) const;
};	

} // serializer