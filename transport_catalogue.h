#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "geo.h"
#include "domain.h"

namespace transport_cataloge {

class TransportCatalogue {
public:
    using DistancesInfo = std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>>;
    
    TransportCatalogue() {}
    
    void AddStop(domain::RoutesStop &s);
    void AddBus(domain::BusRoute &b);
    
    domain::BusInfo GetBusInfo(const std::string &Number) const;
    
    domain::StopInfo GetStopInfo(const std::string &Name) const;
    
    const domain::Bus* FindBus(std::string_view Number) const;
    
    const domain::Stop* FindStop(std::string_view Name) const;
    
    const DistancesInfo& GetAllDistances() const;
    
    void AddDistance(std::string_view src, std::string_view dest, int distance);
    
    // получение рассотяния по паре src и dest
    int GetDistance(std::string_view src, std::string_view dest) const;
    
    // список остановок
    std::vector<domain::Stop> GetListAllStops() const;
    // список маршрутов
    std::vector<domain::Bus> GetListAllBuses() const;
    
    int GetCountBuses() const;
    
    int GetCountStops() const;
    
private:
    // список остановок
    std::deque<domain::Stop> fStops;
    
    // список автобусов
    std::deque<domain::Bus> fBuses;
    
    // массив индексов остановок в массиве fStops
    std::unordered_map <std::string, int> IndexesStops;
    // массив индексов остановок в массиве fBuses
    std::unordered_map <std::string, int> IndexesBuses;
    
    // массив соответствия автобусов остановкам
    // set названий для сортировки и обрасывания дубликатов
    std::unordered_map<std::string_view, std::set<std::string_view>> Buses_On_Stop;
    
    // маршруты, хранятся индексы остановок
    std::unordered_map<std::string_view, std::vector<int>> Routes;
    
    // граф рассояний
    DistancesInfo Distances;    
    
    // получение расстояния от src до dest (фиксированное направление)
    bool GetRawDistance(std::string_view src, std::string_view dest, int &result) const;
    
    // подсчёт числа уникальных остановок для маршрута
    int GetCountUniqueStops(std::string_view Number) const;
    
    // расчёт реального расстояния для маршрута Number
    int CalcCurveDistance(std::string_view Number) const;
    
    // расчёт расстояния для маршрута Number как множества прямых отрезков
    double GetLinearDistance(std::string_view Number) const;
    
    // формирование структуры ParseStop на основе RoutesStop
    domain::Stop ParseStop(const domain::RoutesStop &stop);
    
    // формирование структуры ParseBus на основе BusRoute
    domain::Bus ParseBus(const domain::BusRoute &b);
    
};
    
} // конец namespace transport_cataloge