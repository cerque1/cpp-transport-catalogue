#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;

    RequestHandler requestHandler(catalogue, renderer);
    json_reader::JsonReader json_reader_(requestHandler);
    json_reader_.ReadDataBaseRequest(std::cin);
    json_reader_.ReadRenderSettings();
    json_reader_.StatRequestHandle(std::cout);
}