/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MBVTBUILDER_MBVTTILEBUILDER_H_
#define _CARTO_MBVTBUILDER_MBVTTILEBUILDER_H_

#include <vector>

#include <boost/variant.hpp>
#include <boost/math/constants/constants.hpp>

#include <cglib/vec.h>
#include <cglib/bbox.h>

#include <picojson/picojson.h>

#include <protobuf/encodedpbf.hpp>

namespace carto { namespace mbvtbuilder {
    class MBVTLayerEncoder;
    
    class MBVTTileBuilder final {
    public:
        using Point = cglib::vec2<double>;
        using MultiPoint = std::vector<Point>;
        using MultiLineString = std::vector<std::vector<Point>>;
        using MultiPolygon = std::vector<std::vector<std::vector<Point>>>;
        
        explicit MBVTTileBuilder(int minZoom, int maxZoom);

        void createLayer(const std::string& layerId, float buffer = -1);

        void addMultiPoint(MultiPoint coords, picojson::value properties);
        void addMultiLineString(MultiLineString coordsList, picojson::value properties);
        void addMultiPolygon(MultiPolygon ringsList, picojson::value properties);

        void importGeoJSON(const picojson::value& geoJSON);
        void importGeoJSONFeatureCollection(const picojson::value& featureCollectionDef);
        void importGeoJSONFeature(const picojson::value& featureDef);

        void buildTiles(std::function<void(int, int, int, const protobuf::encoded_message&)> handler) const;

    private:
        using Bounds = cglib::bbox2<double>;
        using Geometry = boost::variant<MultiPoint, MultiLineString, MultiPolygon>;

        static constexpr float DEFAULT_LAYER_BUFFER = 4.0 / 256.0f;

        struct Feature {
            Bounds bounds = Bounds::smallest(); // EPSG3856
            Geometry geometry; // EPSG3856
            picojson::value properties;
        };

        struct Layer {
            std::string layerId;
            Bounds bounds = Bounds::smallest(); // EPSG3856
            std::vector<Feature> features;
            float buffer = 0;
        };

        static constexpr double PI = boost::math::constants::pi<double>();
        static constexpr double EARTH_RADIUS = 6378137.0;
        static constexpr double TILE_TOLERANCE = 1.0 / 256.0;

        static void simplifyLayer(Layer& layer, double tolerance);
        static bool encodeLayer(const Layer& layer, const Point& tileOrigin, double tileSize, const Bounds& tileBounds, MBVTLayerEncoder& layerEncoder);

        static std::vector<std::vector<cglib::vec2<double>>> parseCoordinatesRings(const picojson::value& coordsDef);
        static std::vector<cglib::vec2<double>> parseCoordinatesList(const picojson::value& coordsDef);
        static cglib::vec2<double> parseCoordinates(const picojson::value& coordsDef);
        static Point wgs84ToWM(const cglib::vec2<double>& posWgs84);

        std::vector<Layer> _layers;

        const int _minZoom;
        const int _maxZoom;
    };
} }

#endif