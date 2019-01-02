cmake_minimum_required(VERSION 2.8)
project(stream_matrix)
file(GLOB_RECURSE  LWS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch] ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

find_package(PkgConfig 0.29.1 REQUIRED)

pkg_check_modules(GST_MODULES  REQUIRED
                  gstreamer-1.0>=1.14.0
                  gstreamer-base-1.0>=1.14.0
                  gstreamer-rtsp-server-1.0)

include_directories(${GST_MODULES_INCLUDE_DIRS}) 
link_directories   (${GST_MODULES_LIBRARY_DIRS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ADD_DEFINITIONS( -DGST_USE_UNSTABLE_API  )
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/utils)
add_library(stream_matrix  SHARED ${LWS_SOURCES} )
target_link_libraries(stream_matrix ${GST_MODULES_LIBRARIES} )

install(TARGETS stream_matrix
        LIBRARY DESTINATION lib 
        ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
        )