cmake_minimum_required(VERSION 3.9)
project(unmo3)

include_directories(source)
set(SOURCES_UNMO3_FILE 
  ${PROJECT_SOURCE_DIR}/mo3_mp3.c
  ${PROJECT_SOURCE_DIR}/mo3_parse.c
  ${PROJECT_SOURCE_DIR}/mo3_unpack.c
  ${PROJECT_SOURCE_DIR}/unmo3.c
)
set(SOURCES_16TO8_FILE 
  ${PROJECT_SOURCE_DIR}/16to8.c
)

add_executable(unmo3 ${SOURCES_UNMO3_FILE})
add_executable(16to8 ${SOURCES_16TO8_FILE})

install(TARGETS unmo3 DESTINATION bin)
install(TARGETS 16to8 DESTINATION bin)

# add the uninstall support
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	@ONLY)
add_custom_target(uninstall
"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
