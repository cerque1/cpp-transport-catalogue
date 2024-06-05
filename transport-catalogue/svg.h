#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <variant>

namespace svg {

struct Rgb
{
    unsigned short red = 0;
    unsigned short green = 0;
    unsigned short blue = 0;
};

struct Rgba{
    unsigned short red = 0;
    unsigned short green = 0;
    unsigned short blue = 0;
    double opacity = 1.0;
};

struct PrintColor{
    std::ostream& out;

    void operator()(std::monostate) const;
    void operator()(std::string) const;
    void operator()(svg::Rgb) const;
    void operator()(svg::Rgba) const;
};

inline const std::string NoneColor{"none"};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

std::ostream& operator<<(std::ostream& out, Color color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

//---------------------------

struct Point {
    Point() = default;
    Point(double x, double y)
        : x_(x)
        , y_(y) {
    }
    double x_ = 0;
    double y_ = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out_(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out_(out)
        , indent_step_(indent_step)
        , indent_(indent) {
    }

    RenderContext Indented() const {
        return {out_, indent_step_, indent_ + indent_step_};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent_; ++i) {
            out_.put(' ');
        }
    }

    std::ostream& out_;
    int indent_step_ = 0;
    int indent_ = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

//--------------------------------------

template <typename Owner>
class PathProps{
public:
    Owner& SetFillColor(Color color){
        fill_color_ = color;
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color){
        stroke_color_ = color;
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width){
        stroke_width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const{
        using namespace std::literals;

        if(fill_color_.has_value()){
            out << " fill=\""sv << fill_color_.value() << "\""sv;
        }
        if(stroke_color_.has_value()){
            out << " stroke=\""sv << stroke_color_.value() << "\""sv;
        }
        if(stroke_width_.has_value()){
            out << " stroke-width=\""sv << stroke_width_.value() << "\""sv;
        }
        if(line_cap_.has_value()){
            out << " stroke-linecap=\""sv << line_cap_.value() << "\""sv;
        }
        if(line_join_.has_value()){
            out << " stroke-linejoin=\""sv << line_join_.value() << "\""sv;
        }
    }

private:
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;

    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }
};


class Circle final : public Object, public PathProps<Circle> {
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
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    void SetAttr(Color fill, Color stroke, double stroke_width, StrokeLineCap line_cap, StrokeLineJoin line_join);

private: 
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(int size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = {0, 0};
    Point offset_ = {0, 0};
    int size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_ = "";
};

class ObjectContainer{
public:
    ObjectContainer() = default;

    template <typename Obj>
    void Add(Obj obj){
        AddPtr(std::make_unique<Obj>(obj));
    }

    ObjectContainer(ObjectContainer&&) = default;

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
    ~ObjectContainer() = default;

    std::deque<std::unique_ptr<Object>> objects_ptr_;
};

class Drawable{
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    void ConnectDocument(Document doc);

    // Прочие методы и данные, необходимые для реализации класса Document
};

}  // namespace svg