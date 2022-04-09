#include <fstream>
#include <iostream>
#include <string_view>

#include "serialization.h"
#include "json.h"
#include "svg.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void make_base() {
    auto cataloge = transport_cataloge::TransportCatalogue();
    renderer::TransportCatalogeRendererSVG renderer(cataloge);
    TransportRouter router(cataloge);
    serialization::TransportCatalogSerialization serializator;
    
    auto handle = TransportCatalogeHandler(cataloge, renderer, router, serializator);
    auto doc = json::Load(std::cin);
    auto reader_ = reader::JsonReader(doc);
    reader_.SaveDataToCataloge(handle);
    reader_.SetRenderSettings(handle);
    reader_.SetRouterSettings(handle);
    reader_.SaveToFile(handle);
}

void process_requests() {
    auto cataloge = transport_cataloge::TransportCatalogue();
    renderer::TransportCatalogeRendererSVG renderer(cataloge);
    TransportRouter router(cataloge);
    serialization::TransportCatalogSerialization serializator;
    
    auto handle = TransportCatalogeHandler(cataloge, renderer, router, serializator);
    auto doc = json::Load(std::cin);
    auto reader_ = reader::JsonReader(doc);
    reader_.LoadFromFile(handle);
    reader_.RunQuery(handle);
    auto result = reader_.GetResultQuery();
    json::Print(result, std::cout);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        make_base();
    } else if (mode == "process_requests"sv) {
        process_requests();
    } else {
        PrintUsage();
        return 1;
    }
}