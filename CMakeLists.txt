cmake_minimum_required(VERSION 3.16)

project(qlucidcamera
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(OpenCV 4.0 REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml Concurrent)

find_package(ArenaSDK QUIET)
if(NOT ArenaSDK_FOUND)
  find_package(Arena QUIET)
endif()

set(ARENA_SDK_ROOT "" CACHE PATH "Arena SDK root path (contains include/ and lib64/)")

if(NOT ArenaSDK_FOUND AND NOT Arena_FOUND)
  if(NOT ARENA_SDK_ROOT)
    find_path(ARENA_API_HEADER_DIR
      NAMES ArenaApi.h
      HINTS
        $ENV{ARENA_SDK_ROOT}
        $ENV{ARENA_SDK_DIR}
        /opt/ArenaSDK
        /usr/local/ArenaSDK
        /home/dev/ArenaViewMP_v_1.0.0.10_Linux_x64/ArenaSDK_Linux_x64
      PATH_SUFFIXES
        include/Arena
        Arena/include
    )
    if(ARENA_API_HEADER_DIR)
      get_filename_component(_arena_include_dir "${ARENA_API_HEADER_DIR}" DIRECTORY)
      get_filename_component(ARENA_SDK_ROOT "${_arena_include_dir}" DIRECTORY)
    endif()
  endif()

  if(ARENA_SDK_ROOT)
    find_library(ARENA_LIBRARY
      NAMES arena
      PATHS
        ${ARENA_SDK_ROOT}/lib64
        ${ARENA_SDK_ROOT}/lib
      NO_DEFAULT_PATH
    )
    find_library(SAVE_LIBRARY
      NAMES save
      PATHS
        ${ARENA_SDK_ROOT}/lib64
        ${ARENA_SDK_ROOT}/lib
      NO_DEFAULT_PATH
    )

    if(ARENA_LIBRARY AND SAVE_LIBRARY)
      add_library(qlucidcamera_arena_sdk INTERFACE)
      target_include_directories(qlucidcamera_arena_sdk INTERFACE
        ${ARENA_SDK_ROOT}/include
        ${ARENA_SDK_ROOT}/include/Arena
        ${ARENA_SDK_ROOT}/include/Save
        ${ARENA_SDK_ROOT}/GenICam/library/CPP/include
      )
      target_link_libraries(qlucidcamera_arena_sdk INTERFACE
        ${ARENA_LIBRARY}
        ${SAVE_LIBRARY}
      )

      if(UNIX AND NOT APPLE)
        target_link_directories(qlucidcamera_arena_sdk INTERFACE
          ${ARENA_SDK_ROOT}/lib64
          ${ARENA_SDK_ROOT}/lib
        )
      endif()
    endif()
  endif()

  if(NOT TARGET qlucidcamera_arena_sdk)
    message(FATAL_ERROR "qlucidcamera requires Arena SDK. Could not find package 'ArenaSDK' or 'Arena', and fallback search via ARENA_SDK_ROOT failed.")
  endif()
endif()

set(SOURCES
  src/lucid_camera.cpp
  src/lucid_camera_connector.cpp
)

set(PUBLIC_HEADERS
  include/libqlucidcamera-cpp/lucid_camera.h
  include/libqlucidcamera-cpp/lucid_camera_connector.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PUBLIC_HEADERS})

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libqlucidcamera-cpp
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::Concurrent
    ${OpenCV_LIBS}
)

if(TARGET Arena::ArenaSDK)
  target_link_libraries(${PROJECT_NAME} PUBLIC Arena::ArenaSDK)
elseif(TARGET ArenaSDK::ArenaSDK)
  target_link_libraries(${PROJECT_NAME} PUBLIC ArenaSDK::ArenaSDK)
elseif(TARGET Arena::Arena)
  target_link_libraries(${PROJECT_NAME} PUBLIC Arena::Arena)
elseif(TARGET ArenaSDK::Arena)
  target_link_libraries(${PROJECT_NAME} PUBLIC ArenaSDK::Arena)
elseif(TARGET qlucidcamera_arena_sdk)
  target_link_libraries(${PROJECT_NAME} PUBLIC qlucidcamera_arena_sdk)
endif()

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  DEBUG_POSTFIX "d"
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(TARGET qlucidcamera_arena_sdk)
  install(TARGETS qlucidcamera_arena_sdk
    EXPORT ${PROJECT_NAME}Targets
  )
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/libqlucidcamera-cpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qlucidcameraConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR}
)
