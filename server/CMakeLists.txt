cmake_minimum_required(VERSION 3.12)
project(ServerLayer)

add_library(${PROJECT_NAME}
            ItemHolder.h
            ItemHolder.cpp
            KitchenWorker.h
            KitchenWorker.cpp
            Server.h
            Server.cpp
)

find_package(Boost REQUIRED)

target_include_directories(${PROJECT_NAME} 
        PUBLIC 
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PUBLIC
            ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} 
        PUBLIC
            ${Boost_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})