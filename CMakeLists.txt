project(onaprofile)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(onaprofile_SRCS onaprofile.cpp main.cpp)
qt4_automoc(${onaprofile_SRCS})
add_executable(onaprofile ${onaprofile_SRCS})
target_link_libraries(onaprofile ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS onaprofile RUNTIME DESTINATION bin)
