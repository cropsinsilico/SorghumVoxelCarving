cmake_minimum_required(VERSION 2.6)
project(criticalKernelsThinning3D)

find_package(DGtal REQUIRED)
include_directories(${DGTAL_INCLUDE_DIRS})
link_directories(${DGTAL_LIBRARY_DIRS})

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS program_options REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(criticalKernelsThinning3D criticalKernelsThinning3D.cpp)
target_link_libraries(criticalKernelsThinning3D ${DGTAL_LIBRARIES})
target_link_libraries(criticalKernelsThinning3D ${Boost_LIBRARIES})
