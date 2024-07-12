#pragma once
#include <string_view>
#include <string>
#include <sstream>
#include <optional>
#include <variant>

#include "transport_catalogue.h"
#include "json_builder.h"
#include "graph.h"
#include "router.h"

namespace transport_router{

struct WaitInfo {
    std::string type_;
    double time_;

    WaitInfo(std::string type, double time) : type_(type), time_(time){}
};

struct WaitStopInfo : WaitInfo {
    std::string stop_name_;

    WaitStopInfo(std::string type, double time, std::string stop_name) : WaitInfo(type, time), stop_name_(stop_name){}
};

struct WaitBusInfo : WaitInfo {
    int span_count_;
    std::string bus_name_;

    WaitBusInfo(std::string type, double time, int span_count, std::string bus_name) : WaitInfo(type, time), span_count_(span_count), bus_name_(bus_name){}
};

struct RouteInfo{
    std::optional<double> total_time_;
    std::vector<std::variant<WaitBusInfo, WaitStopInfo>> items_;

    RouteInfo(std::optional<double> total_time, std::vector<std::variant<WaitBusInfo, WaitStopInfo>> items) : total_time_(total_time), items_(items){}
};

class TransportRouter{
public:
    TransportRouter() = default;
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue){}

    void SetRoutingSettings(int bus_wait_time, double bus_velocity);
    RouteInfo GetRoute(std::string_view from, std::string_view to) const;
    void CreateGraph();

private:
    int bus_wait_time_;
    double bus_velocity_;
    graph::Router<double> router_;
    const transport_catalogue::TransportCatalogue& catalogue_;
};

}