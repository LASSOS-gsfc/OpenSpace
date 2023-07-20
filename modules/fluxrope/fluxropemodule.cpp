#include <modules/fluxrope/fluxropemodule.h>

#include <modules/fluxrope/rendering/renderablefluxrope.h>
#include <openspace/rendering/renderable.h>
#include <openspace/util/factorymanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/templatefactory.h>
#include <openspace/documentation/documentation.h>

namespace openspace {

    FluxRopeModule::FluxRopeModule() : OpenSpaceModule(Name) {}

    void FluxRopeModule::internalInitialize(const ghoul::Dictionary&) {
        ghoul::TemplateFactory<Renderable>* fRenderable =
            FactoryManager::ref().factory<Renderable>();
        ghoul_assert(fRenderable, "No renderable factory existed");

        fRenderable->registerClass<RenderableFluxRope>("RenderableFluxRope");
    }

    std::vector<documentation::Documentation> FluxRopeModule::documentations() const
    {
        return {
            RenderableFluxRope::Documentation()
        };
    }

} // namespace openspace
