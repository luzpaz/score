include(debug)
set(SCORE_STATIC_PLUGINS True)
set(CMAKE_C_FLAGS "-g0")
set(CMAKE_CXX_FLAGS "-g0")
if(UNIX AND NOT APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS --disable-debug-symbols --disable-debug --no-map-whole-files --no-map-output-file)
  set(CMAKE_EXE_LINKER_FLAGS --disable-debug-symbols --disable-debug --no-map-whole-files --no-map-output-file)
  set(CMAKE_MODULE_LINKER_FLAGS --disable-debug-symbols --disable-debug --no-map-whole-files --no-map-output-file)
endif()
