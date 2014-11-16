cmake_minimum_required(VERSION 2.8.9)
project(cconf)


include(ExternalProject)


# c++11
# note: the -fPIC flag is for building with Qt...
#       it complains about position-independent code otherwise
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

# includes
include_directories(src)

# TODO: include a FindQt5.cmake instead of hard-coding this
if(APPLE)
    set (CMAKE_PREFIX_PATH "/usr/local/Cellar/qt5/5.3.2/lib/cmake/Qt5Widgets")
endif()

# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)


ExternalProject_Add(jsoncpp-proj
    GIT_REPOSITORY git://github.com/open-source-parsers/jsoncpp
    GIT_TAG master
    INSTALL_COMMAND "" # prevent global install
)
link_directories(${CMAKE_BINARY_DIR}/jsoncpp-proj-prefix/src/jsoncpp-proj-build/lib)


# the main executable
file(GLOB cconf_SRC
    "src/*.cpp"
)
add_executable("cconf-test" ${cconf_SRC})
add_dependencies(cconf-test jsoncpp-proj)
target_link_libraries(cconf-test "jsoncpp")
qt5_use_modules("cconf-test" Widgets)
