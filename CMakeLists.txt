cmake_minimum_required(VERSION 3.1)

project(Binpack)

# option(CXX "enable C++ compilation" ON)
if(CXX)
  enable_language(CXX)
endif()



set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


find_package(Gurobi)

include_directories(SYSTEM ${GUROBI_INCLUDE_DIRS})

FILE(GLOB MyCppSources src/*.cpp src/*.cc src/*/*.c src/*/*.cpp src/*/*/*.c)
add_executable(Binpack ${MyCppSources})

target_include_directories(Binpack PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/svm"
				       "${CMAKE_CURRENT_SOURCE_DIR}/src/linear_svm"
				       "${CMAKE_CURRENT_SOURCE_DIR}/src/linear_svm/blas")

target_link_libraries(  Binpack PRIVATE ${GUROBI_CXX_LIBRARY})
target_link_libraries(  Binpack PRIVATE ${GUROBI_LIBRARY})


find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(  Binpack PRIVATE dl   
			${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})