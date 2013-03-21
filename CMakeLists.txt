project(onaprofile)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(onaprofile_SOURCES
    sources/selectprofilesdlg.cpp
    sources/profile.cpp
    sources/main.cpp)

set(onaprofile_HEADERS
    sources/selectprofilesdlg.hpp
    sources/profile.hpp)

set(onaprofile_UIS
    resources/selectprofilesdlg.ui)

set(onaprofile_RESOURCES
    resources/onaprofile.qrc)

qt4_wrap_cpp(onaprofile_HEADERS_MOC ${onaprofile_HEADERS})
qt4_add_resources(onaprofile_RESOURCES_RCC ${onaprofile_RESOURCES})
qt4_wrap_ui(onaprofile_HEADERS_UI ${onaprofile_UIS})

add_executable(onaprofile
               ${onaprofile_SOURCES}
               ${onaprofile_HEADERS_MOC}
               ${onaprofile_HEADERS_UI}
               ${onaprofile_RESOURCES_RCC})

target_link_libraries(onaprofile
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      qjson)

install(TARGETS onaprofile RUNTIME DESTINATION bin)
