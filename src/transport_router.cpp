#include <utility>
#include <string>
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
    graph = std::move(make_unique<graph::DirectedWeightedGraph<EdgeInfo>>(transportCatalogue.GetCountStops()));
    busVelocity = bus_velocity * KmH_To_MMin;
    busWaitTime = bus_wait_time;
    ScanTransportCatalogue();
    router = std::move(make_unique<graph::Router<EdgeInfo>>(*graph));
}

void TransportRouter::BuildIndexes() {
    for (size_t i = 0; i < stops.size(); i++) {
        indexStops[stops[i].Name] = i;
    }
}

void TransportRouter::AddEdge(std::string_view busNumber, int firstStopId,  int secondStopId, double distance, int stopCount) {
    double weight = distance / busVelocity;
    weight += busWaitTime;
    EdgeInfo edgeInfo = {busNumber, stopCount, weight};
    graph::Edge<EdgeInfo> edge = {static_cast<size_t>(firstStopId), static_cast<size_t>(secondStopId), edgeInfo};
    graph->AddEdge(edge);
}

void TransportRouter::ScanTransportCatalogue() {
    buses = std::move(transportCatalogue.GetListAllBuses());
    stops = std::move(transportCatalogue.GetListAllStops());
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
        result_items.push_back(BuildBusStatus(edge_graph.weight.BusNumber, edge_graph.weight.StopsCount,  edge_graph.weight.weight));
    }
    
    return json::Builder{}
                .StartDict()
                    .Key("total_time").Value(tmp_result.value().weight.weight)
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