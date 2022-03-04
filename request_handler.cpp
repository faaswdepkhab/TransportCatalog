#include "request_handler.h"

void TransportCatalogeHandler::AddStop(domain::RoutesStop &s) {
    db_.AddStop(s);
}

void TransportCatalogeHandler::AddBus(domain::BusRoute &b) {
    db_.AddBus(b);
}

void TransportCatalogeHandler::AddDistance(std::string_view src, std::string_view dest, int distance) {
    db_.AddDistance(src, dest, distance);
}

domain::BusInfo TransportCatalogeHandler::GetBusInfo(const std::string &Number) const {
    return db_.GetBusInfo(Number);
}

domain::StopInfo TransportCatalogeHandler::GetStopInfo(const std::string &Name) const {
    return db_.GetStopInfo(Name);
}

void TransportCatalogeHandler::SetRenderSettings(renderer::RenderSettings settings) {
    renderer_.SetRenderSettings(settings);
}

std::string TransportCatalogeHandler::RenderMap() const {
    renderer_.RenderMap();
    return renderer_.GetSVGResultAsString();
}

void TransportCatalogeHandler::SetRouterSettings(double bus_velocity, int bus_wait_time) {
    router_.BuildRouter(bus_velocity, bus_wait_time);
}

json::Dict TransportCatalogeHandler::GetRoute(std::string from, std::string to) {
    return router_.GetRoute(from , to);
}