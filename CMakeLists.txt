cmake_minimum_required(VERSION 3.25.0)
project(MyServerr)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost 1.83.0 REQUIRED COMPONENTS filesystem system thread chrono)

include_directories(${BOOST_INCLUDE_DIRS})
message("boost lib: ${Boost_LIBRARIES}")
message("boost inc:${Boost_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} main.cpp)

enable_testing()

add_subdirectory(server)
add_subdirectory(tests)

target_link_libraries(${PROJECT_NAME} 
        PUBLIC
            ServerLayer
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

target_link_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})