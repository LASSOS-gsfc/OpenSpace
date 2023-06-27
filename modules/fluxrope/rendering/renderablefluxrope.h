#ifndef __OPENSPACE_MODULE_FLUXROPE___RENDERABLEFLUXROPE___H__
#define __OPENSPACE_MODULE_FLUXROPE___RENDERABLEFLUXROPE___H__

#include <OpenSpace/rendering/renderable.h>

#include <modules/fieldlinessequence/rendering/renderablefieldlinessequence.h>

namespace openspace {
    class RenderableFluxRope : public RenderableFieldlinesSequence {
    public:
        RenderableFluxRope(const ghoul::Dictionary& dictionary);
    };
}

#endif 
