#include <modules/fluxrope/rendering/renderablefluxrope.h>
#include <modules/fieldlinessequence/rendering/renderablefieldlinessequence.h>

#include <openspace/rendering/renderengine.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/openglstatecache.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/textureunit.h>
#include <modules/fieldlinessequence/fieldlinessequencemodule.h>
#include "renderablefluxrope.h"

namespace openspace {

    RenderableFluxRope::RenderableFluxRope(const ghoul::Dictionary& dictionary) : RenderableFieldlinesSequence(dictionary){}

}
