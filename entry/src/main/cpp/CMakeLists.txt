cmake_minimum_required(VERSION 3.5.0)
project(ime_engine)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(engine SHARED
    napi_init.cpp
    core/EngineCore.cpp
)

target_include_directories(engine PRIVATE
    ${NATIVERENDER_ROOT_PATH}
    ${NATIVERENDER_ROOT_PATH}/core
)

# HarmonyOS native 接口库
find_package(ohos_napi REQUIRED)
target_link_libraries(engine PUBLIC ohos_napi)