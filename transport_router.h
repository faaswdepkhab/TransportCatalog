#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <string_view>
#include <transport_router.pb.h>

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include "json.h"

const double KmH_To_MMin = 1000.0 / 60;

class TransportRouter {
public:
    
    struct EdgeInfo {
        size_t IdBus;
        int StopsCount;
    };
    
    TransportRouter(transport_cataloge::TransportCatalogue &newTransportCatalogue): transportCatalogue(newTransportCatalogue) {}
    
    void BuildRouter(double bus_velocity ,int bus_wait_time);
    
    json::Dict GetRoute(std::string from, std::string to);
    
    void Serialize(transport_router_serialize::TransportRouter &serialData) const;
    
    void Deserialize(const transport_router_serialize::TransportRouter &serialData);
    
private:
    // траспортный каталог
    transport_cataloge::TransportCatalogue &transportCatalogue;
    
    // маршрутизатор
    std::unique_ptr<graph::Router<double>> router;
    
    // граф для маршрутизатора
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph;
    
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
    std::map<std::string_view, size_t, std::less<>> indexBuses;
    
    // список информации о рёбрах
    std::vector<EdgeInfo> listEdges;

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
    
    // сериализация графа
    void SerializeGraph(transport_router_serialize::TransportRouter &serialData) const;
    
    // десериализация графа
    void DeserializeGraph(const transport_router_serialize::TransportRouter &serialData);
    
    // сериализация списка рёбер
    void SerializeListEdges(transport_router_serialize::TransportRouter &serialData) const;
    
    // десериализация списка рёбер
    void DeserializeListEdges(const transport_router_serialize::TransportRouter &serialData);

// сериализация настроек маршрутизатора
    void SerializeRoutersSettings(transport_router_serialize::TransportRouter &serialData) const;
    
    // десериализация настроек маршрутизатора
    void DeserializeRoutersSettings(const transport_router_serialize::TransportRouter &serialData);
    
    // начальные действия при десериализации
    void InitDeserialize();
    
};
