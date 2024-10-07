
cmake_minimum_required(VERSION 3.11)
project(jaylib)

set(SRC_DIR "./archive/2024-10-05/")
set(SRC_DIR "./archive/2024-10-06/")
set(SRC_DIR "./archive/2024-10-07/")
set(SRC_DIR ".")

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# don't build the raylib examples
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("submodules/raylib/")

add_executable(
	${PROJECT_NAME}
	"${SRC_DIR}/main.c"
)

#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

