#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"

#include "geo.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialization {
    
struct Bus {
    std::string_view Number;
    bool IsLoop;
    std::vector<int> id_stops;
};  
    
struct Stop {
    std::string_view Name;
    geo::Coordinates Coord;
};
    
class TransportCatalogSerialization {
public:
    TransportCatalogSerialization() = default;
    
    bool SaveToFile(std::string fileName, const transport_cataloge::TransportCatalogue &catalog, const renderer::TransportCatalogeRendererSVG &render, const TransportRouter &router);
    
    bool LoadFromFile(std::string fileName, transport_cataloge::TransportCatalogue &catalog, renderer::TransportCatalogeRendererSVG &render, TransportRouter &router);
    //
private:
    // буфер для остановок
    std::unordered_map<int, std::string_view> id_stop;
    std::unordered_map<std::string_view, int > stop_id;
    // буфер для маршрутов
    //std::vector<BusRoute> buses;
    // буфер для расстояний
    //std::unordered_map<int, std::unordered_map<int, int>> distances;
    
    transport_catalogue_serialize::Catalogue catalog_proto;
    
    // сброс внутренних буферов
    void Reset();
    
    void CatalogeToProto(const transport_cataloge::TransportCatalogue &catalog);
    void ProtoToCatalog(transport_cataloge::TransportCatalogue &catalog);
    
    void ProtoToRenderer(renderer::TransportCatalogeRendererSVG &render);
    void RendererToProto(const renderer::TransportCatalogeRendererSVG &render);
    
    transport_catalogue_serialize::Coordinates CoordToProto(const geo::Coordinates &coord) const;
    geo::Coordinates ProtoToCoord(const transport_catalogue_serialize::Coordinates &coord_proto) const;
    
    transport_catalogue_serialize::Stop GetProtoStop(int id_stop, domain::Stop &stop);
    transport_catalogue_serialize::Bus GetProtoBus(domain::BusInfo busInfo);
    
    void LoadStopsFromProto(transport_cataloge::TransportCatalogue &catalog);
    void LoadBusesFromProto(transport_cataloge::TransportCatalogue &catalog);
    void LoadDistancesProto(transport_cataloge::TransportCatalogue &catalog);
    
    domain::RoutesStop GetStopRouteFromProto(const transport_catalogue_serialize::Stop &stop_proto);
    domain::BusRoute GetBusRouteFromProto(const transport_catalogue_serialize::Bus &bus_proto);
};
    
} // end namespace serialization
    