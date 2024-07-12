#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader{
    class JsonReader{
    public:
        JsonReader(RequestHandler& requestHandler);

        void ReadRequests(std::istream& in);
        void HandleBaseRequests();
        void HandleRoutingSettings();
        void HandleRenderSettings();
        void HandleStatRequest(std::ostream& out);

    private:
        RequestHandler& requestHandler_;
        json::Array base_requests;
        json::Array stat_requests;
        json::Dict render_settings;
        json::Dict routing_settings;
    };
}