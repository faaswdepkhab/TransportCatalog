#include "serialization.h"

#include <fstream>
#include <transport_router.pb.h>

using namespace std;

namespace serialization {

void TransportCatalogSerialization::Reset() {
    stop_id.clear();
    id_stop.clear();
    catalog_proto.Clear();
}
    
transport_catalogue_serialize::Coordinates TransportCatalogSerialization::CoordToProto(const geo::Coordinates &coord) const {
    transport_catalogue_serialize::Coordinates result;
    result.set_latitude(coord.lat);
    result.set_longitude(coord.lng);
    return result;
}
    
 geo::Coordinates TransportCatalogSerialization::ProtoToCoord(const transport_catalogue_serialize::Coordinates &coord_proto) const {
    geo::Coordinates result;
    result.lat = coord_proto.latitude();
    result.lng = coord_proto.longitude();
    return result;
}    

transport_catalogue_serialize::Stop TransportCatalogSerialization::GetProtoStop(int id_stop, domain::Stop &stop) {
    transport_catalogue_serialize::Stop result;
    *result.mutable_coord() = CoordToProto(stop.Coord);
    result.set_name(string(stop.Name));
    result.set_id(id_stop);
    return result;
}
    
transport_catalogue_serialize::Bus TransportCatalogSerialization::GetProtoBus(domain::BusInfo busInfo) {
    transport_catalogue_serialize::Bus result;
    result.set_number(string(busInfo.Number));
    result.set_is_loop(busInfo.IsLoop);
    for (auto& stop_name:busInfo.StopNames) {
        int id_stop = stop_id[stop_name];
        result.add_id_stops(id_stop);
    }
    return result;
}    

