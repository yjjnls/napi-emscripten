cmake_minimum_required(VERSION 3.11)
project(test)

file(GLOB_RECURSE  _SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.[ch] 
${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp
${CMAKE_CURRENT_SOURCE_DIR}/*.cc)


macro(conan_find_pkgconfig version )
    find_package(PkgConfig ${version} )
    if( NOT PKG_CONFIG_FOUND )
        set( _version 0.29.2)
        
        if( MSVC )
           set( _basename "pkg-config-${_version}-windows")
           set( _tarname  "${_basename}.tar.gz")
           set( _hash e247a276562398946507abde0a8bcb1f)
           set( _url ${CAM_TOOLS_URL}/${_tarname})
           set( _filename "${CMAKE_CURRENT_BINARY_DIR}/build-tools/.cache/${_tarname}")
           
           message(STATUS "Downloading pkg-config binary ...")
           file( DOWNLOAD ${_url} ${_filename}
           SHOW_PROGRESS
           INACTIVITY_TIMEOUT 60 # seconds
           EXPECTED_HASH MD5=${_hash} )

           execute_process(COMMAND ${CMAKE_COMMAND} -E tar xz "${_filename}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
           RESULT_VARIABLE __result )

           if(NOT __result EQUAL 0)
               message(FATAL_ERROR "error ${__result}")
           endif()

           set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/${_basename}/pkg-config")

        endif()
    endif()
    find_package(PkgConfig ${version} REQUIRED )
endmacro(conan_find_pkgconfig)

macro(conan_compiler_flags )
    if(MSVC)
    	ADD_DEFINITIONS( -D_CRT_SECURE_NO_DEPRECATE  )
    	ADD_DEFINITIONS( -D_CRT_NONSTDC_NO_DEPRECATE )
    	ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS   )
    	
    	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")
    else()
    	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")    	
    endif()
endmacro(conan_compiler_flags)

conan_find_pkgconfig( 0.29.1 )
conan_compiler_flags()
pkg_check_modules(_MODULES  REQUIRED
                   lept
                   tesseract
                 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${_MODULES_INCLUDE_DIRS}) 
link_directories   (${_MODULES_LIBRARY_DIRS})

message(${CMAKE_CURRENT_SOURCE_DIR})
message("----------------")
message("${_SOURCES}")
message("----------------")
add_executable(test test.cc )
target_link_libraries(test ${_MODULES_LIBRARIES} )