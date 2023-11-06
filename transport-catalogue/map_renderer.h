#pragma once

#include <algorithm>
#include <map>
#include <unordered_map>
#include <string>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace map_renderer {

struct RenderSettings {
    double width = 0.0;
    double height = 0.0;

    double padding = 0.0;

    double line_width = 0.0;
    double stop_radius = 0;

    int bus_label_font_size = 0;
    svg::Point bus_label_offset;

    int stop_label_font_size = 0;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 0.0;

    std::vector<svg::Color> color_palette;
};

using Buses = std::map<std::string_view, const domain::Bus*>;
using Stops = std::map<std::string_view, const domain::Stop*>;

class SphereProjector;

class MapRenderer final {
public:
    MapRenderer(const RenderSettings& settings = {}) : settings_(settings) {}

    void SetSettings(RenderSettings settings);

    const RenderSettings& GetSettings() const;

    void RenderMap(const Stops& stops, const Buses& buses, svg::Document& document) const;

private:
    RenderSettings settings_;

    void RenderBusLines(const Buses& buses, svg::Document& document, const SphereProjector& proj) const;
    void RenderBusNames(const Buses& buses, svg::Document& document, const SphereProjector& proj) const;
    void RenderStops(const Stops& stops, svg::Document& document, const SphereProjector& proj) const;
    void RenderStopNames(const Stops& stops, svg::Document& document, const SphereProjector& proj) const;
};

class SphereProjector final {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
    : padding_(padding)
{
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

} // namespace map_renderer