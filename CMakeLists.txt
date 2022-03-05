cmake_minimum_required(VERSION 3.1)

project(TransportCatalog)


include_directories(include)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

add_executable(TransportCatalog 
    src/main.cpp
    src/domain.cpp
    src/json.cpp
    src/map_renderer.cpp
    src/transport_catalogue.cpp
    src/geo.cpp
    src/json_reader.cpp
    src/request_handler.cpp
    src/transport_router.cpp
    src/json_builder.cpp
    src/svg.cpp
)

