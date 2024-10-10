cmake_minimum_required(VERSION 3.10)
project(MarkdownPreview)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework CoreServices")
endif()

# Add cmark
add_subdirectory(external/cmark)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src)

# Add MacOS
include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreFoundation.framework/Headers)

# Add cpp-httplib
add_subdirectory(external/cpp-httplib)

# Main executable
add_executable(md_preview
    src/main.cc
    src/md2html.cc
    src/file_watcher.cc
)

target_include_directories(md_preview PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-httplib
)
target_link_libraries(md_preview PRIVATE cmark httplib "-framework CoreServices")