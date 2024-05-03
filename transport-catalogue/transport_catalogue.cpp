#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <unordered_set>

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const std::string& name, geography::Coordinates coord){
        stops_.push_back({name, std::move(coord)});
        stops_points_[stops_.back().name] = &stops_.back();
        stop_to_buses[stops_.back().name] = {};
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops_name){
        std::deque<const Stop*> stops;
        buses_.push_back({name, {}});
        for(const auto& stop_name : stops_name){
            auto stop = FindStop(stop_name);
            stops.push_back(stop);
            if(std::count(stop_to_buses.at(stops.back()->name).begin(), stop_to_buses.at(stops.back()->name).end(), buses_.back().name) == 0){
                stop_to_buses[stops.back()->name].push_back(buses_.back().name);
            }
        }
        buses_.back().stops = std::move(stops);
        buses_points_[buses_.back().name] = &buses_.back();
    }

    const Bus* TransportCatalogue::FindBus(std::string_view name) const{
        if(buses_points_.count(name)){
            return buses_points_.at(name);
        }
        return nullptr;
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const{
        if(stops_points_.count(name)){
            return stops_points_.at(name);
        }
        return nullptr;
    }

    const BusInfo TransportCatalogue::GetInfo(std::string_view name) const{
        BusInfo info;
        const Bus* find_bus = FindBus(name);
        if(find_bus == nullptr){
            return BusInfo{};
        }

        info.stops_route = static_cast<int>(find_bus->stops.size());
        info.unique_stops = static_cast<int>(std::unordered_set<const Stop*>(find_bus->stops.begin(), find_bus->stops.end()).size());

        info.length = 0;
        geography::Coordinates last_coord = {};
        for(const auto i : find_bus->stops){
            if(last_coord != geography::Coordinates{}){
                info.length += geography::ComputeDistance(last_coord, i->coordinates);
            }
            last_coord = const_cast<Stop*>(i)->coordinates;
        }
        return info;
    }

    const std::vector<std::string_view> TransportCatalogue::GetStopInfo(std::string_view name) const{
        if(stop_to_buses.count(name)){
            return stop_to_buses.at(name);
        }
        return {};
    }
}