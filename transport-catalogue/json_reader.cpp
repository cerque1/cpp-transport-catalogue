#include "json_reader.h"
#include "json.h"
#include "domain.h"
#include "geo.h"

#include <algorithm>
#include <string_view>
#include <sstream>

namespace json_reader{
    using namespace json;

    namespace details{

        //Создает остановку по запросу
        void ReadStop(const Dict& stop_parameters, RequestHandler& requestHandler){
            geo::Coordinates coord = {stop_parameters.at("latitude").AsDouble(), stop_parameters.at("longitude").AsDouble()};
            std::string name = stop_parameters.at("name").AsString();
            requestHandler.AddStop(name, coord);

            Dict stops_to_distance = stop_parameters.at("road_distances").AsMap();
            for(auto [stop, distance] : stops_to_distance){
                requestHandler.AddDistance(name, stop, distance.AsInt());
            }
        }

        //Создает маршрут по запросу
        void ReadBus(const Dict& bus_parameters, RequestHandler& requestHandler){
            std::string name = bus_parameters.at("name").AsString();
            std::vector<std::string> stops;
            for(auto stop_name : bus_parameters.at("stops").AsArray()){
                stops.push_back(stop_name.AsString());
            }
            requestHandler.AddBus(name, std::vector<std::string_view>(stops.begin(), stops.end()), bus_parameters.at("is_roundtrip").AsBool());
        }


        void GetStopInfo(Array& out, const Dict& request_info, const RequestHandler& requestHandler){
            Dict dict_answer;
            dict_answer["request_id"] = request_info.at("id");
            std::string name = request_info.at("name").AsString();
            if(requestHandler.FindStop(name) == nullptr){
                dict_answer["error_message"] = "not found";
                out.emplace_back(dict_answer);
                return;
            }

            std::vector<std::string_view> buses_to_stop = requestHandler.GetBusesByStop(name);
            std::sort(buses_to_stop.begin(), buses_to_stop.end());

            Array buses;
            for(auto i : buses_to_stop){
                buses.push_back(Node{std::string(i)});
            }
            dict_answer["buses"] = Node{buses};
            out.emplace_back(dict_answer);
        }

        void GetBusInfo(Array& out, const Dict& request_info, const RequestHandler& requestHandler){
            Dict dict_answer;
            dict_answer["request_id"] = request_info.at("id");
            if(requestHandler.FindBus(request_info.at("name").AsString()) == nullptr){
                dict_answer["error_message"] = Node{"not found"};
                out.emplace_back(dict_answer);
                return;
            } 

            BusInfo bus_info = requestHandler.GetBusStat(request_info.at("name").AsString());

            dict_answer["curvature"] = Node{bus_info.curvature};
            dict_answer["route_length"] = Node{bus_info.length};
            dict_answer["stop_count"] = Node{bus_info.stops_route};
            dict_answer["unique_stop_count"] = Node{bus_info.unique_stops};
            out.emplace_back(dict_answer);
        }

        void GetMap(Array& out, const RequestHandler& requestHandler_, int request_id){
            std::stringstream result;
            result << "\"";
            result << requestHandler_.RenderMap().str();
            result << "\"";
            Dict dict_answer;
            dict_answer["request_id"] = request_id;
            dict_answer["map"] = json::Load(result).GetRoot().AsString();
            out.emplace_back(dict_answer);
        }

        //Обрабатывает запросы к базе данных
        void DataRequestHandle(const Array& data_requests, RequestHandler& requestHandler){
            for(size_t i = 0; i < data_requests.size(); i++){
                if(data_requests.at(i).AsMap().at("type") == "Stop"){
                    ReadStop(data_requests.at(i).AsMap(), requestHandler);
                }
                else if(data_requests.at(i).AsMap().at("type") == "Bus"){
                    ReadBus(data_requests.at(i).AsMap(), requestHandler);
                }
            }
        }

