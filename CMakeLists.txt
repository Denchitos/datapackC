cmake_minimum_required(VERSION 4.4.0-rc3)
project(DPCompiler)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(dpcompiler ${APP_SOURCES})