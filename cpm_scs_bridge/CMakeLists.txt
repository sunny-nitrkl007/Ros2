cmake_minimum_required(VERSION 3.16)
project(cpm_scs_bridge)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(generated_interfaces REQUIRED)   # WeighStatus.msg lives here

# AIS framework paths — set by the AIS SDK (eta-ais) build environment.
# In the CAT cross-compile environment these are injected by scons-build.sh.
# For local ROS2 builds, point to your AIS SDK sysroot:
#   cmake -DAIS_SDK_ROOT=/path/to/eta-ais ..
if(DEFINED AIS_SDK_ROOT)
    include_directories(${AIS_SDK_ROOT}/include)
    link_directories(${AIS_SDK_ROOT}/lib)
endif()

# ── Bridge library ────────────────────────────────────────────────────────────
add_library(${PROJECT_NAME} SHARED
    src/WeighScsRosBridge.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(${PROJECT_NAME}
    rclcpp
    generated_interfaces
)

# AIS runtime libraries (provided by eta-ais SDK)
target_link_libraries(${PROJECT_NAME}
    ais_task        # task::Task, task::InterfaceDb
    ais_interfaces  # InputInterface<T>
    ais_log         # AIS_LOG_*
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

ament_package()
