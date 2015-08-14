cmake_minimum_required(VERSION 2.8.9)
project(cconf)


include(ExternalProject)


# c++11
# note: the -fPIC flag is for building with Qt...
#       it complains about position-independent code otherwise
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

# put executables in the bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# DEBUG
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# includes
include_directories(src)

# Qt5
if(APPLE)
    find_package(Qt5Widgets
        REQUIRED
        PATHS /usr/local/Cellar/qt5/*)
else()
    find_package(Qt5Widgets REQUIRED)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


ExternalProject_Add(jsoncpp-proj
    GIT_REPOSITORY ${PROJECT_SOURCE_DIR}/third_party/jsoncpp
    GIT_TAG master
    INSTALL_COMMAND "" # prevent global install
    UPDATE_COMMAND ""
)
link_directories(${CMAKE_BINARY_DIR}/jsoncpp-proj-prefix/src/jsoncpp-proj-build/lib)
include_directories(${CMAKE_BINARY_DIR}/jsoncpp-proj-prefix/src/jsoncpp-proj/include)


# the main executable
file(GLOB cconf_lib_SRC
    "src/ConfigContext.cpp"
)
add_library(cconf ${cconf_lib_SRC})
add_dependencies(cconf jsoncpp-proj)
target_link_libraries(cconf "jsoncpp")
qt5_use_modules(cconf Widgets Core)

add_executable(cconf-demo src/main.cpp)
target_link_libraries(cconf-demo cconf)
