set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/usr/jamoma/share/cmake/Jamoma")
set(ISCORE_COTIRE_DISABLE_UNITY True)

include(travis/debug)
include(all-plugins)
