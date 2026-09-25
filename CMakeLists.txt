cmake_minimum_required(VERSION 3.25)
project(NandStudio VERSION 0.1.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(NAND_BUILD_GUI "Build the Qt Quick application" ON)
option(NAND_SANITIZE "Enable address and undefined behavior sanitizers" OFF)
file(GLOB hdl_resources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/hdl/*.hdl")
set(hdl_header "#pragma once\n#include <string_view>\n#include <utility>\nnamespace nand { inline constexpr std::pair<std::string_view,std::string_view> builtinHdl[] = {\n")
foreach(hdl IN LISTS hdl_resources)
  get_filename_component(chip "${hdl}" NAME_WE)
  file(READ "${hdl}" contents)
  string(APPEND hdl_header "{\"${chip}\",R\"HDL(${contents})HDL\"},\n")
endforeach()
string(APPEND hdl_header "}; }\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin_hdl.hpp.in" "${hdl_header}")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/generated/builtin_hdl.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin_hdl.hpp" COPYONLY)
add_library(nand_core src/core.cpp src/jack.cpp src/vm.cpp src/script.cpp src/hdl.cpp)
target_include_directories(nand_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_include_directories(nand_core PUBLIC src)
if(MSVC)
  target_compile_options(nand_core PRIVATE /W4 /utf-8 /permissive-)
else()
  target_compile_options(nand_core PRIVATE -Wall -Wextra -Wpedantic)
  if(NAND_SANITIZE)
    target_compile_options(nand_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(nand_core PUBLIC -fsanitize=address,undefined)
  endif()
endif()
if(NOT ANDROID)
  foreach(tool IN ITEMS nand Assembler CPUEmulator VMEmulator JackCompiler TextComparer HardwareSimulator)
    add_executable(${tool} src/cli.cpp)
    target_link_libraries(${tool} PRIVATE nand_core)
    install(TARGETS ${tool} RUNTIME DESTINATION bin)
  endforeach()
endif()
include(CTest)
if(BUILD_TESTING AND NOT ANDROID)
  add_executable(core_tests tests/core_tests.cpp)
  target_link_libraries(core_tests PRIVATE nand_core)
  add_test(NAME core COMMAND core_tests)
  add_executable(hardware_tests tests/hardware_tests.cpp)
  target_link_libraries(hardware_tests PRIVATE nand_core)
  add_test(NAME hardware COMMAND hardware_tests)
endif()
if(NAND_BUILD_GUI)
  find_package(Qt6 6.11.1 EXACT REQUIRED COMPONENTS Quick QuickControls2 Concurrent)
  if(ANDROID)
    # Qt's GLES finder exports the NDK C sysroot as an explicit -isystem path.
    # That moves C headers ahead of libc++ wrappers. Clang already searches
    # this directory via --sysroot; remove only this redundant imported path.
    get_property(android_imports DIRECTORY PROPERTY IMPORTED_TARGETS)
    foreach(imported IN LISTS android_imports)
      get_target_property(import_includes ${imported} INTERFACE_INCLUDE_DIRECTORIES)
      if(import_includes)
        set(clean_includes "${import_includes}")
        list(FILTER clean_includes EXCLUDE REGEX "/sysroot/usr/include/?$")
        if(NOT "${clean_includes}" STREQUAL "${import_includes}")
          message(STATUS "Removing redundant NDK C include from ${imported}: ${import_includes}")
          set_property(TARGET ${imported} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${clean_includes}")
        endif()
      endif()
    endforeach()
  endif()
  qt_standard_project_setup(REQUIRES 6.11)
  qt_add_executable(NandStudio src/app/main.cpp src/app/studio.cpp src/app/studio.hpp src/app/editor_services.cpp src/app/editor_services.hpp src/app/storage.cpp src/app/storage.hpp)
  target_link_libraries(NandStudio PRIVATE nand_core Qt6::Quick Qt6::QuickControls2 Qt6::Concurrent)
  if(BUILD_TESTING AND NOT ANDROID)
    find_package(Qt6 6.11.1 EXACT REQUIRED COMPONENTS Test)
    target_sources(NandStudio PRIVATE tests/gui_checks.cpp)
    target_include_directories(NandStudio PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_link_libraries(NandStudio PRIVATE Qt6::Test)
    target_compile_definitions(NandStudio PRIVATE NAND_GUI_CHECKS)
    add_test(NAME gui_workflow COMMAND NandStudio --self-test ${CMAKE_CURRENT_BINARY_DIR}/gui-test)
    get_filename_component(qt_plugin_dir "${Qt6_DIR}/../../../plugins" ABSOLUTE)
    set_tests_properties(gui_workflow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_PLUGIN_PATH=${qt_plugin_dir};NAND_TEST_STATE_DIR=${CMAKE_CURRENT_BINARY_DIR}/gui-test/state" TIMEOUT 30)
  endif()
  set_source_files_properties(src/app/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
  qt_add_qml_module(NandStudio URI NandStudio VERSION 1.0 OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/NandStudio" QML_FILES
    src/app/ActionButton.qml
    src/app/AppDialog.qml
    src/app/AppField.qml
    src/app/AppToolbar.qml
    src/app/ConfirmDialog.qml
    src/app/ConflictDialog.qml
    src/app/ConsolePane.qml
    src/app/EditorPane.qml
    src/app/EditorTab.qml
    src/app/EditorWorkspace.qml
    src/app/GoToLineDialog.qml
    src/app/HardwareInspector.qml
    src/app/HardwareTrace.qml
    src/app/HardwareDiagram.qml
    src/app/MachinePane.qml
    src/app/Main.qml
    src/app/MemoryInspector.qml
    src/app/PathDialog.qml
    src/app/ScreenView.qml
    src/app/SearchBar.qml
    src/app/SettingsDialog.qml
    src/app/Theme.qml
    src/app/WorkspacePane.qml
  )
  set_target_properties(NandStudio PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
  target_compile_definitions(NandStudio PRIVATE NAND_VERSION="${PROJECT_VERSION}")
  if(ANDROID)
    set_target_properties(NandStudio PROPERTIES QT_ANDROID_MIN_SDK_VERSION 28 QT_ANDROID_TARGET_SDK_VERSION 36 QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}" QT_ANDROID_VERSION_CODE 3)
    target_link_options(NandStudio PRIVATE -Wl,-z,max-page-size=16384)
  endif()
  install(TARGETS NandStudio BUNDLE DESTINATION . RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
  if(NOT ANDROID)
    qt_generate_deploy_qml_app_script(TARGET NandStudio OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script})
  endif()
endif()
install(FILES NOTICE.md DESTINATION share/nandstudio)
install(FILES README.md CONTRIBUTING.md toolchain-lock.json DESTINATION share/nandstudio)
install(DIRECTORY docs/ DESTINATION share/nandstudio/docs FILES_MATCHING PATTERN "*.md" PATTERN "*.json")
install(DIRECTORY licenses/ DESTINATION share/nandstudio/licenses)
install(DIRECTORY resources/hdl DESTINATION share/nandstudio)
set(CPACK_PACKAGE_NAME NandStudio)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR ZIP)
include(CPack)