 domain::RoutesStop TransportCatalogSerialization::GetStopRouteFromProto(const transport_catalogue_serialize::Stop &stop_proto) {
    domain::RoutesStop result;
    result.Name = stop_proto.name();
    result.Coord = ProtoToCoord(stop_proto.coord());
    return result;
}
    
domain::BusRoute TransportCatalogSerialization::GetBusRouteFromProto(const transport_catalogue_serialize::Bus &bus_proto) {
    domain::BusRoute result;
    result.Number = bus_proto.number();
    result.IsLoop = bus_proto.is_loop();
    int count = bus_proto.id_stops_size();
    for (int i = 0; i < count; i++) {
        result.Stops.push_back(string(id_stop[bus_proto.id_stops(i)]));
    }
    return result;
}
    
void TransportCatalogSerialization::CatalogeToProto(const transport_cataloge::TransportCatalogue &catalog) {
    auto stops = catalog.GetListAllStops();
    auto buses = catalog.GetListAllBuses();
    
    int id_stop = 0;
    // заполнение список остановок
    for (auto &stop:stops) {
        stop_id.insert({stop.Name, id_stop});
        *catalog_proto.add_stops() = GetProtoStop(id_stop, stop);
        id_stop++;
    }
    
    // заполнение список маршрутов
    for (auto &bus:buses) {
        *catalog_proto.add_buses() = GetProtoBus(catalog.GetBusInfo(string(bus.Number)));
    }
    
    auto distances = catalog.GetAllDistances();
    for (auto &item1:distances) {
        int id_from = stop_id[item1.first];
        for (auto &item2:item1.second) {
            int id_to = stop_id[item2.first];
            transport_catalogue_serialize::Distance distance_proto;
            distance_proto.set_id_from(id_from);
            distance_proto.set_id_to(id_to);
            distance_proto.set_distance(item2.second);
            
            *catalog_proto.add_distances() = distance_proto;
        }
    }
}    
    
bool TransportCatalogSerialization::SaveToFile(std::string fileName, const transport_cataloge::TransportCatalogue &catalog, const renderer::TransportCatalogeRendererSVG &render, const TransportRouter &router) {
    Reset();
    try {
        CatalogeToProto(catalog);
        RendererToProto(render);
        
        transport_router_serialize::TransportRouter router_proto; 
        router.Serialize(router_proto);
        *catalog_proto.mutable_transport_router() = router_proto;
        
        ofstream ofs(fileName, ios::binary);
        catalog_proto.SerializeToOstream(&ofs);
        return true;
    } catch (...) {
        return false;
    }
    
}

void TransportCatalogSerialization::LoadStopsFromProto(transport_cataloge::TransportCatalogue &catalog) {
    int count = catalog_proto.stops_size();
    for (int i = 0; i < count; i++) {
        int id = catalog_proto.stops(i).id();
        string_view name = catalog_proto.stops(i).name();
        id_stop.insert({id, name});
        auto stop = GetStopRouteFromProto(catalog_proto.stops(i));
        catalog.AddStop(stop);
    }
}
    
void TransportCatalogSerialization::LoadBusesFromProto(transport_cataloge::TransportCatalogue &catalog) {
    int count = catalog_proto.buses_size();
    for (int i = 0; i < count; i++) {
        auto bus = GetBusRouteFromProto(catalog_proto.buses(i));
        catalog.AddBus(bus);
    }
}
    
void TransportCatalogSerialization::LoadDistancesProto(transport_cataloge::TransportCatalogue &catalog) {
    int count = catalog_proto.distances_size();
    for (int i = 0; i < count; i++) {
        //auto bus = GetBusRouteFromProto(catalog_proto.buses(i));
        auto from_stop = id_stop[catalog_proto.distances(i).id_from()];
        auto to_stop = id_stop[catalog_proto.distances(i).id_to()];
        catalog.AddDistance(from_stop, to_stop, catalog_proto.distances(i).distance()
        );
    }
}    
    
void TransportCatalogSerialization::ProtoToCatalog(transport_cataloge::TransportCatalogue &catalog) {
    // считывание списка остановок в буфер
    LoadStopsFromProto(catalog);
    LoadBusesFromProto(catalog);
    LoadDistancesProto(catalog);
} 
 
    
void TransportCatalogSerialization::RendererToProto(const renderer::TransportCatalogeRendererSVG &render) {
    auto settings = render.GetRenderSettings();
    auto rs = catalog_proto.mutable_render_settings();
    
    rs->set_width(settings.width);
    rs->set_height(settings.height);

    rs->set_padding(settings.padding);
    
    rs->set_line_width(settings.line_width);
    rs->set_stop_radius(settings.stop_radius);
    
    rs->set_bus_label_font_size(settings.bus_label_font_size);
    rs->mutable_bus_label_offset()->set_x(settings.bus_label_offset.x);
    rs->mutable_bus_label_offset()->set_y(settings.bus_label_offset.y);
    
    rs->set_stop_label_font_size(settings.stop_label_font_size);
    rs->mutable_stop_label_offset()->set_x(settings.stop_label_offset.x);
    rs->mutable_stop_label_offset()->set_y(settings.stop_label_offset.y);

    rs->set_underlayer_color(settings.underlayer_color);
    rs->set_underlayer_width(settings.underlayer_width);
    
    int count = settings.color_palette.size();
    for (int i = 0; i < count; i++) {
        *rs->add_color_palette() = settings.color_palette[i];
    }
}
    
void TransportCatalogSerialization::ProtoToRenderer(renderer::TransportCatalogeRendererSVG &render) {
    
    renderer::RenderSettings settings;
    auto rs = catalog_proto.render_settings();
    
    settings.width = rs.width();
    settings.height = rs.height();
    
    settings.padding = rs.padding();
    
    settings.line_width = rs.line_width();
    settings.stop_radius = rs.stop_radius();
    
    settings.bus_label_font_size = rs.bus_label_font_size();
    settings.bus_label_offset.x = rs.bus_label_offset().x();
    settings.bus_label_offset.y = rs.bus_label_offset().y();
    
    settings.stop_label_font_size = rs.stop_label_font_size();
    settings.stop_label_offset.x = rs.stop_label_offset().x();
    settings.stop_label_offset.y = rs.stop_label_offset().y();
    
    settings.underlayer_color = rs.underlayer_color();
    settings.underlayer_width = rs.underlayer_width();
    
    int count = rs.color_palette_size();
    for (int i = 0; i < count; i++) {
        settings.color_palette.push_back(rs.color_palette(i));
    }
    
    render.SetRenderSettings(settings);
}
    
bool TransportCatalogSerialization::LoadFromFile(std::string fileName, transport_cataloge::TransportCatalogue &catalog, renderer::TransportCatalogeRendererSVG &render, TransportRouter &router) {
    Reset();
    try {
        ifstream ifs(fileName, ios::binary);
        if (!catalog_proto.ParseFromIstream(&ifs)) {
            return false;
        }
        ProtoToCatalog(catalog);
        ProtoToRenderer(render);
        
        auto router_proto = catalog_proto.transport_router();
        router.Deserialize(router_proto);
        
        return true;
    } catch (...) {
        return false;
    }
    
}     
    
} // end namespace serialization    