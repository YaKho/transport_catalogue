#pragma once

#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace request_handler {

class RequestHandler {
public:
    using Route = transport_router::TransportRoute;

    RequestHandler(const transport_catalogue::TransportCatalogue& db, 
                   const map_renderer::MapRenderer& renderer,
                   const transport_router::TransportRouter& router);

    domain::BusStat GetBusStat(const std::string_view& bus_name) const;

    const std::set<std::string_view>& GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

    std::optional<Route> BuildRoute(const std::string& from, const std::string& to) const;

    const transport_router::TransportRouter& GetTransportRouter() const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
    const transport_router::TransportRouter& router_;
};

}