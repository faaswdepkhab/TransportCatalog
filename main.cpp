#include <iostream>

#include "json.h"
#include "svg.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

renderer::RenderSettings GetRenderSettings() {
    renderer::RenderSettings result;
    
    result.width = 600.0;
    result.height = 400.0;
    
    result.padding = 50.0;
    
    result.line_width = 14.0;
    result.stop_radius = 5.0;
    
    result.bus_label_font_size = 20;
    result.bus_label_offset = svg::Point(7.0, 15.0);
    
    result.stop_label_font_size = 20;
    result.stop_label_offset = svg::Point(7.0, -3.0);
    
    result.underlayer_color = "rgba(255, 255, 255, 0.85)";
    result.underlayer_width = 3.0;
    
    result.color_palette = {
        "green",
        "rgb(255, 160, 0)",
        "red"
    };
        
    return result;
}

int main() {
    auto cataloge = transport_cataloge::TransportCatalogue();
    renderer::TransportCatalogeRendererSVG renderer(cataloge);
    TransportRouter router(cataloge);
    
    auto handle = TransportCatalogeHandler(cataloge, renderer, router);
    auto doc = json::Load(std::cin);
    auto reader_ = reader::JsonReader(doc);
    reader_.SaveDataToCataloge(handle);
    reader_.SetRenderSettings(handle);
    reader_.SetRouterSettings(handle);
    reader_.RunQuery(handle);
    auto result = reader_.GetResultQuery();
    json::Print(result, std::cout);
    
    //std::cout <<"Finish" << std::endl;

    return 0;
}