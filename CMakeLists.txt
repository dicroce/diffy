cmake_minimum_required(VERSION 2.8)
project(diffy)

include(common.cmake NO_POLICY_SCOPE)
include(packaging.cmake NO_POLICY_SCOPE)

set(SOURCES source/Main.cpp
            source/MD5.cpp)

set(WINDOWS_LIBS XSDK)
set(LINUX_LIBS XSDK)

set(APPLICATION_TYPE "NORMAL")

include("${devel_artifacts_path}/build/base_app.cmake" NO_POLICY_SCOPE)

install(TARGETS diffy DESTINATION diffy)
install(FILES ${devel_artifacts_path}/lib/libXSDK.so DESTINATION diffy/libs)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/diffy_ignores DESTINATION /etc)
