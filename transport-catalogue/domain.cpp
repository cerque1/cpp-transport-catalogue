#include "domain.h"

#include <tuple>

size_t Stop::operator()(const Stop& stop) const {
    return std::hash<std::string>{}(stop.name);
}

bool Stop::operator==(const Stop& stop) const{
    return name == stop.name && coordinates.lat == stop.coordinates.lat && coordinates.lng == stop.coordinates.lng;
}

size_t Bus::operator()(const Bus* bus) const {
    return std::hash<std::string>{}(bus->name);
}

bool Bus::operator==(const Bus& bus) const{
    return bus.name == name && std::equal(stops.begin(), stops.end(), bus.stops.begin(), bus.stops.end());
}

bool BusInfo::operator==(const BusInfo& info){
    return std::tie(info.length, info.stops_route, info.unique_stops) == std::tie(length, stops_route, unique_stops);
}