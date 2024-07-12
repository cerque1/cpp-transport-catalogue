#include <algorithm>
#include <deque>
#include <vector>
#include <optional>

#include "transport_router.h"
#include "router.h"
#include "domain.h"

namespace transport_router{
using namespace json;

const double CoefficientSpeedConversion = 16.66666;

namespace details{
    template <typename It>
    void ParseBusToEdges(It begin, It end, double bus_velocity, const transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph){
        for(auto iter = begin; iter != end; iter++){
            std::string first_stop_name = (*iter)->name;
            std::string last_stop_name = first_stop_name;
            std::optional<int> first_index = catalogue.FindStopIndex(first_stop_name);
            double distance_sum = 0;
            if(!first_index){
                throw std::runtime_error("stop is not found");
            }
            first_index = first_index.value() * 2 + 1;
            for(auto j_iter = next(iter, 1); j_iter != end; j_iter++){
                std::string second_stop_name = (*j_iter)->name;
                std::optional<int> second_index = catalogue.FindStopIndex(second_stop_name);
                distance_sum += catalogue.FindDistance(last_stop_name, second_stop_name);
                if(!first_index){
                    throw std::runtime_error("stop is not found");
                }
                second_index = second_index.value() * 2;
                graph.AddEdge(graph::Edge<double>(static_cast<size_t>(*first_index), static_cast<size_t>(*second_index), distance_sum / static_cast<double>(bus_velocity * CoefficientSpeedConversion)));
                last_stop_name = second_stop_name;
            }
            distance_sum = 0;
        }
    }
}

void TransportRouter::SetRoutingSettings(int bus_wait_time, double bus_velocity){
    bus_wait_time_ = bus_wait_time;
    bus_velocity_ = bus_velocity;
}

RouteInfo TransportRouter::GetRoute(std::string_view from, std::string_view to) const{
    size_t from_index = static_cast<size_t>(catalogue_.FindStopIndex(from).value());
    size_t to_index = static_cast<size_t>(catalogue_.FindStopIndex(to).value());

    auto route = router_.BuildRoute(from_index*2, to_index*2);
    std::vector<std::variant<WaitBusInfo, WaitStopInfo>> wait;
    double total_time = 0;
    const graph::DirectedWeightedGraph<double>& graph = router_.GetGraph();

    if(route.has_value()){
        for(auto edgeid : route.value().edges){
            auto edge = graph.FindEdge(edgeid);
            std::string stop_name_from = catalogue_.GetStops().at(edge.from/2).name;
            std::string stop_name_to = catalogue_.GetStops().at(edge.to/2).name;
            total_time += edge.weight;

            if(edge.from % 2 == 0 && edge.to - edge.from == 1){
                wait.emplace_back(WaitStopInfo("Wait", edge.weight, catalogue_.GetStops().at(edge.from/2).name));
            }
            else{
                std::vector<std::string_view> v1 = catalogue_.FindStopToBus(catalogue_.GetStops().at(edge.from/2).name);
                std::vector<std::string_view> v2 = catalogue_.FindStopToBus(catalogue_.GetStops().at(edge.to/2).name);

                auto [bus_name, stops_count] = catalogue_.FindBusNameByTwoStopsAndDistance(stop_name_from, stop_name_to, edge.weight * bus_velocity_ * CoefficientSpeedConversion);
                wait.emplace_back(WaitBusInfo("Bus", edge.weight, stops_count, bus_name));
            }
        }
        return RouteInfo{total_time, wait};
    }
    return RouteInfo{std::nullopt, {}};
}

void TransportRouter::CreateGraph(){
    const std::deque<Bus>& buses = catalogue_.GetBuses();
    graph::DirectedWeightedGraph<double> graph(catalogue_.GetStopsCount()*2);

    for(size_t i = 0; i < catalogue_.GetStopsCount()*2; i += 2){
        graph.AddEdge(graph::Edge<double>(i, static_cast<size_t>(i + 1), bus_wait_time_));
    }
    
    for(auto bus : buses){
        if(bus.is_round){
            details::ParseBusToEdges(bus.stops.begin(), bus.stops.end(), bus_velocity_, catalogue_, graph);
        }
        else{
            details::ParseBusToEdges(bus.stops.begin(), bus.stops.end(), bus_velocity_, catalogue_, graph);
            details::ParseBusToEdges(bus.stops.rbegin(), bus.stops.rend(), bus_velocity_, catalogue_, graph);
        }
    }

    router_ = graph::Router(std::move(graph));
}

}