# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_rsg_v1_three_node_cross_pubsub_matrix_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED rsg_v1_three_node_cross_pubsub_matrix_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(rsg_v1_three_node_cross_pubsub_matrix_FOUND FALSE)
  elseif(NOT rsg_v1_three_node_cross_pubsub_matrix_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(rsg_v1_three_node_cross_pubsub_matrix_FOUND FALSE)
  endif()
  return()
endif()
set(_rsg_v1_three_node_cross_pubsub_matrix_CONFIG_INCLUDED TRUE)

# output package information
if(NOT rsg_v1_three_node_cross_pubsub_matrix_FIND_QUIETLY)
  message(STATUS "Found rsg_v1_three_node_cross_pubsub_matrix: 0.1.0 (${rsg_v1_three_node_cross_pubsub_matrix_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'rsg_v1_three_node_cross_pubsub_matrix' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${rsg_v1_three_node_cross_pubsub_matrix_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(rsg_v1_three_node_cross_pubsub_matrix_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${rsg_v1_three_node_cross_pubsub_matrix_DIR}/${_extra}")
endforeach()
