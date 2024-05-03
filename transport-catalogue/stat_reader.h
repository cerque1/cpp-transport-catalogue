#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
#include "geo.h"

namespace statistic {
    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::string_view request,
                       std::ostream& output);

    void PrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::istream& in, std::ostream& output);
}