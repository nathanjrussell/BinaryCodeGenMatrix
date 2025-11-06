# discover and include every per-repo file under cmake/versions
get_filename_component(_vdir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE) # this file's dir
file(GLOB _version_files RELATIVE "${_vdir}" "${_vdir}/external_repos/*.cmake")
foreach(_vf IN LISTS _version_files)
    message(STATUS "Including external repo config: ${_vf}")
    include("${_vdir}/${_vf}")
endforeach()