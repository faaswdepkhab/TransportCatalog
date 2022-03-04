#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <string_view>

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include "json.h"

const double KmH_To_MMin = 1000.0 / 60;

class TransportRouter {
public:
    struct EdgeInfo {
        std::string_view BusNumber;
        int StopsCount;
        double weight;
        
        bool operator <(const EdgeInfo &other) const {
            return weight < other.weight;
        }
        
        bool operator >(const EdgeInfo &other) const {
            return weight > other.weight;
        }
        
        friend EdgeInfo operator+(const EdgeInfo &ls, const EdgeInfo &rs);
    };
    
    TransportRouter(transport_cataloge::TransportCatalogue &newTransportCatalogue): transportCatalogue(newTransportCatalogue) {}
    void BuildRouter(double bus_velocity ,int bus_wait_time);
    json::Dict GetRoute(std::string from, std::string to);
private:
    // траспортный каталог
    transport_cataloge::TransportCatalogue &transportCatalogue;
    
    // маршрутизатор
    std::unique_ptr<graph::Router<EdgeInfo>> router;
    
    // граф для маршрутизатора
    std::unique_ptr<graph::DirectedWeightedGraph<EdgeInfo>> graph;
    
    // скорость автобуса (м/мин)
    double busVelocity;
    
    // время ожидания на остановке
    int busWaitTime;
    
    // список маршрутов
    std::vector<domain::Bus> buses;
    
    // список остановок
    std::vector<domain::Stop> stops;
    
    // индексы остановок
    std::map<std::string_view, size_t, std::less<>> indexStops;
    
    // индексы маршрутов
    //std::map<std::string_view, size_t, std::less<>> indexBuses;
    
    // список информации о рёбрах
    //std::vector<EdgeInfo> listEdges;

    // добавление участка пути
    void AddEdge(std::string_view busNumber, int firstStopId, int secondStopId, double distance, int stopCount);
    
    // сканирование транспортного каталога и построение графа
    void ScanTransportCatalogue();

    // построение индекса остановок
    void BuildIndexes();
    
    // построение рёбер для заданного маршрута
    void BuildRoutesForBus(std::string_view);
    // построение расстояний всех возможных пар остановок (по маршруту)
    void CalcDistances(std::string_view busNumber, const std::vector<int> &stopsId, bool isLoop);
    
    // построение статуса "wait" для результата
    json::Dict BuildWaitStatus(std::string_view stopName);
    
    // построение статуса "wait" для результата
    json::Dict BuildBusStatus(std::string_view busNumber, int stopCount, double time);
    
};

inline TransportRouter::EdgeInfo operator+(const TransportRouter::EdgeInfo &ls, const TransportRouter::EdgeInfo &rs) {
    double weight = ls.weight + rs.weight;
    return {ls.BusNumber, ls.StopsCount, weight};
}
