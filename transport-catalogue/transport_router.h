#pragma once
#include <string_view>
#include <string>
#include <sstream>
#include <optional>

#include "transport_catalogue.h"
#include "json_builder.h"
#include "graph.h"
#include "router.h"

namespace transport_router{

struct RouteInfo{
    std::optional<double> total_time_;
    json::Array items_;

    RouteInfo(std::optional<double> total_time, json::Array items) : total_time_(total_time), items_(items){}
};

class TransportRouter{
public:
    TransportRouter() = default;

    void SetRoutingSettings(int bus_wait_time, double bus_velocity);
    RouteInfo GetRoute(size_t from, size_t to, const transport_catalogue::TransportCatalogue& catalogue) const;
    void CreateGraph(const transport_catalogue::TransportCatalogue& catalogue);

private:
    int bus_wait_time_;
    double bus_velocity_;
    graph::Router<double> router_;
};

}