cmake_minimum_required(VERSION 3.12)
project(typing_cat)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(utils/CompileUtility)
include(utils/InstallUtility)
include(utils/VersionUtility)

set(CMAKE_CXX_STANDARD 17)

# Generate compile flags.
generate_flags(TYPINGCAT_COMPILE_FLAGS "native" 2 true)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${TYPINGCAT_COMPILE_FLAGS}")

# Find libraries.
find_package(lambdacommon REQUIRED)

include_directories(${LAMBDACOMMON_INCLUDE_DIRS} libs/include)

# Now build the executable.
add_executable(typing_cat main.cpp)
target_link_libraries(typing_cat AperLambda::lambdacommon)
