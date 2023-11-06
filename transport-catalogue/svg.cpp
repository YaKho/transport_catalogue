#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//----------- Polyline -----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (const auto& point : points_) {
        if (!first) {
            out << " "sv;
        } else {
            first = false;
        }
        out << point.x << "," << point.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//------------- Text -------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" "sv;
    out << "y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" "sv;
    out << "dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    out << EscapeText(data_);
    out << "</text>"sv;
}

std::string Text::EscapeText(const std::string& text) {
    std::string res;

    for (char c : text) {
        switch (c) {
        case '\"':
            res += "&quot;"s;
            break;
        case '\'':
            res += "&apos;"s;
            break;
        case '<':
            res += "&lt;"s;
            break;
        case '>':
            res += "&gt;"s;
            break;
        case '&':
            res += "&amp;"s;
            break;
        default:
            res += c;
            break;
        }
    }
    return res;
}

// -------------- Document ---------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext context(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(context);
    }
    out << "</svg>"sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap obj) {
    switch (obj) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin obj) {
    switch (obj) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(OstreamColorPrinter{ out }, color);
    return out;
}

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}
void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}
void OstreamColorPrinter::operator()(svg::Rgb color) const {
    out << "rgb("sv
        << int(color.red) << ","sv
        << int(color.green) << ","sv
        << int(color.blue)
        << ")"sv;
}
void OstreamColorPrinter::operator()(svg::Rgba color) const {
    out << "rgba("sv
        << int(color.red) << ","sv
        << int(color.green) << ","sv
        << int(color.blue) << ","sv
        << color.opacity
        << ")"sv;
}

}  // namespace svg