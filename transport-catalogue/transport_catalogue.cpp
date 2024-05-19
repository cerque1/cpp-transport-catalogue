#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <unordered_set>

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const std::string& name, geography::Coordinates coord){
        stops_.push_back({name, std::move(coord)});
        stops_points_[stops_.back().name] = &stops_.back();
        stop_to_buses_[stops_.back().name] = {};
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops_name){
        std::vector<const Stop*> stops;
        buses_.push_back({name, {}});
        for(const auto& stop_name : stops_name){
            auto stop = FindStop(stop_name);
            stops.push_back(stop);
            if(std::count(stop_to_buses_.at(stops.back()->name).begin(), stop_to_buses_.at(stops.back()->name).end(), buses_.back().name) == 0){
                stop_to_buses_[stops.back()->name].push_back(buses_.back().name);
            }
        }
        buses_.back().stops = std::move(stops);
        buses_points_[buses_.back().name] = &buses_.back();
    }

    void TransportCatalogue::AddDistance(const std::string& name, std::vector<std::pair<std::string, int>> stops_to_distance){
        if(!stops_to_distance.empty()){
            for(auto stop_to_distance : stops_to_distance){
                distances_[{name, stop_to_distance.first}] = stop_to_distance.second;
            }
        }
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

    int TransportCatalogue::FindDistance(const std::pair<std::string, std::string>& other) const{
        if(distances_.count(other)){
            return distances_.at(other);
        }
        if(distances_.count({other.second, other.first})){
            return distances_.at({other.second, other.first});
        }
        return 0;
    }

    BusInfo TransportCatalogue::GetInfo(std::string_view name) const{
        BusInfo info;
        const Bus* find_bus = FindBus(name);
        if(find_bus == nullptr){
            return BusInfo{};
        }

        info.stops_route = static_cast<int>(find_bus->stops.size());
        info.unique_stops = static_cast<int>(std::unordered_set<const Stop*>(find_bus->stops.begin(), find_bus->stops.end()).size());

        info.length = 0;
        for(long long unsigned int i = 0; i < find_bus->stops.size() - 1; i++){
            info.length += FindDistance({find_bus->stops.at(i)->name, find_bus->stops.at(i + 1)->name});
        }

        double length = 0;
        geography::Coordinates last_coord = {};
        for(const auto i : find_bus->stops){
            if(last_coord != geography::Coordinates{}){
                length += geography::ComputeDistance(last_coord, i->coordinates);
            }
            last_coord = const_cast<Stop*>(i)->coordinates;
        }

        info.curvature = info.length / length;
        return info;
    }

    std::vector<std::string_view> TransportCatalogue::GetStopInfo(std::string_view name) const{
        if(stop_to_buses_.count(name)){
            return stop_to_buses_.at(name);
        }
        return {};
    }
}