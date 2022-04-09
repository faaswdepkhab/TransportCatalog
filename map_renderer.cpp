#include <vector>
#include <sstream>

#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
} 

namespace renderer { 
    
using namespace std;
using namespace svg;    
    
void ITransportCatalogeRenderer::RenderMap() {
    Init();
    DrawRoutes();
    DrawRoutesNames();
    DrawStops();
    DrawStopsNames();
}    

void TransportCatalogeRendererSVG::Init() {
    listBuses_ = db_.GetListAllBuses();
    listStops_ = db_.GetListAllStops();
    vector<geo::Coordinates> ListCoords;
    for (auto &item:listStops_) {
        auto stop_info = db_.GetStopInfo(string(item.Name));
        if (stop_info.BusesNames.size() > 0) {
            ListCoords.push_back(item.Coord);
        }
    }
    projector_ = make_unique<SphereProjector>(ListCoords.begin(), ListCoords.end(), settings_.width, settings_.height, settings_.padding);
}    

void TransportCatalogeRendererSVG::DrawRoute(bool isLoop, Color color, vector<string_view> &stopNames) {
    svg::Polyline conture;
    conture
        .SetStrokeColor(color)
        .SetFillColor(NoneColor)
        .SetStrokeWidth(settings_.line_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    
    if (isLoop) {
        for (auto &name:stopNames) {
            auto stop = db_.GetStopInfo(string(name));
            conture.AddPoint((*projector_)(stop.Coord));
        }
    } else {
        int s = stopNames.size(); 
        int j;
        for (int i = 0; i < (2 * s - 1); i++) {
            j = (i < s) ? i : 2 * s - i - 2;
            auto stop = db_.GetStopInfo(string(stopNames[j]));
            conture.AddPoint((*projector_)(stop.Coord));
        }
    }
    doc_.Add(conture);
}    
    
// отображение пути    
void TransportCatalogeRendererSVG::DrawRoutes() {
    size_t index_color = 0;
    Color color;
    for (auto &bus:listBuses_) {
        auto bus_info = db_.GetBusInfo(string(bus.Number));
        if (!bus_info.StopNames.empty()) {
            color = settings_.color_palette[index_color];
            DrawRoute(bus.IsLoop, color, bus_info.StopNames);
            index_color++;
            if (index_color == settings_.color_palette.size()) {
                index_color = 0;
            }   
        }
    }
}

void TransportCatalogeRendererSVG::DrawStops() {
    for (auto &stop:listStops_) {
        auto stop_info = db_.GetStopInfo(string(stop.Name));
        if (stop_info.BusesNames.empty()) {
            continue;
        }
        auto point = (*projector_)(stop_info.Coord);
        doc_.Add(svg::Circle()
            .SetCenter(point)
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"));
    }
}

void TransportCatalogeRendererSVG::DrawComplexRoutesName(string_view name, svg::Point &point, Color color) {
    svg::Text front_text;
    svg::Text back_text;
    front_text
        .SetPosition(point)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(string(name));
    back_text = front_text;
    back_text
        .SetStrokeColor(settings_.underlayer_color)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    front_text.SetFillColor(color);
    doc_.Add(back_text);
    doc_.Add(front_text);
}

void TransportCatalogeRendererSVG::DrawRoutesName(string_view name, Color color, std::vector<std::string_view> &stopNames) {
    auto point = (*projector_)(db_.GetStopInfo(string(stopNames[0])).Coord);
    DrawComplexRoutesName(name, point, color);
    
    int s = stopNames.size();
    if  (!(db_.FindBus(name)->IsLoop) &&
        (stopNames[0] != stopNames[s-1])) {
        auto point = (*projector_)(db_.GetStopInfo(string(stopNames[s - 1])).Coord);
        DrawComplexRoutesName(name, point, color);
    }
    
}

void TransportCatalogeRendererSVG::DrawRoutesNames() {
    int index_color = 0;
    Color color;
    for (auto &num:listBuses_) {
        auto bus_info = db_.GetBusInfo(string(num.Number));
        if (bus_info.StopNames.empty()) {
            continue;
        }
        color = settings_.color_palette[index_color % settings_.color_palette.size()];
        DrawRoutesName(num.Number, color, bus_info.StopNames);
        index_color++;
    }
}

void TransportCatalogeRendererSVG::DrawComplexStopsName(string_view name, Point point) {
    svg::Text front_text;
    svg::Text back_text;
    front_text
        .SetPosition(point)
        .SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(string(name));
    back_text = front_text;
    back_text
        .SetStrokeColor(settings_.underlayer_color)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    front_text.SetFillColor("black");
    doc_.Add(back_text);
    doc_.Add(front_text);
}
    
void TransportCatalogeRendererSVG::DrawStopsNames() {
    for (auto &stop:listStops_) {
        auto stop_info = db_.GetStopInfo(string(stop.Name));
        if (stop_info.BusesNames.empty()) {
            continue;
        }
        auto point = (*projector_)(stop_info.Coord);
        DrawComplexStopsName(stop.Name, point);
    }    
}    
    
void TransportCatalogeRendererSVG::SetRenderSettings(RenderSettings settings) {
    settings_ = settings;
}    

string TransportCatalogeRendererSVG::GetSVGResultAsString() {
    ostringstream strm;
    doc_.Render(strm);
    string result = strm.str();
    return result;
}

const RenderSettings& TransportCatalogeRendererSVG::GetRenderSettings() const {
    return settings_;
}    
}  // namespace renderer  