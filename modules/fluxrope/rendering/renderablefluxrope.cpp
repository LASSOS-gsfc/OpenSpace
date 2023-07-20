#include <modules/fluxrope/rendering/renderablefluxrope.h>
#include <modules/fluxrope/fluxropemodule.h>
#include <modules/fieldlinessequence/rendering/renderablefieldlinessequence.h>

#include <openspace/rendering/renderengine.h>

#include <openspace/engine/globals.h>
#include <openspace/engine/windowdelegate.h>
#include <openspace/navigation/navigationhandler.h>
#include <openspace/navigation/orbitalnavigator.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/scene/scene.h>
#include <openspace/util/timemanager.h>
#include <openspace/util/updatestructures.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/openglstatecache.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/textureunit.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <thread>


namespace {
    constexpr std::string_view _loggerCat = "RenderableFluxRope";
    // don't know if this is the right way to do this
    constexpr openspace::properties::Property::PropertyInfo PointsBtnInfo = {
       "LoadNewPoints",
       "Load a new set of points",
       "Loads a new set of points into the object",
       openspace::properties::Property::Visibility::NoviceUser
    };

    constexpr openspace::properties::Property::PropertyInfo PathToPointsFileInfo = {
        "PathToPoints",
        "Path to new flux rope file",
        "This is the path to a new set a points to be rendered",
        openspace::properties::Property::Visibility::NoviceUser
    };

    struct [[codegen::Dictionary(RenderableFluxRope)]] Parameters {

        std::string fluxRopePath;

        

    };

#include "renderablefluxrope_codegen.cpp"
}

namespace openspace {

    documentation::Documentation RenderableFluxRope::Documentation() {
        return codegen::doc<Parameters>("fluxrope_renderablefluxrope");
    }

    RenderableFluxRope::RenderableFluxRope(const ghoul::Dictionary& dictionary) 
        : RenderableFieldlinesSequence(dictionary), 
        _loadNewPointsBtn(PointsBtnInfo),
        _fluxRopeFilePath(PathToPointsFileInfo ) 
    
    {
        const Parameters p = codegen::bake<Parameters>(dictionary);

        _fluxRopeFilePath = absPath(p.fluxRopePath).string();
    
    }

    void RenderableFluxRope::initializeGL() {
        RenderableFieldlinesSequence::initializeGL();

        setupProperties();
    }

    void RenderableFluxRope::definePropertyCallbackFunctions() {
        _loadNewPointsBtn.onChange([this]() {
            LWARNING(fmt::format("Failed to load state from: {}", _fluxRopeFilePath.value()));
            if (!_fluxRopeFilePath.value().empty()) loadNewJsonStateIntoRAM();
            });
    }

    void RenderableFluxRope::setupProperties() {
   //     RenderableFieldlinesSequence::setupProperties();

        addProperty(_loadNewPointsBtn);
        addProperty(_fluxRopeFilePath);
        definePropertyCallbackFunctions();
    }

    bool RenderableFluxRope::loadNewJsonStateIntoRAM() {

        if (!_isLoadingStateFromDisk && !_newStateIsReady) {
            _isLoadingStateFromDisk = true;
     
            std::string filePath = _fluxRopeFilePath.value();
            std::thread readBinaryThread([this, f = std::move(filePath)]() {
                readNewJsonState(f);
                });
            readBinaryThread.detach();
        }

        return true;
    }

    fls::Model stringToModelFR(std::string str) {
        std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            [](char c) { return static_cast<char>(::tolower(c)); }
        );
        return fls::stringToModel(str);
    }

    void RenderableFluxRope::readNewJsonState(const std::string& filePath) {
        fls::Model model = stringToModelFR(_modelStr);
        _newState = std::make_unique<FieldlinesState>();
        if (_newState->loadStateFromJson(filePath,
            model,
            _scalingFactor)) {
            LWARNING("New state loaded");
            _newStateIsReady = true;
        }
        _isLoadingStateFromDisk = false;
        _hasBeenUpdated = false; 

    }

}
