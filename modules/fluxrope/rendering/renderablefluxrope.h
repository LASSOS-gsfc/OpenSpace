#ifndef __OPENSPACE_MODULE_FLUXROPE___RENDERABLEFLUXROPE___H__
#define __OPENSPACE_MODULE_FLUXROPE___RENDERABLEFLUXROPE___H__

#include <OpenSpace/rendering/renderable.h>

#include <modules/fieldlinessequence/rendering/renderablefieldlinessequence.h>
#include <openspace/properties/triggerproperty.h>
#include <openspace/properties/stringproperty.h>
#include <modules/fieldlinessequence/util/fieldlinesstate.h>

namespace openspace {
    class RenderableFluxRope : public RenderableFieldlinesSequence {
    public:
        RenderableFluxRope(const ghoul::Dictionary& dictionary);
        void initializeGL() override;


        static documentation::Documentation Documentation();

    private: 
 
        void definePropertyCallbackFunctions();
        void setupProperties();
        bool loadNewJsonStateIntoRAM();
        void readNewJsonState(const std::string& filePath);

        properties::TriggerProperty _loadNewPointsBtn;
        properties::TriggerProperty _runPythonBtn;
        properties::StringProperty _fluxRopeFilePath;
    };
}

#endif 
