cmake_minimum_required(VERSION 3.8)
project(lps_core LANGUAGES C CXX)
find_package(ament_cmake REQUIRED)

option(LPS_ENABLE_PASS_TRACKER "Build the existing legacy Pass Tracker C core" OFF)
set(LPS_LEGACY_ROOT "" CACHE PATH "Super-project root containing lps_pass_tracker")

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(LPS_ENABLE_PASS_TRACKER)
  if(NOT IS_DIRECTORY "${LPS_LEGACY_ROOT}/lps_pass_tracker/src")
    message(FATAL_ERROR "LPS_LEGACY_ROOT must contain lps_pass_tracker/src")
  endif()
  file(GLOB LPS_PASS_TRACKER_SOURCES CONFIGURE_DEPENDS
    "${LPS_LEGACY_ROOT}/lps_pass_tracker/src/*.c")
  if(NOT LPS_PASS_TRACKER_SOURCES)
    message(FATAL_ERROR "No Pass Tracker C sources found")
  endif()

  add_library(lps_pass_tracker_legacy STATIC ${LPS_PASS_TRACKER_SOURCES})
  set_target_properties(lps_pass_tracker_legacy PROPERTIES
    C_STANDARD 99 C_STANDARD_REQUIRED YES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(lps_pass_tracker_legacy PUBLIC
    "${LPS_LEGACY_ROOT}/lps_pass_tracker/include"
    "${LPS_LEGACY_ROOT}/lps_app_common/include"
    "${LPS_LEGACY_ROOT}/eta-ais/prod/coretech/include"
    "${LPS_LEGACY_ROOT}/lps_common/include")

  add_library(lps_pass_tracker_adapter STATIC src/pass_tracker_adapter.cpp)
  target_compile_features(lps_pass_tracker_adapter PUBLIC cxx_std_17)
  target_compile_definitions(lps_pass_tracker_adapter PUBLIC LPS_HAS_PASS_TRACKER=1)
  target_include_directories(lps_pass_tracker_adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    "${LPS_LEGACY_ROOT}/lps_pass_tracker/include"
    "${LPS_LEGACY_ROOT}/lps_app_common/include"
    "${LPS_LEGACY_ROOT}/eta-ais/prod/coretech/include"
    "${LPS_LEGACY_ROOT}/lps_common/include")
  target_link_libraries(lps_pass_tracker_adapter PUBLIC lps_pass_tracker_legacy)

  install(TARGETS lps_pass_tracker_legacy lps_pass_tracker_adapter
    EXPORT export_${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
endif()

install(DIRECTORY include/ DESTINATION include)
install(TARGETS ${PROJECT_NAME} EXPORT export_${PROJECT_NAME})
ament_export_targets(export_${PROJECT_NAME})
ament_export_include_directories(include)
if(LPS_ENABLE_PASS_TRACKER)
  ament_export_libraries(lps_pass_tracker_adapter lps_pass_tracker_legacy)
  ament_export_definitions(LPS_HAS_PASS_TRACKER=1)
endif()
ament_package()
