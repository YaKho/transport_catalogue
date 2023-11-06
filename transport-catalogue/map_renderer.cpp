#include <vector>

#include "map_renderer.h"

namespace map_renderer {

void MapRenderer::RenderMap(const Stops& stops, const Buses& buses, svg::Document& document) const {
	std::vector<geo::Coordinates> coordinates;

	for (const auto& [name, ptr] : stops) {
		coordinates.emplace_back(ptr->coordinates);
	}

	const SphereProjector proj{ coordinates.begin(), coordinates.end(), settings_.width, settings_.height, settings_.padding };

	RenderBusLines(buses, document, proj);
	RenderBusNames(buses, document, proj);
	RenderStops(stops, document, proj);
	RenderStopNames(stops, document, proj);
}

void MapRenderer::SetSettings(RenderSettings settings) {
	settings_ = settings;
}

const RenderSettings& MapRenderer::GetSettings() const {
	return settings_;
}

void MapRenderer::RenderBusLines(const Buses& buses, svg::Document& document, const SphereProjector& proj) const {
	auto colors_count = settings_.color_palette.size();
	size_t cur_color = 0;

	for (const auto& [name, ptr] : buses) {
		if (ptr->route.size()) {
			svg::Polyline line;
			line.SetStrokeColor(settings_.color_palette.at(cur_color % colors_count))
				.SetFillColor(svg::NoneColor)
				.SetStrokeWidth(settings_.line_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			for (const auto& stop : ptr->route) {
				line.AddPoint(proj(stop->coordinates));
			}
			if (!ptr->ring_route) {
				for (auto it = std::next(ptr->route.rbegin()); it < ptr->route.rend(); ++it) {
					line.AddPoint(proj((*it)->coordinates));
				}
			}
			document.Add(line);
			++cur_color;
		}
	}

}

void MapRenderer::RenderBusNames(const Buses& buses, svg::Document& document, const SphereProjector& proj) const {
	auto colors_count = settings_.color_palette.size();
	size_t cur_color = 0;

	for (const auto& [name, ptr] : buses) {
		if (ptr->route.size()) {
			svg::Text text, text_background;
			text.SetData(std::string(name))
				.SetPosition(proj(ptr->route.front()->coordinates))
				.SetOffset(settings_.bus_label_offset)
				.SetFontSize(static_cast<std::uint32_t>(settings_.bus_label_font_size))
				.SetFontFamily("Verdana").SetFontWeight("bold");
			text_background = text;
			text.SetFillColor(settings_.color_palette.at(cur_color % colors_count));
			text_background.SetFillColor(settings_.underlayer_color)
				.SetStrokeColor(settings_.underlayer_color)
				.SetStrokeWidth(settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(text_background);
			document.Add(text);
			if (!(ptr->ring_route) && ptr->route.back() != ptr->route.front()) {
				text.SetPosition(proj(ptr->route.back()->coordinates));
				text_background.SetPosition(proj(ptr->route.back()->coordinates));
				document.Add(text_background);
				document.Add(text);
			}
			++cur_color;
		}
	}
}

void MapRenderer::RenderStops(const Stops& stops, svg::Document& document, const SphereProjector& proj) const {
	for (const auto& [name, ptr] : stops) {
		svg::Circle circle;
		circle.SetCenter(proj(ptr->coordinates)).
			SetRadius(settings_.stop_radius).SetFillColor("white");
		document.Add(circle);
	}
}

void MapRenderer::RenderStopNames(const Stops& stops, svg::Document& document, const SphereProjector& proj) const {
	for (const auto& [name, ptr] : stops) {
		svg::Text text, text_background;
		text.SetData(std::string(name))
			.SetPosition(proj(ptr->coordinates))
			.SetOffset(settings_.stop_label_offset)
			.SetFontSize(static_cast<std::uint32_t>(settings_.stop_label_font_size))
			.SetFontFamily("Verdana");
		text_background = text;
		text.SetFillColor("black");
		text_background.SetFillColor(settings_.underlayer_color)
			.SetStrokeColor(settings_.underlayer_color)
			.SetStrokeWidth(settings_.underlayer_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		document.Add(text_background);
		document.Add(text);
	}
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
	return {
		(coords.lng - min_lon_) * zoom_coeff_ + padding_,
		(max_lat_ - coords.lat) * zoom_coeff_ + padding_
	};
}

bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

} // namespace map_renderer