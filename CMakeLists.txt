# cmake_minimum_required(VERSION 3.0.0)
# project(sdl2Test VERSION 0.1.0)

# include(CTest)
# enable_testing()

# add_executable(sdl2Test src/main.cpp)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)



cmake_minimum_required(VERSION 3.0.0)
project(SDL2test VERSION 0.1.0)


find_package(SDL2 REQUIRED)


message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")


add_executable(SDL2test src/main.cpp)


include_directories(${SDL2_INCLUDE_DIRS})
#target_link_libraries(SDL2test SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(SDL2test ${SDL2_LIBRARIES} -lSDL2 -lSDL2main)


