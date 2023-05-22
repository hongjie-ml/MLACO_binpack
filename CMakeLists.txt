cmake_minimum_required(VERSION 3.1)

project(Binpack)

# option(CXX "enable C++ compilation" ON)
if(CXX)
  enable_language(CXX)
endif()



set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


find_package(Gurobi)
include_directories(SYSTEM ${GUROBI_INCLUDE_DIRS})

FILE(GLOB MyCppSources src/*.cpp)
add_executable(Binpack ${MyCppSources})


target_link_libraries(  Binpack PRIVATE ${GUROBI_CXX_LIBRARY})
target_link_libraries(  Binpack PRIVATE ${GUROBI_LIBRARY})
