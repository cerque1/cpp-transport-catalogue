#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>


namespace input_reader {
    namespace details {
        /**
         * Парсит строку вида "10.123,  -30.1837, ..." и возвращает пару координат (широта, долгота)
         */
        geography::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return {nan, nan};
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);
            auto comma2 = str.find(',', not_space2);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

            return {lat, lng};
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        std::vector<std::pair<std::string, int>> ParseDistanceToStop(std::string_view str){
            auto comma = str.find_first_of(',');
            str.find_first_of(',', comma + 1) == str.npos ? str = str.substr(str.size()) : str = str.substr(str.find_first_of(',', comma + 1) + 1);

            std::vector<std::pair<std::string, int>> stop_to_distance;
            auto str_pos = str.find_first_not_of(" ");
            while(str_pos != str.npos){
                auto end_num = str.find_first_of("m", str_pos);
                int distance = std::stoi(std::string(str.substr(str_pos, end_num - str_pos)));

                auto begin_str = str.find_first_not_of(" ", str.find_first_of("to", end_num) + 2);
                auto end_str = str.find_first_of(",") == str.npos ? str.find_last_not_of(" ") + 1 : str.find_first_of(",");
                std::string stop_name = std::string(str.substr(begin_str, end_str - begin_str));

                stop_to_distance.push_back({std::string(Trim(stop_name)), distance});
                str = str.substr(end_str == str.size() ? end_str : end_str + 1);
                str_pos = str.find_first_not_of(" ");
            }
            return stop_to_distance;
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1))};
        }
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = details::ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::SortCommands(){
        std::sort(commands_.begin(), commands_.end(), [](const CommandDescription& lhs, const CommandDescription& rhs){
            return lhs.command == "Stop" && rhs.command == "Bus";
        });
    }

    void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
        for(auto command : commands_){
            if(command.command == "Stop"){
                catalogue.AddStop(command.id, details::ParseCoordinates(command.description));   
                catalogue.AddDistance(command.id, details::ParseDistanceToStop(command.description));
            }
            else if(command.command == "Bus"){
                catalogue.AddBus(command.id, details::ParseRoute(command.description));
            }
        }
    }

    void Read(std::istream& in, transport_catalogue::TransportCatalogue& catalogue){
        int base_request_count;
        in >> base_request_count >> std::ws;

        {
            input_reader::InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                std::getline(in, line);
                reader.ParseLine(line);
            }
            reader.SortCommands();
            reader.ApplyCommands(catalogue);
        }
    }
}