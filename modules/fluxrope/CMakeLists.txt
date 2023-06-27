include(${PROJECT_SOURCE_DIR}/support/cmake/module_definition.cmake)

set(HEADER_FILES
  rendering/renderablefluxrope.h
)
source_group("Header Files" FILES ${HEADER_FILES})

set(SOURCE_FILES
  rendering/renderablefluxrope.cpp
)
source_group("Source Files" FILES ${SOURCE_FILES})

set(SHADER_FILES
  shaders/fieldlinessequence_vs.glsl
  shaders/fieldlinessequence_fs.glsl
)
source_group("Shader Files" FILES ${SHADER_FILES})


create_new_module(
  "FluxRope"
  fluxrope_module
  ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES}
)
