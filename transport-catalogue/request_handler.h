#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "domain.h"
#include "map_renderer.h"

#include <optional>

class RequestHandler {
public:
    RequestHandler(transport_catalogue::TransportCatalogue&, renderer::MapRenderer& renderer);

    void AddStop(const std::string& name, geo::Coordinates coord);
    void AddBus(const std::string &name, const std::vector<std::string_view> &stops_name, bool is_round);
    void AddDistance(const std::string &stop_from, const std::string &stop_to, int distance);

    const Stop* FindStop(std::string_view name) const;
    const Bus* FindBus(std::string_view name) const;
    int FindDistance(const std::string& stop_from, const std::string& stop_to) const;

    // Возвращает информацию о маршруте (запрос Bus)
    BusInfo GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::vector<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;

    void SetRenderSettings(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                           geo::Coordinates bus_label_offset, int stop_label_font_size, geo::Coordinates stop_label_offset, svg::Color underlayer_color, double underlayer_width,
                           std::vector<svg::Color> color_palette);

    // Этот метод будет нужен в следующей части итогового проекта
    std::stringstream RenderMap() const;

    void SetRoutingSettings(int bus_wait_time, double bus_velocity);
    void CreateRoute(int bus_wait_time, double bus_velocity);
    transport_router::RouteInfo GetRoute(std::string_view from, std::string_view to) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    transport_router::TransportRouter router_;
};