#ifndef __OPENSPACE_MODULE_FLUXROPE___FLUXROPEMODULE___H__
#define __OPENSPACE_MODULE_FLUXROPE___FLUXROPEMODULE___H__

#include <openspace/util/openspacemodule.h>

namespace openspace {

    class FluxRopeModule : public OpenSpaceModule {
    public:
        constexpr static const char* Name = "FluxRope";

        FluxRopeModule();

        std::vector<documentation::Documentation> documentations() const override;

    protected:
        void internalInitialize(const ghoul::Dictionary&) override;
    };
}

#endif
