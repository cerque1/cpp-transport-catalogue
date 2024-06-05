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

        void ReadDataBaseRequest(std::istream& in);
        void ReadRenderSettings();
        void StatRequestHandle(std::ostream& out);

    private:
        RequestHandler& requestHandler_;
        json::Array stat_requests;
        json::Dict render_settings;
    };
}