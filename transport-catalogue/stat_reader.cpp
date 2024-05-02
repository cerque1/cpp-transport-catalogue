#include "stat_reader.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <iomanip>

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto first_comm = request.find_first_not_of(" ");
    if(first_comm == request.npos){
        return;
    }
    auto last_comm = request.find_first_of(" ", first_comm);
    auto comm = request.substr(first_comm, last_comm);
    
    if(comm == "Bus"){
        auto bus_name = request.substr(last_comm + 1, request.find_last_not_of(" "));

        output << "Bus " << bus_name << ": ";
        transport_catalogue::BusInfo info = tansport_catalogue.GetInfo(bus_name);
        if(info == transport_catalogue::BusInfo{}){
            output << "not found\n";
            return;
        }
        
        output << info.R << " stops on route, " << info.U << " unique stops, " << std::setprecision(6) << info.L << " route length\n";
    }
    else if(comm == "Stop"){
        auto stop_name = request.substr(last_comm + 1, request.find_last_not_of(" "));

        output << "Stop " << stop_name << ":";
        auto stop_info = tansport_catalogue.GetStopInfo(stop_name);
        if(stop_info.first.empty()){
            output << " not found\n";
            return;
        }
        else if(stop_info.second.empty()){
            output << " no buses\n";
            return;
        }

        output << " buses";

        std::sort(stop_info.second.begin(), stop_info.second.end());
  
        for(auto i : stop_info.second){
            output << " " << i;
        }
        output << "\n";
    }
}