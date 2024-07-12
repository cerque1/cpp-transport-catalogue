#include <sstream>

#include "request_handler.h"
#include "domain.h"

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer) : db_(catalogue), renderer_(renderer){}

void RequestHandler::AddStop(const std::string& name, geo::Coordinates coord){
    db_.AddStop(name, coord);
}

void RequestHandler::AddBus(const std::string &name, const std::vector<std::string_view> &stops_name, bool is_round){
    db_.AddBus(name, stops_name, is_round);
}

void RequestHandler::AddDistance(const std::string &stop_from, const std::string &stop_to, int distance){
    db_.AddDistance(stop_from, stop_to, distance);
}

const Stop* RequestHandler::FindStop(std::string_view name) const{
    return db_.FindStop(name);
}

const Bus* RequestHandler::FindBus(std::string_view name) const{
    return db_.FindBus(name);
}

int RequestHandler::FindDistance(const std::string& stop_from, const std::string& stop_to) const{
    return db_.FindDistance(stop_from, stop_to);
}

// Возвращает информацию о маршруте (запрос Bus)
BusInfo RequestHandler::GetBusStat(const std::string_view& bus_name) const{
    return db_.GetInfo(db_.FindBus(bus_name));
}

// Возвращает маршруты, проходящие через
const std::vector<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
    return db_.GetStopInfo(stop_name);
}

void RequestHandler::SetRenderSettings(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                           geo::Coordinates bus_label_offset, int stop_label_font_size, geo::Coordinates stop_label_offset, svg::Color underlayer_color, double underlayer_width,
                           std::vector<svg::Color> color_palette){
    return renderer_.SetRenderSettings(width, height, padding, line_width, stop_radius, bus_label_font_size, bus_label_offset, 
                                           stop_label_font_size, stop_label_offset, underlayer_color, underlayer_width, color_palette);
}

std::stringstream RequestHandler::RenderMap() const{
    auto bus_points = db_.GetBusesPoints();
    renderer_.SetSphereProjector(bus_points);

    svg::Document doc;

    std::vector<const Bus*> buses;
    for(auto [bus_name, bus_pointer] : bus_points){
        buses.emplace_back(bus_pointer);
    }

    std::sort(buses.begin(), buses.end(), [](auto lhs, auto rhs){
        return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
    });

    doc.ConnectDocument(renderer_.RenderMap(buses));

    std::stringstream ss;
    doc.Render(ss);

    return ss;
}

void RequestHandler::CreateRoute(int bus_wait_time, double bus_velocity){
    router_.SetRoutingSettings(bus_wait_time, bus_velocity);
    router_.CreateGraph(db_);
}

transport_router::RouteInfo RequestHandler::GetRoute(std::string_view from, std::string_view to) const{
    if(from == to){
        return transport_router::RouteInfo{0, json::Array{}};
    }
    size_t from_index = static_cast<size_t>(db_.FindStopIndex(from).value());
    size_t to_index = static_cast<size_t>(db_.FindStopIndex(to).value());
    return router_.GetRoute(from_index, to_index, db_);
}
