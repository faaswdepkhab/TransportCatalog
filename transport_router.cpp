#include <utility>
#include <string>
#include <graph.pb.h>

#include "json_builder.h"
#include "transport_router.h"


using namespace std;

namespace {
    // "реверс" индекса
    int reverseIndex(int i, int n, bool reverse) {
        return reverse? n - 1 - i: i;
    }
}

void TransportRouter::BuildRouter(double bus_velocity ,int bus_wait_time) {
    graph = std::move(make_unique<graph::DirectedWeightedGraph<double>>(transportCatalogue.GetCountStops()));
    busVelocity = bus_velocity * KmH_To_MMin;
    busWaitTime = bus_wait_time;
    ScanTransportCatalogue();
    router = std::move(make_unique<graph::Router<double>>(*graph));
}

void TransportRouter::BuildIndexes() {
    indexStops.clear();
    for (size_t i = 0; i < stops.size(); i++) {
        indexStops[stops[i].Name] = i;
    }
    
    indexBuses.clear();
    for (size_t i = 0; i < buses.size(); i++) {
        indexBuses[buses[i].Number] = i;
    }
}

void TransportRouter::AddEdge(std::string_view busNumber, int firstStopId,  int secondStopId, double distance, int stopCount) {
    double weight = distance / busVelocity;
    weight += busWaitTime;
    
    graph::Edge<double> edge = {static_cast<size_t>(firstStopId), static_cast<size_t>(secondStopId), weight};
    graph->AddEdge(edge);
    
    listEdges.push_back({indexBuses[busNumber], stopCount});
}

void TransportRouter::ScanTransportCatalogue() {
    buses = std::move(transportCatalogue.GetListAllBuses());
    stops = std::move(transportCatalogue.GetListAllStops());
    listEdges.clear();
    BuildIndexes();
    
    for (auto &bus:buses) {
        BuildRoutesForBus(bus.Number);
    }
}

void TransportRouter::CalcDistances(string_view busNumber, const std::vector<int> &stopsId, bool isLoop) {
    int countLoop = isLoop? 1: 2;
    bool reverse = false;
    int s = stopsId.size();

    for (int k = 0 ; k < countLoop; k++) {
        for ( int i_raw = 0; i_raw < s - 1; i_raw++ ) {
            double sum = 0;
            int firstId = stopsId[reverseIndex(i_raw, s, reverse)];
            int stopCount = 0;
            for ( int j_raw = i_raw + 1; j_raw < s; j_raw++ ) {
                stopCount++;
                double distance = transportCatalogue.GetDistance(stops[stopsId[reverseIndex(j_raw - 1, s, reverse)]].Name, stops[stopsId[reverseIndex(j_raw, s, reverse)]].Name);
                sum += distance;
                int secondId = stopsId[reverseIndex(j_raw, s, reverse)];
                AddEdge(busNumber, firstId, secondId, sum, stopCount);
            }
        }
        reverse = !reverse;
    }
}

void TransportRouter::BuildRoutesForBus(string_view busNumber) {
    auto busInfo = transportCatalogue.GetBusInfo(string(busNumber));
    vector<int> stopsId;
    for (size_t i = 0; i < busInfo.StopNames.size(); i++) {
        size_t stopId = indexStops[busInfo.StopNames[i]];
        stopsId.push_back(stopId);
    }
    CalcDistances(busNumber, stopsId, busInfo.IsLoop);
    
}

json::Dict TransportRouter::GetRoute(std::string from, std::string to) {
    size_t firstId = static_cast<size_t>(indexStops[from]);
    size_t secondId = static_cast<size_t>(indexStops[to]);
    auto tmp_result = router->BuildRoute(firstId, secondId);
    if (!tmp_result.has_value()) {
        return json::Builder{}
                .StartDict()
                .EndDict().Build().AsDict();
    }
    auto result_items = json::Builder{}
                .StartArray()
                .EndArray().Build().AsArray();
    
    for (auto egdeId:tmp_result.value().edges) {
        auto edge_graph = graph->GetEdge(egdeId);
        result_items.push_back(BuildWaitStatus(stops[static_cast<int>(edge_graph.from)].Name));
        result_items.push_back(BuildBusStatus(buses[listEdges[egdeId].IdBus].Number, listEdges[egdeId].StopsCount,  edge_graph.weight));
    }
    
    return json::Builder{}
                .StartDict()
                    .Key("total_time").Value(tmp_result.value().weight)
                    .Key("items").Value(result_items)
                .EndDict().Build().AsDict();
}

