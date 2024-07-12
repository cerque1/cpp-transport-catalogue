#include "json_reader.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);

    RequestHandler requestHandler(catalogue, renderer, router);
    json_reader::JsonReader json_reader_(requestHandler);
    json_reader_.ReadRequests(std::cin);
    json_reader_.HandleBaseRequests();
    json_reader_.HandleRoutingSettings();
    json_reader_.HandleRenderSettings();
    json_reader_.HandleStatRequest(std::cout);
}