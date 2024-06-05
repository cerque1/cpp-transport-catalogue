#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <algorithm>
#include <optional>
#include <unordered_map>

namespace renderer{

namespace details{
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector{
    public:
        SphereProjector() = default;

        template <typename RandomPointIt>
        SphereProjector(RandomPointIt points_begin, RandomPointIt points_end, double width, double height, double padding) : padding_(padding){
            if(points_begin == points_end){
                return;
            }

            auto [min_lng_point, max_lng_point] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs){
                return lhs.lng < rhs.lng;
            });
            min_lng_ = min_lng_point->lng;
            const double max_lng = max_lng_point->lng;

            auto [min_lat_point, max_lat_point] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { 
                return lhs.lat < rhs.lat;
            });
            const double min_lat = min_lat_point->lat;
            max_lat_ = max_lat_point->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lng - min_lng_)) {
                width_zoom = (width - 2 * padding) / (max_lng - min_lng_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lng_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
}

class MapRenderer final{
public:
    MapRenderer() = default;

    void SetRenderSettings(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                           const geo::Coordinates& bus_label_offset, int stop_label_font_size, const geo::Coordinates& stop_label_offset, const svg::Color& underlayer_color, 
                           double underlayer_width, std::vector<svg::Color> color_palette);

    svg::Document RenderMap(const std::vector<const Bus*>& buses) const;
    
    void SetSphereProjector(const std::unordered_map<std::string_view, const Bus*>& bus_points);

private:
    details::SphereProjector sphereProjector_;

    double width_ = 0;
    double height_ = 0;
    double padding_ = 0;
    
    double line_width_ = 0;
    double stop_radius_ = 0;

    int bus_label_font_size_ = 0;
    geo::Coordinates bus_label_offset_ = {0, 0};

    int stop_label_font_size_ = 0;
    geo::Coordinates stop_label_offset_ = {0, 0};

    svg::Color underlayer_color_;
    double underlayer_width_ = 0;

    std::vector<svg::Color> color_palette_;
};

}
