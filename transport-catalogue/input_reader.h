#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <deque>

#include "geo.h"
#include "transport_catalogue.h"

namespace input_reader {
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    class InputReader {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        void SortCommands();
        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    private:
        std::deque<CommandDescription> commands_;
    };

    void Read(std::istream& in, transport_catalogue::TransportCatalogue& catalogue);
}