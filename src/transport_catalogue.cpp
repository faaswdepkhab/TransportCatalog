#include <iostream>
#include <algorithm>
#include "transport_catalogue.h"

using namespace std;

namespace transport_cataloge {

domain::Stop TransportCatalogue::ParseStop(const domain::RoutesStop &stop) {
    domain::Stop result;
    result.Name = IndexesStops.find(stop.Name)->first;
    result.Coord = stop.Coord;
    return result;
}


domain::Bus TransportCatalogue::ParseBus(const domain::BusRoute &b) {
    //
    domain::Bus result;
    result.Number = IndexesBuses.find(b.Number)->first;
    result.IsLoop = b.IsLoop;
    return result;
}


void TransportCatalogue::AddStop(domain::RoutesStop &s) {
    if (IndexesStops.find(s.Name) == IndexesStops.end() ) {
        int index = fStops.size();
        IndexesStops[s.Name] = index;
        fStops.emplace_back(ParseStop(s));
    }
}

void TransportCatalogue::AddBus(domain::BusRoute &b) {
    if (IndexesBuses.find(b.Number) == IndexesBuses.end() ) {
        int index = fBuses.size();
        IndexesBuses[b.Number] = index;
        fBuses.emplace_back(ParseBus(b));
        
        // сохранение остановок, по которым проходит автобус
        for (auto stop:b.Stops) {
            Buses_On_Stop[IndexesStops.find(stop)->first].insert(IndexesBuses.find(b.Number)->first);
            Routes[IndexesBuses.find(b.Number)->first].push_back(IndexesStops.find(stop)->second);
        }
    }
}

domain::BusInfo TransportCatalogue::GetBusInfo(const string &Number) const{
    const domain::Bus *bus = FindBus(Number);
    auto it = IndexesBuses.find(Number);
    if (it != IndexesBuses.end()) {
        
        domain::BusInfo result;
        result.IsLoop = bus->IsLoop;
        result.Number = Number;
        result.CountUniqueStop  = GetCountUniqueStops(result.Number);
        result.LinearDistance = GetLinearDistance(result.Number);
        result.CurveDistance = CalcCurveDistance(result.Number);
        
        // заполнение списка остановок
        for (auto index_stop:Routes.at(Number)) {
            result.StopNames.push_back(fStops[index_stop].Name);
        }
        
        if (bus->IsLoop) {
            result.CountStop  = Routes.at(result.Number).size();
        } else {
            result.CountStop  = 2 * Routes.at(result.Number).size() - 1;

        }
        return result;    
    } else {
        return {Number, -1 ,-1, -1, -1.0, false, {}};
    }
}

domain::StopInfo TransportCatalogue::GetStopInfo(const string &Name) const{
    auto it = IndexesStops.find(Name);
    domain::StopInfo result;
    result.Name = Name;
    if (it != IndexesStops.end()) {
        result.IsExist = true;
        result.Coord = fStops[it->second].Coord;
        if (Buses_On_Stop.count(Name) > 0) {
            for (auto s:Buses_On_Stop.at(Name)) {
                result.BusesNames.push_back(s);
            }
        } else {
            result.BusesNames = {};
        }
        
    } else {
        result.IsExist = false;
    }
    return result;
}

const domain::Bus* TransportCatalogue::FindBus(std::string_view Number) const {
    //
    auto it = IndexesBuses.find(string(Number));
    if (it != IndexesBuses.end()) {
        int index = it->second;
        auto pos = fBuses.begin()+ index;
        return &*pos;
    } else {
        return nullptr;
    }
}
    
const domain::Stop* TransportCatalogue::FindStop(std::string_view Name) const {
    //
    auto it = IndexesStops.find(string(Name));
    if (it != IndexesStops.end()) {
        int index = it->second;
        auto pos = fStops.begin()+ index;
        return &*pos;
    } else {
        return nullptr;
    }
}

void TransportCatalogue::AddDistance(std::string_view src, std::string_view dest, int distance) {
    Distances[IndexesStops.find(string(src))->first]
        [IndexesStops.find(string(dest))->first] = distance;
}

bool TransportCatalogue::GetRawDistance(std::string_view src, std::string_view dest, int &result) const {
    auto it_src = Distances.find(src);
    if (it_src != Distances.end()) {
        auto it_dest = it_src->second.find(dest);
        if (it_dest == it_src->second.end()) {
            return false;
        } else {
            result = it_dest->second;
            return true;
        }
    } else {
        return false;
    }
}
    

int TransportCatalogue::GetDistance(std::string_view src, std::string_view dest) const {
    int result;
    if (GetRawDistance(src, dest, result)) {
        return result;
    } else {
        if (GetRawDistance(dest, src, result)) {
            return result;
        } else {
            throw std::out_of_range("Расстояние между остановками на задано");
        }
    }
}

int TransportCatalogue::CalcCurveDistance(string_view Number) const {
    const domain::Bus *bus = FindBus(Number);

    int sum = 0;
    auto it = Routes.find(Number);
    for (size_t i = 1; i < it->second.size(); i++) {
        sum += GetDistance(fStops[it->second[i-1]].Name, fStops[it->second[i]].Name);
    }
    if (!bus->IsLoop) {
        for (int i = it->second.size() - 1; i > 0; i--) {
            sum += GetDistance(fStops[it->second[i]].Name, fStops[it->second[i-1]].Name);
        }
    }

    return sum;
}

int TransportCatalogue::GetCountUniqueStops(std::string_view Number) const {
    unordered_set<int> set_stops(Routes.at(Number).begin(), Routes.at(Number).end());
    return set_stops.size();
}

double TransportCatalogue::GetLinearDistance(std::string_view Number) const {
    double sum = 0;
    for (size_t i = 1; i < Routes.at(Number).size(); i++) {
        sum += ComputeDistance(fStops[Routes.at(Number)[i - 1]].Coord, fStops[Routes.at(Number)[i]].Coord);
    }
    
    if (!FindBus(Number)->IsLoop){
        sum *= 2;
    }
    return sum;
}
    
std::vector<domain::Stop> TransportCatalogue::GetListAllStops() const {
    std::vector<domain::Stop> result;
    std::copy(fStops.begin(), fStops.end(), std::back_inserter(result));
    std::sort(result.begin(), result.end(), [](auto &left, auto &right){
        return left.Name < right.Name;
    });
    return result;
}
    
std::vector<domain::Bus> TransportCatalogue::GetListAllBuses() const {
    std::vector<domain::Bus> result;
    std::copy(fBuses.begin(), fBuses.end(), std::back_inserter(result));
    std::sort(result.begin(), result.end(), [](auto &left, auto &right){
        return left.Number < right.Number;
    });
    return result;
}
    
int TransportCatalogue::GetCountBuses() const {
    return fBuses.size();
}
    
int TransportCatalogue::GetCountStops() const {
    return fStops.size();
}
    
} // конец namespace transport_cataloge    