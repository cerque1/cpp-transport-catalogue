#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <unordered_set>

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coord){
        stops_.push_back({name, std::move(coord)});
        stops_points_[stops_.back().name] = &stops_.back();
        stop_to_buses_[stops_.back().name] = {};
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops_name, bool is_round){
        std::vector<const Stop*> stops;
        buses_.push_back({name, {}, is_round});
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

    void TransportCatalogue::AddDistance(const std::string& stop_from, const std::string& stop_to, int distance){
        if(!stop_from.empty() && !stop_from.empty()){
            distances_[{stop_from, stop_to}] = distance;
        }
    }

    const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetBusesPoints() const{
        return buses_points_;
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

    int TransportCatalogue::FindDistance(const std::string& stop_from, const std::string& stop_to) const{
        if(distances_.count({stop_from, stop_to})){
            return distances_.at({stop_from, stop_to});
        }
        if(distances_.count({stop_to, stop_from})){
            return distances_.at({stop_to, stop_from});
        }
        return 0;
    }

    BusInfo TransportCatalogue::GetInfo(const Bus* bus) const{
        BusInfo info;
        if(bus == nullptr){
            return BusInfo{};
        }

        info.stops_route = static_cast<int>(bus->stops.size());
        info.unique_stops = static_cast<int>(std::unordered_set<const Stop*>(bus->stops.begin(), bus->stops.end()).size());

        double length = 0;
        geo::Coordinates last_coord = {};
        for(const auto i : bus->stops){
            if(last_coord != geo::Coordinates{}){
                length += geo::ComputeDistance(last_coord, i->coordinates);
            }
            last_coord = const_cast<Stop*>(i)->coordinates;
        }

        info.length = 0;
        for(long long unsigned int i = 0; i < bus->stops.size() - 1; i++){
            info.length += FindDistance(bus->stops.at(i)->name, bus->stops.at(i + 1)->name);
        }

        if(bus->is_round == false){
            for(long long unsigned int i = bus->stops.size() - 1; i > 0; i--){
                info.length += FindDistance(bus->stops.at(i)->name, bus->stops.at(i - 1)->name);
            }
            info.stops_route = info.stops_route * 2 - 1;
            length *= 2;
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