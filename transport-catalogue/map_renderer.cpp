#include "map_renderer.h"
#include <vector>

namespace renderer{

namespace details{
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const{
        return {
            (coords.lng - min_lng_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    svg::Document FirstLayer(const std::vector<const Stop*>& stops, bool is_round, const svg::Color& color,
                             double line_width_, const details::SphereProjector& sphereProjector_){
        svg::Document doc;
        svg::Polyline polyline;

        polyline.SetAttr(svg::NoneColor, color, line_width_, svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND);

        for(auto stop : stops){
            polyline.AddPoint(sphereProjector_(stop->coordinates));
        }

        if(is_round == false){
            std::vector<const Stop*> reverse_stops(++stops.rbegin(), stops.rend());
            for(auto stop : reverse_stops){
                polyline.AddPoint(sphereProjector_(stop->coordinates));
            }
        }
        doc.Add(polyline);
        return doc;
    }

    svg::Document SecondLayer(const Bus* bus, const details::SphereProjector& sphereProjector_, geo::Coordinates bus_label_offset, int font_size, 
                              const svg::Color& underlayer_color, double underlayer_width, const svg::Color& fill_color){
        svg::Document doc;
        
        if(bus->is_round || std::equal(bus->stops.back()->name.begin(), bus->stops.back()->name.end(), bus->stops.front()->name.begin(), bus->stops.front()->name.end())){
            svg::Text bus_name;
            bus_name.SetPosition(sphereProjector_(bus->stops.front()->coordinates)).SetOffset(svg::Point(bus_label_offset.lat, bus_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetFillColor(underlayer_color).SetStrokeWidth(underlayer_width)
            .SetStrokeColor(underlayer_color).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetData(bus->name);

            svg::Text substrate;
            substrate.SetPosition(sphereProjector_(bus->stops.front()->coordinates)).SetOffset(svg::Point(bus_label_offset.lat, bus_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetFillColor(fill_color).SetData(bus->name);

            doc.Add(bus_name);
            doc.Add(substrate);
        }
        else if(bus->stops.size() > 1){
            svg::Text bus_name_begin_;
            bus_name_begin_.SetPosition(sphereProjector_(bus->stops.front()->coordinates)).SetOffset(svg::Point(bus_label_offset.lat, bus_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetFillColor(underlayer_color).SetStrokeWidth(underlayer_width)
            .SetStrokeColor(underlayer_color).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetData(bus->name);

            svg::Text substrate_begin_;
            substrate_begin_.SetPosition(sphereProjector_(bus->stops.front()->coordinates)).SetOffset(svg::Point(bus_label_offset.lat, bus_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetFillColor(fill_color).SetData(bus->name);

            svg::Text bus_name_end_ = bus_name_begin_;
            bus_name_end_.SetPosition(sphereProjector_(bus->stops.back()->coordinates));
            svg::Text substrate_end_ = substrate_begin_;
            substrate_end_.SetPosition(sphereProjector_(bus->stops.back()->coordinates));

            doc.Add(bus_name_begin_);
            doc.Add(substrate_begin_);
            doc.Add(bus_name_end_);
            doc.Add(substrate_end_);
        }
        return doc;
    }
    
    svg::Document ThirdLayer(std::vector<const Stop*> stops, double radius, const SphereProjector& sphereProjector_){
        svg::Document doc;

        for(auto stop : stops){
            svg::Circle stop_circle;
            stop_circle.SetCenter(sphereProjector_(stop->coordinates)).SetRadius(radius).SetFillColor("white");
            doc.Add(stop_circle);
        }
        return doc;
    }

    svg::Document FourthLayer(std::vector<const Stop*> stops, const SphereProjector& sphereProjector_, geo::Coordinates stop_label_offset, 
                     int font_size, const svg::Color &underlayer_color, double underlayer_width){
        svg::Document doc;

        for(auto stop : stops){
            svg::Text stop_name_;
            stop_name_.SetPosition(sphereProjector_(stop->coordinates)).SetOffset(svg::Point(stop_label_offset.lat, stop_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFillColor(underlayer_color).SetStrokeWidth(underlayer_width)
            .SetStrokeColor(underlayer_color).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetData(stop->name);

            svg::Text stop_substrate_;
            stop_substrate_.SetPosition(sphereProjector_(stop->coordinates)).SetOffset(svg::Point(stop_label_offset.lat, stop_label_offset.lng))
            .SetFontSize(font_size).SetFontFamily("Verdana").SetFillColor("black").SetData(stop->name);

            doc.Add(stop_name_);
            doc.Add(stop_substrate_);
        }
        return doc;
    }
}

void MapRenderer::SetRenderSettings(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                           const geo::Coordinates& bus_label_offset, int stop_label_font_size, const geo::Coordinates& stop_label_offset,
                           const svg::Color& underlayer_color, double underlayer_width, std::vector<svg::Color> color_palette){
    width_ = width;
    height_ = height;
    padding_ = padding;
    line_width_ = line_width;
    stop_radius_ = stop_radius;
    bus_label_font_size_ = bus_label_font_size;
    bus_label_offset_ = bus_label_offset;
    stop_label_font_size_ = stop_label_font_size;
    stop_label_offset_ = stop_label_offset;
    underlayer_color_ = underlayer_color;
    underlayer_width_ = underlayer_width;
    color_palette_ = std::move(color_palette);
}

void MapRenderer::SetSphereProjector(const std::unordered_map<std::string_view, const Bus*>& bus_points){
    std::vector<geo::Coordinates> coords;
    for(auto [bus_name, stops] : bus_points){
        for(auto stop : stops->stops){
            coords.emplace_back(stop->coordinates);
        }
    }

    sphereProjector_ = details::SphereProjector{coords.begin(), coords.end(), width_, height_, padding_};
}

svg::Document MapRenderer::RenderMap(const std::vector<const Bus*>& buses) const{
    svg::Document doc;
    size_t num_color = 0;

    for(auto bus : buses){
        doc.ConnectDocument(details::FirstLayer(bus->stops, bus->is_round, color_palette_[num_color % color_palette_.size()], line_width_, sphereProjector_));
        num_color++;
    }

    num_color = 0;

    for(auto bus : buses){
        doc.ConnectDocument(details::SecondLayer(bus, sphereProjector_, bus_label_offset_, bus_label_font_size_, underlayer_color_, underlayer_width_, color_palette_[num_color % color_palette_.size()]));
        num_color++;
    }


    std::vector<const Stop*> stops;
    for(auto bus : buses){
        for(auto stop : bus->stops){
            if(!std::count(stops.begin(), stops.end(), stop)){
                stops.emplace_back(stop);
            }
        }
    }

    std::sort(stops.begin(), stops.end(), [](auto lhs, auto rhs){
        return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
    });

    doc.ConnectDocument(details::ThirdLayer(stops, stop_radius_, sphereProjector_));
    doc.ConnectDocument(details::FourthLayer(stops, sphereProjector_, stop_label_offset_, stop_label_font_size_, underlayer_color_, underlayer_width_));

    return doc;
}

}