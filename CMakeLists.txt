cmake_minimum_required(VERSION 3.15)

project(sdklauncher CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE)
endif()

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	set(QT_INCLUDE "/usr/include/x86_64-linux-gnu/qt5")
endif()

if(WIN32)
	# On Windows, the install dir must be specified, as it's not standardized
	if(DEFINED QT_BASEDIR)
		# Add it to the prefix path so find_package can find it
		list(APPEND CMAKE_PREFIX_PATH ${QT_BASEDIR})
		set(QT_INCLUDE ${QT_BASEDIR}/include)
		# CMake has an odd policy that links a special link lib for Qt on newer versions of CMake. Enable it so we don't get spammed, and I get to write less
		cmake_policy(SET CMP0020 NEW)
	else()
		message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt5/here")
	endif()
endif()	
	
message("Using ${QT_INCLUDE} as our Qt include dir")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
list(APPEND LINK_LIBS Qt5::Widgets Qt5::Core Qt5::Gui)
list(APPEND INC_DIRS ${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets ${QT_INCLUDE}/QtGui ${QT_INCLUDE}/QtCore)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Steam
set(STEAM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/steam)
add_library(steam_api64 SHARED IMPORTED)
include_directories("${STEAM_DIR}/public/steam")
list(APPEND LINK_DIRS "${STEAM_DIR}/redistributable_bin")
if (WIN32)
	list(APPEND LINK_LIBS win64/steam_api64)
else()
	list(APPEND LINK_LIBS ${CMAKE_BINARY_DIR}/libsteam_api.so)
endif()

# Main Application
set(SRC
src/mainview.h
src/mainview.cpp

src/main.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	
add_executable(${PROJECT_NAME} ${SRC} res/resource.qrc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})
target_link_directories(${PROJECT_NAME} PUBLIC ${LINK_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIRS})

if(WIN32)
	# Set up the debugger so it can run the program without copying a million dlls
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")

	# Copy the steam windows binary
	configure_file("${STEAM_DIR}/redistributable_bin/win64/steam_api64.dll" "${CMAKE_BINARY_DIR}/steam_api64.dll" COPYONLY)
endif()

if(UNIX)
	configure_file("${STEAM_DIR}/redistributable_bin/linux64/libsteam_api.so" "${CMAKE_BINARY_DIR}/libsteam_api.so" COPYONLY)
endif()
