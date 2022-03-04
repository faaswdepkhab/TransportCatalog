#include <map>
#include <cmath>

#include "svg.h"

    
namespace svg {

using namespace std::literals;
    
std::string StrokeLineJoinToString(StrokeLineJoin line_join) {
    std::string result;
    switch(line_join) {
    case StrokeLineJoin::ARCS:
        result = "arcs"s;
        break;
    case StrokeLineJoin::BEVEL:
        result = "bevel"s;
        break;
    case StrokeLineJoin::MITER:
        result = "miter"s;
        break;
    case StrokeLineJoin::MITER_CLIP:
        result = "miter-clip"s;
        break;
    case StrokeLineJoin::ROUND:
        result = "round"s;
        break;
    }
    return result;
}
 
std::string StrokeLineCapToString(StrokeLineCap line_cap) {  
    std::string result;
    switch (line_cap) {
        case StrokeLineCap::BUTT: 
            result = "butt"s;
            break;
        case StrokeLineCap::ROUND: 
            result = "round"s;
            break;
        case StrokeLineCap::SQUARE: 
            result = "square"s;
            break;
    }
    return result;
}    
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    // Делегируем вывод тега своим подклассам
    RenderObject(context);
    context.out << std::endl;
}

ObjectContainer::~ObjectContainer() {
}    
    
Drawable::~Drawable() {
}

// ---------- Circle ------------------
Circle& Circle::SetCenter(Point center = {0.0, 0.0})  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius = 0.0)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto &out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (auto &point:points_) {
        if (!first) {
            out << " "sv;
        }
        first = false;
        out << point.x << ","sv << point.y;
    }
    
    out << "\"";
    RenderAttrs(out);
    out << "/>";
}    
    
    
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

std::string Text::EscapeText(const std::string &s) {
    std::string result = "";
    std::map<char, std::string> EscapeChars;
    EscapeChars.insert({'"', "&quot;"});
    EscapeChars.insert({'\'', "&apos;"});
    EscapeChars.insert({'<', "&lt;"});
    EscapeChars.insert({'>', "&gt;"});
    EscapeChars.insert({'&', "&amp;"});
    for (char c:s) {
        if (EscapeChars.count(c) > 0) {
            result += EscapeChars.at(c);
        } else {
            result += c;
        }
    }
    return result;
}    
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<text"sv;
    
    RenderAttrs(out);
    
    out << " x=\""sv << pos_.x << "\""sv;
    out << " y=\""sv << pos_.y << "\""sv;
    
    out << " dx=\""sv << offset_.x << "\""sv;
    out << " dy=\""sv << offset_.y << "\"";
    
    out << " font-size=\""sv << size_ << "\""sv;
    
    if (font_family_.has_value()) {
        out << " font-family=\""sv << font_family_.value() << "\""sv;    
    }
    
    if (font_weight_.has_value()) {
        out << " font-weight=\""sv << font_weight_.value() << "\""sv;    
    }
    
    out << ">"sv << EscapeText(data_);
    out << "</text>"sv;
}     
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::RenderHeader(const RenderContext& context) const {
    auto& out = context.out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
}
void Document::RenderFooter(const RenderContext& context) const {
    auto& out = context.out;
    out << "</svg>" << std::endl;
}
    
    
void Document::Render(std::ostream& out) const {
    RenderContext context(out);
    RenderHeader(context);
    for (auto &object:objects_) {
        object->Render(context);
    }
    RenderFooter(context);
}
    
}  // namespace svg

namespace shapes {

void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

void Star::Draw(svg::ObjectContainer& container) const {
    svg::Polyline polyline;
    for (int i = 0; i <= num_rays_; ++i) {
        double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle)});
        if (i == num_rays_) {
            break;
        }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle)});
    }
    polyline.SetFillColor("red").SetStrokeColor("black");
    container.Add(polyline);
}
    
void Snowman::Draw(svg::ObjectContainer& container) const {
    //std::cout << "Snowman" << std::endl;
    container.Add(svg::Circle()
        .SetCenter(svg::Point(center_.x, center_.y + 5 * radius_))
        .SetRadius(2.0 * radius_)
        .SetFillColor("rgb(240,240,240)")
        .SetStrokeColor("black"));
    container.Add(svg::Circle()
        .SetCenter(svg::Point(center_.x, center_.y + 2 * radius_))
        .SetRadius(1.5 * radius_)
        .SetFillColor("rgb(240,240,240)")
        .SetStrokeColor("black"));
    container.Add(svg::Circle()
        .SetCenter(svg::Point(center_.x, center_.y))
        .SetRadius(radius_)
        .SetFillColor("rgb(240,240,240)")
        .SetStrokeColor("black"));
}    
    
} // namespace shapes   