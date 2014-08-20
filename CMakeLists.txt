cmake_minimum_required(VERSION 2.8.9)
project(cconf)


include(ExternalProject)


# c++11
# note: the -fPIC flag is for building with Qt...
#       it complains about position-independent code otherwise
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

# includes
include_directories(src)


# # Qt5
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# find_package(Qt5Widgets REQUIRED)


ExternalProject_Add(VarTypes
    GIT_REPOSITORY git://github.com/RoboJackets/vartypes
    GIT_TAG master
    INSTALL_COMMAND "" # prevent global install
)


# the main executable
file(GLOB cconf_SRC
    "src/*.cpp"
)
add_executable("cconf-test" ${cconf_SRC})
add_dependencies("cconf-test" VarTypes)
link_directories({CMAKE_BINARY_DIR}/lib/VarTypes-prefix/src/VarTypes-build)
target_link_libraries("cconf-test" vartypes)
