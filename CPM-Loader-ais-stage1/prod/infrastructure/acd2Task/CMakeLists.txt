# acd2Task — program (DATALINKMANAGER_SUPPORTED only)
if(NOT DATALINKMANAGER_SUPPORTED)
    message(STATUS "Skipping acd2Task (DATALINKMANAGER_SUPPORTED=OFF)")
    return()
endif()

add_executable(acd2Task
    Acd2Task.cpp
)

# Resolve the full transitive link dependency chain exactly as SCons does,
# by parsing the _libs/libs-*.txt files for the Acd2Task module.
ais_get_libs_list_from_module(Acd2Task _acd2task_libs)

# Wrap in --start-group / --end-group so the GNU linker iterates over the
# entire group until no new symbols are resolved (handles circular deps
# between static archives).
target_link_libraries(acd2Task PRIVATE
    -Wl,--start-group
    ${_acd2task_libs}
    -Wl,--end-group
)
install_binary(acd2Task)
