#pragma once

#include "geo.h"

#include <string>
#include <vector>

struct Stop {
    std::string name;
    geo::Coordinates coordinates;

    size_t operator()(const Stop& stop) const;

    bool operator==(const Stop& stop) const;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_round;

    size_t operator()(const Bus* bus) const;

    bool operator==(const Bus& bus) const;
};

struct BusInfo {
    int stops_route;
    int unique_stops;
    int length; 
    double curvature;

    bool operator==(const BusInfo& info);
};