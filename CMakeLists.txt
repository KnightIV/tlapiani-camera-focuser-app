cmake_minimum_required(VERSION 3.16)

include(FetchContent)

project(camera-focuser-app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(cpr CONFIG REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
)

# add_compile_options(
#   "$<$<COMPILE_LANGUAGE:CXX>:-DBUILD_SHARED_LIBS:BOOL=OFF>"
# )

# SET( CMAKE_CXX_FLAGS  "-DBUILD_SHARED_LIBS:BOOL=OFF")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(camera-focuser-app
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ipwi4client.h
        resources.qrc
        focuscontroller.h focuscontroller.cpp
        pwiclient.cpp
        pwiclient.cpp
        pwiclient.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET camera-focuser-app APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(camera-focuser-app SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(camera-focuser-app
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(camera-focuser-app PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(camera-focuser-app PRIVATE cpr::cpr)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.camera-focuser-app)
endif()
set_target_properties(camera-focuser-app PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS camera-focuser-app
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(camera-focuser-app)
endif()
