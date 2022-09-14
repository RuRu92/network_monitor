

function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS FRAMEWORKS_DIRS)
    if(APPLE)
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAMES ${_FRAMEWORK} PATHS ${FRAMEWORKS_DIRS} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${FRAMEWORKS_DIRS}")
            endif()
        endforeach()
    endif()
endmacro()


function(conan_package_library_targets libraries package_libdir deps out_libraries out_libraries_target build_type package_name)
    unset(_CONAN_ACTUAL_TARGETS CACHE)
    unset(_CONAN_FOUND_SYSTEM_LIBS CACHE)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAMES ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            list(APPEND _out_libraries ${CONAN_FOUND_LIBRARY})
            if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
                # Create a micro-target for each lib/a found
                string(REGEX REPLACE "[^A-Za-z0-9.+_-]" "_" _LIBRARY_NAME ${_LIBRARY_NAME})
                set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${build_type})
                if(NOT TARGET ${_LIB_NAME})
                    # Create a micro-target for each lib/a found
                    add_library(${_LIB_NAME} UNKNOWN IMPORTED)
                    set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${CONAN_FOUND_LIBRARY})
                    set(_CONAN_ACTUAL_TARGETS ${_CONAN_ACTUAL_TARGETS} ${_LIB_NAME})
                else()
                    conan_message(STATUS "Skipping already existing target: ${_LIB_NAME}")
                endif()
                list(APPEND _out_libraries_target ${_LIB_NAME})
            endif()
            conan_message(STATUS "Found: ${CONAN_FOUND_LIBRARY}")
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            list(APPEND _out_libraries_target ${_LIBRARY_NAME})
            list(APPEND _out_libraries ${_LIBRARY_NAME})
            set(_CONAN_FOUND_SYSTEM_LIBS "${_CONAN_FOUND_SYSTEM_LIBS};${_LIBRARY_NAME}")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
        # Add all dependencies to all targets
        string(REPLACE " " ";" deps_list "${deps}")
        foreach(_CONAN_ACTUAL_TARGET ${_CONAN_ACTUAL_TARGETS})
            set_property(TARGET ${_CONAN_ACTUAL_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES "${_CONAN_FOUND_SYSTEM_LIBS};${deps_list}")
        endforeach()
    endif()

    set(${out_libraries} ${_out_libraries} PARENT_SCOPE)
    set(${out_libraries_target} ${_out_libraries_target} PARENT_SCOPE)
endfunction()


include(FindPackageHandleStandardArgs)

conan_message(STATUS "Conan: Using autogenerated Findnlohmann_json.cmake")
# Global approach
set(nlohmann_json_FOUND 1)
set(nlohmann_json_VERSION "3.11.2")

find_package_handle_standard_args(nlohmann_json REQUIRED_VARS
                                  nlohmann_json_VERSION VERSION_VAR nlohmann_json_VERSION)
mark_as_advanced(nlohmann_json_FOUND nlohmann_json_VERSION)


set(nlohmann_json_INCLUDE_DIRS "C:/Users/Ruslan/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(nlohmann_json_INCLUDE_DIR "C:/Users/Ruslan/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(nlohmann_json_INCLUDES "C:/Users/Ruslan/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(nlohmann_json_RES_DIRS )
set(nlohmann_json_DEFINITIONS )
set(nlohmann_json_LINKER_FLAGS_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(nlohmann_json_COMPILE_DEFINITIONS )
set(nlohmann_json_COMPILE_OPTIONS_LIST "" "")
set(nlohmann_json_COMPILE_OPTIONS_C "")
set(nlohmann_json_COMPILE_OPTIONS_CXX "")
set(nlohmann_json_LIBRARIES_TARGETS "") # Will be filled later, if CMake 3
set(nlohmann_json_LIBRARIES "") # Will be filled later
set(nlohmann_json_LIBS "") # Same as nlohmann_json_LIBRARIES
set(nlohmann_json_SYSTEM_LIBS )
set(nlohmann_json_FRAMEWORK_DIRS )
set(nlohmann_json_FRAMEWORKS )
set(nlohmann_json_FRAMEWORKS_FOUND "") # Will be filled later
set(nlohmann_json_BUILD_MODULES_PATHS )

conan_find_apple_frameworks(nlohmann_json_FRAMEWORKS_FOUND "${nlohmann_json_FRAMEWORKS}" "${nlohmann_json_FRAMEWORK_DIRS}")

mark_as_advanced(nlohmann_json_INCLUDE_DIRS
                 nlohmann_json_INCLUDE_DIR
                 nlohmann_json_INCLUDES
                 nlohmann_json_DEFINITIONS
                 nlohmann_json_LINKER_FLAGS_LIST
                 nlohmann_json_COMPILE_DEFINITIONS
                 nlohmann_json_COMPILE_OPTIONS_LIST
                 nlohmann_json_LIBRARIES
                 nlohmann_json_LIBS
                 nlohmann_json_LIBRARIES_TARGETS)

# Find the real .lib/.a and add them to nlohmann_json_LIBS and nlohmann_json_LIBRARY_LIST
set(nlohmann_json_LIBRARY_LIST )
set(nlohmann_json_LIB_DIRS )

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_nlohmann_json_DEPENDENCIES "${nlohmann_json_FRAMEWORKS_FOUND} ${nlohmann_json_SYSTEM_LIBS} ")

conan_package_library_targets("${nlohmann_json_LIBRARY_LIST}"  # libraries
                              "${nlohmann_json_LIB_DIRS}"      # package_libdir
                              "${_nlohmann_json_DEPENDENCIES}"  # deps
                              nlohmann_json_LIBRARIES            # out_libraries
                              nlohmann_json_LIBRARIES_TARGETS    # out_libraries_targets
                              ""                          # build_type
                              "nlohmann_json")                                      # package_name

set(nlohmann_json_LIBS ${nlohmann_json_LIBRARIES})

foreach(_FRAMEWORK ${nlohmann_json_FRAMEWORKS_FOUND})
    list(APPEND nlohmann_json_LIBRARIES_TARGETS ${_FRAMEWORK})
    list(APPEND nlohmann_json_LIBRARIES ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${nlohmann_json_SYSTEM_LIBS})
    list(APPEND nlohmann_json_LIBRARIES_TARGETS ${_SYSTEM_LIB})
    list(APPEND nlohmann_json_LIBRARIES ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(nlohmann_json_LIBRARIES_TARGETS "${nlohmann_json_LIBRARIES_TARGETS};")
set(nlohmann_json_LIBRARIES "${nlohmann_json_LIBRARIES};")

set(CMAKE_MODULE_PATH "C:/Users/Ruslan/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "C:/Users/Ruslan/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_PREFIX_PATH})

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
    # Target approach
    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        if(nlohmann_json_INCLUDE_DIRS)
            set_target_properties(nlohmann_json::nlohmann_json PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                  "${nlohmann_json_INCLUDE_DIRS}")
        endif()
        set_property(TARGET nlohmann_json::nlohmann_json PROPERTY INTERFACE_LINK_LIBRARIES
                     "${nlohmann_json_LIBRARIES_TARGETS};${nlohmann_json_LINKER_FLAGS_LIST}")
        set_property(TARGET nlohmann_json::nlohmann_json PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     ${nlohmann_json_COMPILE_DEFINITIONS})
        set_property(TARGET nlohmann_json::nlohmann_json PROPERTY INTERFACE_COMPILE_OPTIONS
                     "${nlohmann_json_COMPILE_OPTIONS_LIST}")
        
    endif()
endif()

foreach(_BUILD_MODULE_PATH ${nlohmann_json_BUILD_MODULES_PATHS})
    include(${_BUILD_MODULE_PATH})
endforeach()