        //Обрабатывает запросы на получение статистики
        void StatRequestHandle(Array& out, const Array& stat_requests, const RequestHandler& requestHandler){
            for(size_t i = 0; i < stat_requests.size(); i++){
                if(stat_requests.at(i).AsMap().at("type") == "Stop"){
                    GetStopInfo(out, stat_requests.at(i).AsMap(), requestHandler);
                }
                else if(stat_requests.at(i).AsMap().at("type") == "Bus"){
                    GetBusInfo(out, stat_requests.at(i).AsMap(), requestHandler);
                }
                else if(stat_requests.at(i).AsMap().at("type") == "Map"){
                    GetMap(out, requestHandler, stat_requests.at(i).AsMap().at("id").AsInt());
                }
            }
        }

        void SortRequest(Array& requests){
            size_t end = static_cast<size_t>(requests.size());
            for(size_t i = 0; i < end; i++){
                for(size_t j = 0; j < end - 1; j++){
                    if(requests[j].AsMap().at("type").AsString() == "Bus" && requests[j + 1].AsMap().at("type").AsString() == "Stop"){
                        std::swap(requests[j], requests[j + 1]);
                    }
                }
            }
        }

        svg::Color NodeToColor(const Node& node){
            svg::Color color;
            if(node.IsArray()){
                Array color_as_array = node.AsArray();
                if(color_as_array.size() == 3){
                    color = svg::Rgb{static_cast<unsigned short>(color_as_array.at(0).AsInt()), static_cast<unsigned short>(color_as_array.at(1).AsInt())
                                    , static_cast<unsigned short>(color_as_array.at(2).AsInt())};
                }
                else if(color_as_array.size() == 4){
                    color = svg::Rgba{static_cast<unsigned short>(color_as_array.at(0).AsInt()), static_cast<unsigned short>(color_as_array.at(1).AsInt()), 
                                      static_cast<unsigned short>(color_as_array.at(2).AsInt()), color_as_array.at(3).AsDouble()};
                }
            }
            else if(node.IsString()){
                color = node.AsString();
            }
            return color;
        }
    }

    JsonReader::JsonReader(RequestHandler& requestHandler) : requestHandler_(requestHandler){}

    //Вызывает обработчики определенных запросов
    void JsonReader::ReadDataBaseRequest(std::istream& in){
        Dict requests = Load(in).GetRoot().AsMap();
        stat_requests = requests.at("stat_requests").AsArray();
        render_settings = requests.at("render_settings").AsMap();

        if(requests.count("base_requests")){
            Array& value_as_array = const_cast<Array&>(requests.at("base_requests").AsArray());
            details::SortRequest(value_as_array);
            details::DataRequestHandle(value_as_array, requestHandler_);
        }
    }

    void JsonReader::ReadRenderSettings(){
        if(!render_settings.empty()){
            double width = render_settings["width"].AsDouble();
            double height = render_settings["height"].AsDouble();
            double padding = render_settings["padding"].AsDouble();

            double line_width_ = render_settings["line_width"].AsDouble();
            double stop_radius_ = render_settings["stop_radius"].AsDouble();

            int bus_label_font_size_ = render_settings["bus_label_font_size"].AsInt();
            Array coords = render_settings["bus_label_offset"].AsArray();
            geo::Coordinates bus_label_offset_ = {coords.at(0).AsDouble(), coords.at(1).AsDouble()};

            int stop_label_font_size_ = render_settings["stop_label_font_size"].AsInt();
            coords = render_settings["stop_label_offset"].AsArray();
            geo::Coordinates stop_label_offset_ = {coords.at(0).AsDouble(), coords.at(1).AsDouble()};

            svg::Color underlayer_color_ = details::NodeToColor(render_settings["underlayer_color"]);
            
            double underlayer_width_ = render_settings["underlayer_width"].AsDouble();

            Array color_palette_node = render_settings["color_palette"].AsArray();
            std::vector<svg::Color> color_palette_;
            for(const Node& node : color_palette_node){
                color_palette_.push_back(details::NodeToColor(node));
            }

            requestHandler_.SetRenderSettings(width, height, padding, line_width_, stop_radius_, bus_label_font_size_, bus_label_offset_, 
                                           stop_label_font_size_, stop_label_offset_, underlayer_color_, underlayer_width_, std::move(color_palette_));
        }
    }

    void JsonReader::StatRequestHandle(std::ostream& out){
        Array answer;
        if(!stat_requests.empty()){
            details::StatRequestHandle(answer, stat_requests, requestHandler_);
        }

        Print(Document{answer}, out);
    }
}