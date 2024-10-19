
cmake_minimum_required(VERSION 3.11)
project(jaylib)

# Maybe set the date as a -D def and pass to main.c to override usual get_date()
# method?
set(SRC_DIR "./archive/2024-10-05/")
set(SRC_DIR "./archive/2024-10-13/")
set(SRC_DIR "./archive/2024-10-14/")
set(SRC_DIR "./archive/2024-10-19/")
#set(SRC_DIR ".")

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

