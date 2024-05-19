#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    
    input_reader::Read(cin, catalogue);
    statistic::PrintStat(catalogue, cin, cout);
}