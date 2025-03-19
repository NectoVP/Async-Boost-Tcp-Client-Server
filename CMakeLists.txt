cmake_minimum_required(VERSION 3.25.0)
project(MyServerr)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost 1.83.0 REQUIRED COMPONENTS filesystem system thread chrono)

include_directories(${BOOST_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} main.cpp)

enable_testing()

add_subdirectory(DataLayer)
add_subdirectory(ServiceLayer)
add_subdirectory(tests)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        DataLayer
        ServiceLayer
    PUBLIC
        ${Boost_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC
        ${Boost_INCLUDE_DIRS}
)