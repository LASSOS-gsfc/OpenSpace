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

    constexpr openspace::properties::Property::PropertyInfo PythonBtnInfo = {
      "RunPython",
      "Run python code",
      "Runs python code for generating points",
      openspace::properties::Property::Visibility::NoviceUser
    };
    constexpr openspace::properties::Property::PropertyInfo ParamsInfo = {
      "FluxRopeParams",
      "Input params for flux rope code",
      "Runs python code for generating points",
      openspace::properties::Property::Visibility::NoviceUser
    };

    struct [[codegen::Dictionary(RenderableFluxRope)]] Parameters {

        std::string scriptPath;
        std::vector<std::string> propNames;
        std::vector<double> initialVals;
        std::string pythonPath;
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
        _scriptPath(PathToPointsFileInfo ),
        _runPythonBtn(PythonBtnInfo)
    {
        const Parameters p = codegen::bake<Parameters>(dictionary);

        _scriptPath = absPath(p.scriptPath).string();

        _propNames = p.propNames;

        _pythonPath = p.pythonPath;

        _fluxRopePath = p.fluxRopePath;

        auto iVal = p.initialVals.begin();

        for (auto& name : _propNames) {
            openspace::properties::Property::PropertyInfo info = {
                name.data(),name.data(),name.data(),
                 openspace::properties::Property::Visibility::NoviceUser
            };
            properties::StringProperty props(info);
            props = std::to_string(*iVal);
            _fluxRopeProps.insert({ name,props });
            ++iVal;

        }
    
    }

    void RenderableFluxRope::initializeGL() {
        RenderableFieldlinesSequence::initializeGL();

        setupProperties();
    }

    void RenderableFluxRope::definePropertyCallbackFunctions() {
        _loadNewPointsBtn.onChange([this]() {
            _loadingStatesDynamically = true; 
            if (!_scriptPath.value().empty()) loadNewJsonStateIntoRAM();
            });


        _runPythonBtn.onChange([this]() {
 
            std::string argv = "\"{";
            for (auto& _prop : _fluxRopeProps) argv += "\\\"" + _prop.first + "\\\":"+_prop.second.value() + ",";
            argv.pop_back();
            argv += "}\"";
            std::string command = _pythonPath + " \"" + _scriptPath.value() + "\" " + argv;
            LWARNING(fmt::format("running command at: {}", command));
        //    std::system("C:\\Python310\\python.exe \"C:\\Users\\eatoc\\OneDrive\\Documents\\My Documents\\NASA\\Flux rope file\\main.py \"");
            std::system(command.data());
            });
        /**/
       

    }

    void RenderableFluxRope::setupProperties() {
   //     RenderableFieldlinesSequence::setupProperties();

        addProperty(_loadNewPointsBtn);
        addProperty(_scriptPath);
        addProperty(_runPythonBtn);
        for (auto& _prop : _fluxRopeProps) {
            addProperty(_prop.second);
        }
        definePropertyCallbackFunctions();
    }

    bool RenderableFluxRope::loadNewJsonStateIntoRAM() {

        if (!_isLoadingStateFromDisk && !_newStateIsReady) {
            _isLoadingStateFromDisk = true;

            std::string filePath = _fluxRopePath;

            // make again
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
            _newStateIsReady = true;
        }
        _isLoadingStateFromDisk = false;
        _hasBeenUpdated = false; 

    }

}
