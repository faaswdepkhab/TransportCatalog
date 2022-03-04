#pragma once

#include <string_view>
#include <string>

#include "svg.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "json.h"

class TransportCatalogeHandler {
public:
    TransportCatalogeHandler(transport_cataloge::TransportCatalogue &db
        , renderer::TransportCatalogeRendererSVG &renderer, TransportRouter &router): db_(db), renderer_(renderer), router_(router) {}

    std::string RenderMap() const;
    void SetRenderSettings(renderer::RenderSettings settings);
    void SetRouterSettings(double bus_velocity, int bus_wait_time);
    
    void AddStop(domain::RoutesStop &s);
    void AddBus(domain::BusRoute &b);
    void AddDistance(std::string_view src, std::string_view dest, int distance);
    
    domain::BusInfo GetBusInfo(const std::string &Number) const;
    domain::StopInfo GetStopInfo(const std::string &Name) const;
    json::Dict GetRoute(std::string from, std::string to);
    
private:
    transport_cataloge::TransportCatalogue& db_;
    renderer::TransportCatalogeRendererSVG &renderer_;
    TransportRouter &router_;
};