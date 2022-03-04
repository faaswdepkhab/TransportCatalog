#pragma once

#include <vector>
#include <map>
#include <string>
#include <string_view>

#include "geo.h"

namespace domain {

struct Bus {
    std::string_view Number;
    bool IsLoop;
};  
    
struct Stop {
    std::string_view Name;
    geo::Coordinates Coord;
};    
    
struct BusRoute  {
    std::string Number;
    bool IsLoop;
    std::vector<std::string> Stops;
};       
    
struct RoutesStop {
    std::string Name;
    geo::Coordinates Coord;
    std::map<std::string, int> Distances;
};
    
 // тип данных для получения данных из запросов на ввод
struct InputData {
    std::vector<BusRoute> ListBuses;
    std::vector<RoutesStop> ListStops;   
}; 

struct BusInfo {
    std::string_view Number;
    int CountStop;
    int CountUniqueStop;
    int CurveDistance;
    double LinearDistance;
    bool IsLoop;
    std::vector<std::string_view> StopNames;
};

struct StopInfo {
    std::string Name;
    bool IsExist;
    geo::Coordinates Coord;
    std::vector<std::string_view> BusesNames;
};

/*    
enum QueryType { BUS, STOP, MAP, ROUTE };
    
struct QueryLine {
    int Id;
    QueryType Type;
    std::string Query;
};
    
using  QueryLinesVector = std::vector<QueryLine>;  
*/

}     // namespace domain 

 
        