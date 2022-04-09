#pragma once
#include <iostream>

#include "request_handler.h"
#include "json.h"
#include "domain.h"

namespace reader {
    
//интерфейсный класс - читатель данных для транспортного справочника
class ITransportCatalogeReader {
public:
    void SaveDataToCataloge(TransportCatalogeHandler &catalogue_handler);
    void RunQuery(TransportCatalogeHandler &catalogue_handler);
protected:    
    virtual domain::InputData ReadInputQuery() = 0;
    virtual void ReadOutputQuery(TransportCatalogeHandler &catalogue_handler) = 0;
    virtual void ResetResult() = 0;
    
    virtual void SaveBusInfo(int id, const domain::BusInfo &bus) = 0;
    virtual void SaveStopInfo(int id, const domain::StopInfo &stop) = 0;
    virtual void SaveMapRender(int id, std::string raw_data) = 0;
    virtual void SaveRouterData(int id, json::Dict raw_data) = 0;
};

class JsonReader: public ITransportCatalogeReader {
public:
    JsonReader(const json::Document &doc): doc_(doc) {}
    json::Document GetResultQuery();
    void SetRenderSettings(TransportCatalogeHandler &catalogue_handler) const;
    void SetRouterSettings(TransportCatalogeHandler &catalogue_handler) const;
    void SaveToFile(TransportCatalogeHandler &catalogue_handler) const;
    void LoadFromFile(TransportCatalogeHandler &catalogue_handler) const;

protected: 
    domain::InputData ReadInputQuery() override;
    void ReadOutputQuery(TransportCatalogeHandler &catalogue_handler) override;
    void SaveBusInfo(int id, const domain::BusInfo &bus) override;
    void SaveStopInfo(int id, const domain::StopInfo &stop) override;
    void SaveMapRender(int id, std::string raw_data) override;
    void SaveRouterData(int id, json::Dict raw_data) override;
    void ResetResult() override;
private:
    json::Document doc_;
    
    json::Array result_;
    //
    domain::BusRoute ParseBus(const json::Dict &dict);
    domain::RoutesStop ParseStop(const json::Dict &dict);
    void ParseQuery(TransportCatalogeHandler &catalogue_handler, const json::Dict &dict);
    json::Node GetJsonBusInfo(int id, const domain::BusInfo &bus);
    json::Node GetJsonStopInfo(int id, const domain::StopInfo &stop);
    
    svg::Color GetColorFromJson(const json::Node &color) const;
    std::vector<svg::Color> GetColorPaletteFromJson(const json::Node &palette) const;
    json::Dict GetErrorMessage(int id);

};
    
} // namespace reader   