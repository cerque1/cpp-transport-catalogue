#include "stat_reader.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <iomanip>
namespace statistic {
    namespace details {
        void PrintBusStat(std::string_view bus_name, const transport_catalogue::TransportCatalogue& catalogue, std::ostream& output){
            output << "Bus " << bus_name << ": ";
            transport_catalogue::BusInfo info = catalogue.GetInfo(bus_name);
            if(info == transport_catalogue::BusInfo{}){
                output << "not found\n";
                return;
            }
            
            output << info.stops_route << " stops on route, " << info.unique_stops << " unique stops, " << std::setprecision(6) << info.length << " route length\n";
        }

        void PrintStopStat(std::string_view stop_name, const transport_catalogue::TransportCatalogue& catalogue, std::ostream& output){
            output << "Stop " << stop_name << ":";
            if(catalogue.FindStop(stop_name) == nullptr){
                output << " not found\n";
                return;
            }
            auto stop_info = catalogue.GetStopInfo(stop_name);
            if(stop_info.empty()){
                output << " no buses\n";
                return;
            }

            output << " buses";

            std::sort(stop_info.begin(), stop_info.end());
    
            for(auto bus_name : stop_info){
                output << " " << bus_name;
            }
            output << "\n";
        }
    }


    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::string_view request,
                        std::ostream& output) {
        auto first_comm = request.find_first_not_of(" ");
        if(first_comm == request.npos){
            return;
        }
        auto last_comm = request.find_first_of(" ", first_comm);
        auto comm = request.substr(first_comm, last_comm);
        
        if(comm == "Bus"){
            auto bus_name = request.substr(last_comm + 1, request.find_last_not_of(" "));
            details::PrintBusStat(bus_name, catalogue, output);
        }
        else if(comm == "Stop"){
            auto stop_name = request.substr(last_comm + 1, request.find_last_not_of(" "));
            details::PrintStopStat(stop_name, catalogue, output);
        }
    }

    void PrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::istream& in, std::ostream& output){
        int stat_request_count;
        in >> stat_request_count >> std::ws;
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            std::getline(in, line);
            ParseAndPrintStat(catalogue, line, output);
        }
    }
}