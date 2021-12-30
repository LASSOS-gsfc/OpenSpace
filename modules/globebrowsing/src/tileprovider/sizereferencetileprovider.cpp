/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2021                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/globebrowsing/src/tileprovider/sizereferencetileprovider.h>

#include <modules/globebrowsing/src/geodeticpatch.h>
#include <openspace/engine/globals.h>
#include <ghoul/font/fontmanager.h>

namespace {
    constexpr const char* KeyRadii = "Radii";
} // namespace

namespace openspace::globebrowsing {

SizeReferenceTileProvider::SizeReferenceTileProvider(const ghoul::Dictionary& dictionary)
    : TextTileProvider(tileTextureInitData(layergroupid::GroupID::ColorLayers, false))
{
    ZoneScoped

    font = global::fontManager->font("Mono", static_cast<float>(fontSize));

    if (dictionary.hasValue<glm::dvec3>(KeyRadii)) {
        ellipsoid = dictionary.value<glm::dvec3>(KeyRadii);
    }
    else if (dictionary.hasValue<double>(KeyRadii)) {
        const double r = dictionary.value<double>(KeyRadii);
        ellipsoid = glm::dvec3(r, r, r);
    }
}

Tile SizeReferenceTileProvider::tile(const TileIndex& tileIndex) {
    ZoneScoped

    const GeodeticPatch patch(tileIndex);
    const bool aboveEquator = patch.isNorthern();
    const double lat = aboveEquator ? patch.minLat() : patch.maxLat();
    const double lon1 = patch.minLon();
    const double lon2 = patch.maxLon();
    int l = static_cast<int>(ellipsoid.longitudalDistance(lat, lon1, lon2));

    const bool useKm = l > 9999;
    if (useKm) {
        l /= 1000;
    }
    l = static_cast<int>(std::round(l));
    if (useKm) {
        l *= 1000;
    }
    double tileLongitudalLength = l;

    const char* unit;
    if (tileLongitudalLength > 9999) {
        tileLongitudalLength *= 0.001;
        unit = "km";
    }
    else {
        unit = "m";
    }

    text = fmt::format(" {:.0f} {:s}", tileLongitudalLength, unit);
    textPosition = {
        0.f,
        aboveEquator ?
            fontSize / 2.f :
            initData.dimensions.y - 3.f * fontSize / 2.f
    };
    textColor = glm::vec4(1.f);

    return TextTileProvider::tile(tileIndex);
}

Tile::Status SizeReferenceTileProvider::tileStatus(const TileIndex& index) {
    return Tile::Status::OK;
}

TileDepthTransform SizeReferenceTileProvider::depthTransform() {
    return { 0.f, 1.f };
}

void SizeReferenceTileProvider::update() {}

int SizeReferenceTileProvider::maxLevel() {
    return 1337; // unlimited
}

float SizeReferenceTileProvider::noDataValueAsFloat() {
    return std::numeric_limits<float>::min();
}

} // namespace openspace::globebrowsing
