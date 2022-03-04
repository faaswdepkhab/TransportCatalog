#pragma once

#include <memory>
#include <algorithm>


#include "svg.h"
#include "transport_catalogue.h"

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                    double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lng < rhs.lng;
              });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lat < rhs.lat;
              });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
}; 

namespace renderer {    
   
struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    int bus_label_font_size;
    svg::Point bus_label_offset;

    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};    
    
    
//интерфейсный класс - просмоторщик транспортного справочника
class ITransportCatalogeRenderer {
protected:
    virtual void DrawRoutes() = 0;
    virtual void DrawRoutesNames() = 0;
    virtual void DrawStops() = 0;
    virtual void DrawStopsNames() = 0;
    virtual void Init() = 0;
public:
    void RenderMap();
    virtual ~ITransportCatalogeRenderer() {};

};
    
class TransportCatalogeRendererSVG: public ITransportCatalogeRenderer {
public:
    TransportCatalogeRendererSVG(const transport_cataloge::TransportCatalogue &cataloge):db_(cataloge) {}

    void SetRenderSettings(RenderSettings settings);
    std::string GetSVGResultAsString();
    
protected:
    
    void Init() override;
    void DrawRoutes() override;
    void DrawStops() override;
    void DrawRoutesNames() override;
    void DrawStopsNames() override;
    
private:
    RenderSettings settings_;
    svg::Document doc_;
    const transport_cataloge::TransportCatalogue &db_;
    
    std::vector<domain::Bus> listBuses_;
    std::vector<domain::Stop> listStops_;
    std::unique_ptr<SphereProjector> projector_;
    
    void DrawRoute(bool isLoop, svg::Color color, std::vector<std::string_view> &stopNames);
    void DrawRoutesName(std::string_view name, svg::Color color, std::vector<std::string_view> &stopNames);
    void DrawComplexRoutesName(std::string_view name, svg::Point &point,  svg::Color color);
    void DrawComplexStopsName(std::string_view name, svg::Point point);
};    
    
} // namespace renderer 