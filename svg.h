#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace svg {

using Color = std::string;

inline const Color NoneColor{"none"};    
    
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::string StrokeLineCapToString(StrokeLineCap line_cap);
inline std::ostream& operator<<(std::ostream &out, StrokeLineCap line_cap) {
    out << StrokeLineCapToString(line_cap);
    return out;
}     
    
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::string StrokeLineJoinToString(StrokeLineJoin line_join);
inline std::ostream& operator<<(std::ostream &out, StrokeLineJoin line_join) {
    out << StrokeLineJoinToString(line_join);
    return out;
}    
    
template <typename Owner>    
class PathProps {
public:
    Owner& SetFillColor(Color fill_color) {
        fill_color_ = std::move(fill_color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color stroke_color) {
        stroke_color_ = std::move(stroke_color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        width_ = std::move(width);
        return AsOwner();
    }
    
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

protected:

    ~PathProps() = default;

    void RenderFillAttr(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
    }
    
    void RenderStrokeColorAttr(std::ostream& out) const {
        using namespace std::literals;
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
    }
    
    void RenderWidthAttr(std::ostream& out) const {
        using namespace std::literals;
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
    }
    
    void RenderStrokeLineCapAttr(std::ostream& out) const {
        using namespace std::literals;
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
    }
    
    void RenderStrokeLineJoinAttr(std::ostream& out) const {
        using namespace std::literals;
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }
    
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        
        RenderFillAttr(out);
        RenderStrokeColorAttr(out);
        RenderWidthAttr(out);
        RenderStrokeLineCapAttr(out);
        RenderStrokeLineJoinAttr(out);
        
    }

private:
    std::optional<Color> fill_color_, stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
    
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
    
};
    
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

    
class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    } 

    // Добавляет в svg-документ объект-наследник svg::Object
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    virtual ~ObjectContainer();
    
};
    
class Drawable {
public: 
    virtual void Draw(ObjectContainer &container) const = 0;
    virtual ~Drawable();
}; 
    
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle: public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline: public Object, public PathProps<Polyline> {
public:
    Polyline() = default;
    
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_ = {};
    
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text: public Object, public PathProps<Text> {
public:
    Text() = default;
    
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);
    
    // по умолчанию
    static std::string EscapeText(const std::string &s);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::string data_ = "";
    std::optional<std::string> font_weight_;
    std::optional<std::string> font_family_;
    uint32_t size_ = 1;
    Point offset_ = {0.0, 0.0};
    Point pos_ = {0.0, 0.0};
};

class Document final: public ObjectContainer {
public:
    Document() = default;
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    void RenderHeader(const RenderContext& context) const;
    void RenderFooter(const RenderContext& context) const;
    
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg

namespace shapes {

class Triangle: public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star: public svg::Drawable { 
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays):center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad), num_rays_(num_rays) {
    }
    void Draw(svg::ObjectContainer& container) const override;
private:    
    svg::Point center_;
    double outer_rad_, inner_rad_;
    int num_rays_;    
};
    
class Snowman: public svg::Drawable {
public:  
    Snowman(svg::Point center, double radius): center_(center), radius_(radius) {
    }
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point center_;
    double radius_;
};

    
} // namespace shapes