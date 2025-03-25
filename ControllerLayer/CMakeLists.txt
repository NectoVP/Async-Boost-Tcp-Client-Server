cmake_minimum_required(VERSION 3.10)
project(ControllerLayer)

add_library(${PROJECT_NAME}
    ErrorHandler.h
    ErrorHandler.cpp
    Listener.h
    Listener.cpp
    Session.h
    Session.cpp
    RequestHandler.h
    RequestHandler.cpp
)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})