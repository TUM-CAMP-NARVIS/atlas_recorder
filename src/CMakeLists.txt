SET(APP_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/recorder.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdparser.h"
)

SET(APP_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/recorder.cpp"
        )

add_executable(atlas_recorder ${APP_SOURCES} ${APP_HEADERS} )
set_property(TARGET atlas_recorder PROPERTY CXX_STANDARD 20)
set_target_properties(atlas_recorder PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(atlas_recorder PRIVATE
        CONAN_PKG::kinect-azure-sensor-sdk
        CONAN_PKG::fmt
        pthread
        )

target_include_directories(atlas_recorder PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        )

install(TARGETS atlas_recorder DESTINATION ${CMAKE_INSTALL_BINDIR})
