cmake_minimum_required(VERSION 3.10)  # Update the minimum CMake version
project(VicsekModel)

# Define the executable
add_executable(Vicsek_Model src/main.cpp)  # Replace with your actual source files

# Check for OpenMP support
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    target_link_libraries(Vicsek_Model PRIVATE OpenMP::OpenMP_CXX)
    target_compile_options(Vicsek_Model PRIVATE ${OpenMP_CXX_FLAGS})
endif()

# Look for SDL2
find_package(SDL2 REQUIRED)

# Show SDL2 paths (optional, for debugging)
message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")

# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

# Link SDL2 libraries using keyword form
target_link_libraries(Vicsek_Model PRIVATE ${SDL2_LIBRARIES})
