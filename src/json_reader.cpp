#include <algorithm>
#include <sstream>
#include <iostream>

#include "json_reader.h"
#include "json_builder.h"

using namespace std;

namespace reader {
using namespace std::literals;
    
void ITransportCatalogeReader::SaveDataToCataloge(TransportCatalogeHandler &catalogue_handler) {
    auto input = ReadInputQuery();

    // добавление остановок
    for ( size_t i = 0; i < input.ListStops.size(); i++ ) {
        catalogue_handler.AddStop(input.ListStops[i]);
    }
    
    // добавление маршрутов
    for ( size_t i = 0; i < input.ListBuses.size(); i++ ) {
        catalogue_handler.AddBus(input.ListBuses[i]);
    }

    // добавление расстояний
    for (auto &rec:input.ListStops) {
        for (auto &item:rec.Distances) {
            catalogue_handler.AddDistance(rec.Name, item.first, item.second);
        }
    }    
}    
    
void ITransportCatalogeReader::RunQuery(TransportCatalogeHandler &catalogue_handler) {
    ResetResult();
    ReadOutputQuery(catalogue_handler);
}    
    
domain::BusRoute JsonReader::ParseBus(const json::Dict &dict) {
    domain::BusRoute result;
    
    result.Number = dict.at("name").AsString();
    for (auto &node:dict.at("stops").AsArray()) {
        result.Stops.push_back(node.AsString());
    }
    result.IsLoop = dict.at("is_roundtrip").AsBool();
        
    return result;
}
    
domain::RoutesStop JsonReader::ParseStop(const json::Dict &dict) {
    domain::RoutesStop result;
    
    result.Name = dict.at("name").AsString();
    result.Coord.lat = dict.at("latitude").AsDouble();
    result.Coord.lng = dict.at("longitude").AsDouble();
    for (auto &node:dict.at("road_distances").AsDict()) {
        result.Distances.insert({node.first, node.second.AsInt()});
    }
    
    return result;
}
    
    
domain::InputData JsonReader::ReadInputQuery() {
    if (doc_.GetRoot().AsDict().count("base_requests") == 0) {
        return {};
    }
    domain::InputData result;
    for (auto &item:doc_.GetRoot().AsDict().at("base_requests").AsArray()) {
        auto item_dict = item.AsDict();
        if (item_dict.at("type"s).AsString() == "Stop"s) {
            result.ListStops.push_back(ParseStop(item_dict));
        } else if (item_dict.at("type"s).AsString() == "Bus"s) {
            result.ListBuses.push_back(ParseBus(item_dict));
        }
    }
    
    return result;
}
    
    
void JsonReader::ParseQuery(TransportCatalogeHandler &catalogue_handler, const json::Dict &dict) {
    int requestId = dict.at("id"s).AsInt();
    if (dict.at("type"s).AsString() == "Bus"s) {
        SaveBusInfo(requestId, catalogue_handler.GetBusInfo(dict.at("name").AsString()));
    } else if (dict.at("type"s).AsString() == "Stop"s) {
        SaveStopInfo(requestId, catalogue_handler.GetStopInfo(dict.at("name").AsString()));
    } else if (dict.at("type"s).AsString() == "Map"s) {
        SaveMapRender(requestId, catalogue_handler.RenderMap());
    } else if (dict.at("type"s).AsString() == "Route"s) {
        SaveRouterData(requestId, catalogue_handler.GetRoute(dict.at("from"s).AsString(), dict.at("to"s).AsString()));
    }
}
    
void JsonReader::ReadOutputQuery(TransportCatalogeHandler &catalogue_handler) {
    if (doc_.GetRoot().AsDict().count("stat_requests") == 0) {
        return;
    }
    for (auto &item:doc_.GetRoot().AsDict().at("stat_requests").AsArray()) {
        ParseQuery(catalogue_handler, item.AsDict());
    }
}

json::Node JsonReader::GetJsonBusInfo(int id, const domain::BusInfo &bus) {
    if (bus.CountStop < 0) {
        return GetErrorMessage(id);
    } else {
        return json::Builder{}
                .StartDict()
                    .Key("curvature").Value(bus.CurveDistance/bus.LinearDistance)
                    .Key("request_id").Value(id)
                    .Key("route_length").Value(bus.CurveDistance)
                    .Key("stop_count").Value(bus.CountStop)
                    .Key("unique_stop_count").Value(bus.CountUniqueStop)
                .EndDict().Build();    
    }
}

json::Node JsonReader::GetJsonStopInfo(int id, const domain::StopInfo &stop) {
    if (!stop.IsExist) {
        return GetErrorMessage(id);
    }
    
    json::Array buses;
    for (auto name:stop.BusesNames) {
        buses.push_back(json::Node(string(name)));
    }
    return json::Builder{}
                .StartDict()
                    .Key("buses").Value(buses)
                    .Key("request_id").Value(id)
                .EndDict().Build();
}    
    
    
void JsonReader::SaveBusInfo(int id, const domain::BusInfo &bus) {
    result_.push_back(std::move(GetJsonBusInfo(id, bus)));
}
    
void JsonReader::SaveStopInfo(int id, const domain::StopInfo &stop) {
    result_.push_back(std::move(GetJsonStopInfo(id, stop)));
}
    
void JsonReader::SaveMapRender(int id, string raw_data) {
    json::Dict dict = json::Builder{}
                .StartDict()
                    .Key("map").Value(raw_data)
                    .Key("request_id").Value(id)
                .EndDict().Build().AsDict();
    result_.push_back(std::move(dict));
}
    
void JsonReader::SaveRouterData(int id, json::Dict raw_data) {
    if (raw_data.empty()) {
        result_.push_back(GetErrorMessage(id));
        return;
    }
    json::Dict dict = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("total_time").Value(raw_data.at("total_time").AsDouble())
                    .Key("items").Value(raw_data.at("items").AsArray())
                .EndDict().Build().AsDict();
    result_.push_back(std::move(dict));
}    
    
json::Dict JsonReader::GetErrorMessage(int id) {
    return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found"s)
                .EndDict().Build().AsDict();
}    
    
void JsonReader::ResetResult() {
    result_.clear();
}

json::Document JsonReader::GetResultQuery() {
    return json::Document(result_);
}
    
void JsonReader::SetRouterSettings(TransportCatalogeHandler &catalogue_handler) const {
    if (doc_.GetRoot().AsDict().count("routing_settings"s) == 0) {
        return;
    }
    
    auto dict = doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    
    int bus_wait_time = dict.at("bus_wait_time"s).AsInt();
    double bus_velocity = dict.at("bus_velocity"s).AsDouble();
    
    catalogue_handler.SetRouterSettings(bus_velocity, bus_wait_time);
}    
    
void JsonReader::SetRenderSettings(TransportCatalogeHandler &catalogue_handler) const {
    if (doc_.GetRoot().AsDict().count("render_settings"s) == 0) {
        return;
    }
    renderer::RenderSettings setting;
    auto dict = doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    
    setting.width = dict.at("width"s).AsDouble();
    setting.height = dict.at("height"s).AsDouble();
    
    setting.padding = dict.at("padding"s).AsDouble();
    
    setting.line_width = dict.at("line_width"s).AsDouble();
    setting.stop_radius = dict.at("stop_radius"s).AsDouble();
    
    setting.bus_label_font_size = dict.at("stop_radius"s).AsDouble();
    setting.bus_label_offset = svg::Point(
        dict.at("bus_label_offset"s).AsArray()[0].AsDouble(),
        dict.at("bus_label_offset"s).AsArray()[1].AsDouble());
    
    setting.stop_label_font_size = dict.at("stop_label_font_size"s).AsInt();
    setting.stop_label_offset = svg::Point(
        dict.at("stop_label_offset"s).AsArray()[0].AsDouble(),
        dict.at("stop_label_offset"s).AsArray()[1].AsDouble());
    
    setting.bus_label_font_size = dict.at("bus_label_font_size"s).AsInt();
    setting.bus_label_offset = svg::Point(
        dict.at("bus_label_offset"s).AsArray()[0].AsDouble(),
        dict.at("bus_label_offset"s).AsArray()[1].AsDouble());
    
    setting.underlayer_color = GetColorFromJson(dict.at("underlayer_color"s));
    setting.underlayer_width = dict.at("underlayer_width"s).AsDouble();;
    
    setting.color_palette = GetColorPaletteFromJson(dict.at("color_palette"s));
    catalogue_handler.SetRenderSettings(setting);
}
 
vector<svg::Color> JsonReader::GetColorPaletteFromJson(const json::Node &palette) const {
    vector<string> result;
    for (auto &color:palette.AsArray()) {
        result.push_back(GetColorFromJson(color));
    }
    return result;
}    
svg::Color JsonReader::GetColorFromJson(const json::Node &color) const {
    if (color.IsString()) {
        return color.AsString();
    } else {
        std::ostringstream strm;
        string result;
        auto array = color.AsArray();
        if (array.size() == 3) {
            strm << "rgb("s 
                << array[0].AsInt() << ","s 
                << array[1].AsInt() << ","s 
                << array[2].AsInt() << ")"s;
        } else {
            strm << "rgba("s 
                << array[0].AsInt() << ","s 
                << array[1].AsInt() << ","s 
                << array[2].AsInt() << ","s 
                << array[3].AsDouble() << ")"s;
        }
        result = strm.str();
        return result;
    }
}
    
} // namespace reader       