json::Dict TransportRouter::BuildWaitStatus(std::string_view stopName) {
    return json::Builder{}
                .StartDict()
                    .Key("type").Value("Wait")
                    .Key("stop_name").Value(string(stopName))
                    .Key("time").Value(busWaitTime)
                .EndDict().Build().AsDict();
}

json::Dict TransportRouter::BuildBusStatus(std::string_view busNumber, int stopCount, double time) {
    return json::Builder{}
                .StartDict()
                    .Key("type").Value("Bus")
                    .Key("bus").Value(string(busNumber))
                    .Key("span_count").Value(stopCount)
                    .Key("time").Value(time - busWaitTime)
                .EndDict().Build().AsDict();
}

void TransportRouter::SerializeGraph(transport_router_serialize::TransportRouter &serialData) const {
    size_t count = graph->GetEdgeCount();
    for (size_t i = 0; i < count; i++) {
        auto edge = graph->GetEdge(i);
        graph_serialize::Edge edge_proto;
        edge_proto.set_from(edge.from);
        edge_proto.set_to(edge.to);
        edge_proto.set_weight(edge.weight);
        *serialData.mutable_graph()->add_list_edges() = edge_proto;
    }
}
    
void TransportRouter::DeserializeGraph(const transport_router_serialize::TransportRouter &serialData) {
    graph->Reset();
    graph->SetVertexCount(transportCatalogue.GetCountStops());
    int count = serialData.graph().list_edges_size();
    for (int i = 0; i < count; i++) {
        graph::Edge<double> edge;
        edge.from = serialData.graph().list_edges(i).from();
        edge.to = serialData.graph().list_edges(i).to();
        edge.weight = serialData.graph().list_edges(i).weight();
        graph->AddEdge(edge);
    }
}

void TransportRouter::SerializeListEdges(transport_router_serialize::TransportRouter &serialData) const {
    for (auto &edge:listEdges) {
        transport_router_serialize::EdgeInfo edge_proto;
        edge_proto.set_id_bus(edge.IdBus);
        edge_proto.set_stops_count(edge.StopsCount);
        *serialData.add_list_edges() = edge_proto;
    }
}
    
void TransportRouter::DeserializeListEdges(const transport_router_serialize::TransportRouter &serialData) {
    int count = serialData.list_edges_size();
    listEdges.clear();
    for (int i = 0; i < count; i++) {
        EdgeInfo edge;
        edge.IdBus = serialData.list_edges(i).id_bus();
        edge.StopsCount = serialData.list_edges(i).stops_count();
        listEdges.push_back(edge);
    }
}

void TransportRouter::SerializeRoutersSettings(transport_router_serialize::TransportRouter &serialData) const {
    serialData.set_bus_velocity(busVelocity);
    serialData.set_bus_wait_time(busWaitTime);
}
    
void TransportRouter::DeserializeRoutersSettings(const transport_router_serialize::TransportRouter &serialData) {
    busVelocity = serialData.bus_velocity();
    busWaitTime = serialData.bus_wait_time();
}

void TransportRouter::InitDeserialize() {
    graph = std::move(make_unique<graph::DirectedWeightedGraph<double>>());
    router = std::move(make_unique<graph::Router<double>>(*graph));
    
    buses = std::move(transportCatalogue.GetListAllBuses());
    stops = std::move(transportCatalogue.GetListAllStops());
    listEdges.clear();
    BuildIndexes();
}

void TransportRouter::Serialize(transport_router_serialize::TransportRouter &serialData) const {
    SerializeRoutersSettings(serialData);
    SerializeListEdges(serialData);
    SerializeGraph(serialData);
    router->Serialize(serialData);
}
    
void TransportRouter::Deserialize(const transport_router_serialize::TransportRouter &serialData) {
    InitDeserialize();
    DeserializeRoutersSettings(serialData);
    DeserializeListEdges(serialData);
    DeserializeGraph(serialData);
    router->Deserialize(serialData);
}
