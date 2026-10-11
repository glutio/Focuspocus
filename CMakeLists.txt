cmake_minimum_required(VERSION 3.16)
project(Focuspocus CXX)

# Host build of the hardware-independent core. Arduino IDE/CLI ignores this
# file and compiles src/ directly.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(focuspocus STATIC
  src/BBitmapButton.cpp
  src/BFocusManager.cpp
  src/BGraphics.cpp
  src/BMouse.cpp
  src/BProgressBar.cpp
  src/BScrollbar.cpp
  src/BTextLabel.cpp
  src/BView.cpp
)
target_include_directories(focuspocus PUBLIC src)

if(WIN32)
  add_library(focuspocus_win32 STATIC
    extras/win32/BWin32Graphics.cpp
    extras/win32/BWin32Window.cpp
  )
  target_include_directories(focuspocus_win32 PUBLIC extras/win32)
  target_compile_definitions(focuspocus_win32 PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
  target_link_libraries(focuspocus_win32 PUBLIC focuspocus gdi32 user32)

  add_executable(focuspocus_demo extras/win32/demo.cpp)
  target_include_directories(focuspocus_demo PRIVATE examples/Demo)
  target_link_libraries(focuspocus_demo PRIVATE focuspocus_win32)

  add_executable(focuspocus_layoutfocus extras/win32/layoutfocus.cpp)
  target_include_directories(focuspocus_layoutfocus PRIVATE examples/LayoutFocus)
  target_link_libraries(focuspocus_layoutfocus PRIVATE focuspocus_win32)

  enable_testing()
  file(GLOB FOCUSPOCUS_TESTS CONFIGURE_DEPENDS test/*.cpp)
  add_executable(focuspocus_tests ${FOCUSPOCUS_TESTS})
  target_include_directories(focuspocus_tests PRIVATE test test/third_party)
  target_link_libraries(focuspocus_tests PRIVATE focuspocus_win32)
  add_test(NAME focuspocus_tests COMMAND focuspocus_tests)
endif()
