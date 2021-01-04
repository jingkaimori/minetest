cmake_minimum_required(VERSION 3.5)

# This can be read from ${PROJECT_NAME} after project() is called
project(minetest)
set(PROJECT_NAME_CAPITALIZED "Minetest")

set(CMAKE_CXX_STANDARD 11)
set(GCC_MINIMUM_VERSION "4.8")
set(CLANG_MINIMUM_VERSION "3.4")

# Also remember to set PROTOCOL_VERSION in network/networkprotocol.h when releasing
set(VERSION_MAJOR 5)
set(VERSION_MINOR 4)
set(VERSION_PATCH 0)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

# Change to false for releases
set(DEVELOPMENT_BUILD TRUE)

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
if(VERSION_EXTRA)
	set(VERSION_STRING ${VERSION_STRING}-${VERSION_EXTRA})
elseif(DEVELOPMENT_BUILD)
	set(VERSION_STRING "${VERSION_STRING}-dev")
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	# Append "-debug" to version string
	set(VERSION_STRING "${VERSION_STRING}-debug")
endif()

message(STATUS "*** Will build version ${VERSION_STRING} ***")


# Configuration options
set(DEFAULT_RUN_IN_PLACE FALSE)
if(WIN32)
	set(DEFAULT_RUN_IN_PLACE TRUE)
endif()
set(RUN_IN_PLACE ${DEFAULT_RUN_IN_PLACE} CACHE BOOL
	"Run directly in source directory structure")


set(BUILD_CLIENT TRUE CACHE BOOL "Build client")
set(BUILD_SERVER FALSE CACHE BOOL "Build server")
set(BUILD_UNITTESTS TRUE CACHE BOOL "Build unittests")


set(WARN_ALL TRUE CACHE BOOL "Enable -Wall for Release build")

if(NOT CMAKE_BUILD_TYPE)
	# Default to release
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type: Debug or Release" FORCE)
endif()

# Included stuff
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


# This is done here so that relative search paths are more reasonable
# Attempt to upgrade to IrrlichtBAW
option(ENABLE_IRRLICHTBAW "Use IrrlichtBAW" OFF)
if(ENABLE_IRRLICHTBAW)
	set(NBL_SOURCE_DIR "" CACHE PATH "Path to IrrlichtBAW source")
	set(NBL_BINARY_DIR "" CACHE PATH "Path to IrrlichtBAW binary")
	set(NBL_INSTALL_DIR "." CACHE PATH "Path to IrrlichtBAW install")
	list(APPEND NBL_CMAKE_ARGS "-DIRR_BUILD_DOCS:BOOL=OFF") # enable only if you have doxygen installed and detectable by cmake
	list(APPEND NBL_CMAKE_ARGS "-DIRR_BUILD_EXAMPLES:BOOL=OFF")
	list(APPEND NBL_CMAKE_ARGS "-DIRR_BUILD_TOOLS:BOOL=OFF") # the tools don't work yet (Apr 2020 status, might have changed since then)
	list(APPEND NBL_CMAKE_ARGS "-DIRR_BUILD_MITSUBA_LOADER:BOOL=OFF") # you probably don't want this extension
	list(APPEND NBL_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${NBL_INSTALL_DIR}") # because of https://gitlab.kitware.com/cmake/cmake/-/issues/18790
	ExternalProject_Add(Nabla
		DOWNLOAD_COMMAND  ""
		SOURCE_DIR        "${NBL_SOURCE_DIR}"
		BINARY_DIR        "${NBL_BINARY_DIR}"
		CMAKE_ARGS        ${NBL_CMAKE_ARGS}
		TEST_COMMAND      ""
	)

	#TODO: needs cleanup and adapt for minetest
	# replace with whatever you need ${PROJECT_NAME}
	target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:${NBL_INSTALL_DIR}/debug/include>
        $<$<CONFIG:RelWithDebInfo>:${NBL_INSTALL_DIR}/relwithdebinfo/include>
        $<$<CONFIG:Release>:${NBL_INSTALL_DIR}/include>
     # these are needed because we haven't cleaned up the API properly yet
        $<$<CONFIG:Debug>:${NBL_INSTALL_DIR}/debug/source/Nabla>
        $<$<CONFIG:RelWithDebInfo>:${NBL_INSTALL_DIR}/relwithdebinfo/source/Nabla>
        $<$<CONFIG:Release>:${NBL_INSTALL_DIR}/source/Nabla>
	)
	target_link_libraries(${PROJECT_NAME} 
		$<$<CONFIG:Debug>:${NBL_INSTALL_DIR}/debug/lib/Nabla_debug.lib>
		$<$<CONFIG:RelWithDebInfo>:${NBL_INSTALL_DIR}/relwithdebinfo/lib/Nabla_rwdi.lib>
		$<$<CONFIG:Release>:${NBL_INSTALL_DIR}/lib/Nabla.lib>
	)
	function(link_nbl_dependency DEPENDENCY_NAME)
		target_link_libraries(${PROJECT_NAME} 
			$<$<CONFIG:Debug>:${NBL_INSTALL_DIR}/debug/lib/${DEPENDENCY_NAME}d.lib>
			$<$<CONFIG:RelWithDebInfo>:${NBL_INSTALL_DIR}/relwithdebinfo/lib/${DEPENDENCY_NAME}.lib>
			$<$<CONFIG:Release>:${NBL_INSTALL_DIR}/lib/${DEPENDENCY_NAME}.lib>
		)
	endfunction()
	function(link_nbl_dependency_ DEPENDENCY_NAME)
		target_link_libraries(${PROJECT_NAME} 
			$<$<CONFIG:Debug>:${NBL_INSTALL_DIR}/debug/lib/${DEPENDENCY_NAME}_d.lib>
			$<$<CONFIG:RelWithDebInfo>:${NBL_INSTALL_DIR}/relwithdebinfo/lib/${DEPENDENCY_NAME}.lib>
			$<$<CONFIG:Release>:${NBL_INSTALL_DIR}/lib/${DEPENDENCY_NAME}.lib>
		)
	endfunction()
	link_nbl_dependency(glslang)
	link_nbl_dependency_(jpeg)
	link_nbl_dependency_(IlmImf-2_4)
	link_nbl_dependency_(IexMath-2_4)
	link_nbl_dependency_(Iex-2_4)
	link_nbl_dependency_(IlmThread-2_4)
	link_nbl_dependency_(Half-2_4)
	link_nbl_dependency_(Imath-2_4)
	link_nbl_dependency(libpng16_static)
	# OpenSSL only ever exists in the Release variant
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} 
			${NBL_INSTALL_DIR}/lib/libeay32.lib
			${NBL_INSTALL_DIR}/lib/ssleay32.lib
		)
	else()
		target_link_libraries(${PROJECT_NAME} 
			${NBL_INSTALL_DIR}/lib/libcrypto.lib
			${NBL_INSTALL_DIR}/lib/libssl.lib
		)
	endif()
	link_nbl_dependency_(shaderc)
	link_nbl_dependency_(shaderc_util)
	link_nbl_dependency(SPIRV)
	link_nbl_dependency_(SPIRV-Tools)
	link_nbl_dependency_(SPIRV-Tools-opt)
	link_nbl_dependency(OGLCompiler)
	link_nbl_dependency(OSDependent)
	link_nbl_dependency(HLSL)
	link_nbl_dependency(zlibstatic)
else()
	find_package(Irrlicht)
endif()


# Installation

if(WIN32)
	set(SHAREDIR ".")
	set(BINDIR "bin")
	set(DOCDIR "doc")
	set(EXAMPLE_CONF_DIR ".")
	set(LOCALEDIR "locale")
elseif(APPLE)
	set(BUNDLE_NAME ${PROJECT_NAME}.app)
	set(BUNDLE_PATH "${BUNDLE_NAME}")
	set(BINDIR ${BUNDLE_NAME}/Contents/MacOS)
	set(SHAREDIR ${BUNDLE_NAME}/Contents/Resources)
	set(DOCDIR "${SHAREDIR}/${PROJECT_NAME}")
	set(EXAMPLE_CONF_DIR ${DOCDIR})
	set(LOCALEDIR "${SHAREDIR}/locale")
elseif(UNIX) # Linux, BSD etc
	if(RUN_IN_PLACE)
		set(SHAREDIR ".")
		set(BINDIR "bin")
		set(DOCDIR "doc")
		set(EXAMPLE_CONF_DIR ".")
		set(MANDIR "unix/man")
		set(XDG_APPS_DIR "unix/applications")
		set(APPDATADIR "unix/metainfo")
		set(ICONDIR "unix/icons")
		set(LOCALEDIR "locale")
	else()
		set(SHAREDIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}")
		set(BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
		set(DOCDIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}")
		set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man")
		set(EXAMPLE_CONF_DIR ${DOCDIR})
		set(XDG_APPS_DIR "${CMAKE_INSTALL_PREFIX}/share/applications")
		set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/metainfo")
		set(ICONDIR "${CMAKE_INSTALL_PREFIX}/share/icons")
		set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
	endif()
endif()

set(CUSTOM_SHAREDIR "" CACHE STRING "Directory to install data files into")
if(NOT CUSTOM_SHAREDIR STREQUAL "")
	set(SHAREDIR "${CUSTOM_SHAREDIR}")
	message(STATUS "Using SHAREDIR=${SHAREDIR}")
endif()

set(CUSTOM_BINDIR "" CACHE STRING "Directory to install binaries into")
if(NOT CUSTOM_BINDIR STREQUAL "")
	set(BINDIR "${CUSTOM_BINDIR}")
	message(STATUS "Using BINDIR=${BINDIR}")
endif()

set(CUSTOM_DOCDIR "" CACHE STRING "Directory to install documentation into")
if(NOT CUSTOM_DOCDIR STREQUAL "")
	set(DOCDIR "${CUSTOM_DOCDIR}")
	if(NOT RUN_IN_PLACE)
		set(EXAMPLE_CONF_DIR ${DOCDIR})
	endif()
	message(STATUS "Using DOCDIR=${DOCDIR}")
endif()

set(CUSTOM_MANDIR "" CACHE STRING "Directory to install manpages into")
if(NOT CUSTOM_MANDIR STREQUAL "")
	set(MANDIR "${CUSTOM_MANDIR}")
	message(STATUS "Using MANDIR=${MANDIR}")
endif()

set(CUSTOM_EXAMPLE_CONF_DIR "" CACHE STRING "Directory to install example config file into")
if(NOT CUSTOM_EXAMPLE_CONF_DIR STREQUAL "")
	set(EXAMPLE_CONF_DIR "${CUSTOM_EXAMPLE_CONF_DIR}")
	message(STATUS "Using EXAMPLE_CONF_DIR=${EXAMPLE_CONF_DIR}")
endif()

set(CUSTOM_XDG_APPS_DIR "" CACHE STRING "Directory to install .desktop files into")
if(NOT CUSTOM_XDG_APPS_DIR STREQUAL "")
	set(XDG_APPS_DIR "${CUSTOM_XDG_APPS_DIR}")
	message(STATUS "Using XDG_APPS_DIR=${XDG_APPS_DIR}")
endif()

set(CUSTOM_ICONDIR "" CACHE STRING "Directory to install icons into")
if(NOT CUSTOM_ICONDIR STREQUAL "")
	set(ICONDIR "${CUSTOM_ICONDIR}")
	message(STATUS "Using ICONDIR=${ICONDIR}")
endif()

set(CUSTOM_LOCALEDIR "" CACHE STRING "Directory to install l10n files into")
if(NOT CUSTOM_LOCALEDIR STREQUAL "")
	set(LOCALEDIR "${CUSTOM_LOCALEDIR}")
	message(STATUS "Using LOCALEDIR=${LOCALEDIR}")
endif()


install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/builtin" DESTINATION "${SHAREDIR}")
if(RUN_IN_PLACE)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/mods/mods_here.txt" DESTINATION "${SHAREDIR}/mods")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/textures/texture_packs_here.txt" DESTINATION "${SHAREDIR}/textures")
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/games/minetest_game" DESTINATION "${SHAREDIR}/games/"
	COMPONENT "SUBGAME_MINETEST_GAME" OPTIONAL PATTERN ".git*" EXCLUDE )
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/games/devtest" DESTINATION "${SHAREDIR}/games/"
	COMPONENT "SUBGAME_MINIMAL" OPTIONAL PATTERN ".git*" EXCLUDE )

if(BUILD_CLIENT)
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/client/shaders" DESTINATION "${SHAREDIR}/client")
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/textures/base/pack" DESTINATION "${SHAREDIR}/textures/base")
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fonts" DESTINATION "${SHAREDIR}")
	if(RUN_IN_PLACE)
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/clientmods" DESTINATION "${SHAREDIR}")
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/client/serverlist" DESTINATION "${SHAREDIR}/client")
	endif()
endif()

install(FILES "README.md" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "doc/lua_api.txt" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "doc/client_lua_api.txt" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "doc/menu_lua_api.txt" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "doc/texture_packs.txt" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "doc/world_format.txt" DESTINATION "${DOCDIR}" COMPONENT "Docs")
install(FILES "minetest.conf.example" DESTINATION "${EXAMPLE_CONF_DIR}")

if(UNIX AND NOT APPLE)
	install(FILES "doc/minetest.6" "doc/minetestserver.6" DESTINATION "${MANDIR}/man6")
	install(FILES "misc/net.minetest.minetest.desktop" DESTINATION "${XDG_APPS_DIR}")
	install(FILES "misc/net.minetest.minetest.appdata.xml" DESTINATION "${APPDATADIR}")
	install(FILES "misc/minetest.svg" DESTINATION "${ICONDIR}/hicolor/scalable/apps")
	install(FILES "misc/minetest-xorg-icon-128.png"
		DESTINATION "${ICONDIR}/hicolor/128x128/apps"
		RENAME "minetest.png")
endif()

if(APPLE)
	install(FILES "misc/minetest-icon.icns" DESTINATION "${SHAREDIR}")
	install(FILES "misc/Info.plist" DESTINATION "${BUNDLE_PATH}/Contents")
endif()

# Library pack
find_package(GMP REQUIRED)
find_package(Json REQUIRED)
find_package(Lua REQUIRED)

# JsonCPP doesn't compile well on GCC 4.8
if(NOT ENABLE_SYSTEM_JSONCPP)
	set(GCC_MINIMUM_VERSION "4.9")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${GCC_MINIMUM_VERSION}")
		message(FATAL_ERROR "Insufficient gcc version, found ${CMAKE_CXX_COMPILER_VERSION}. "
			"Version ${GCC_MINIMUM_VERSION} or higher is required.")
	endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${CLANG_MINIMUM_VERSION}")
		message(FATAL_ERROR "Insufficient clang version, found ${CMAKE_CXX_COMPILER_VERSION}. "
			"Version ${CLANG_MINIMUM_VERSION} or higher is required.")
	endif()
endif()

# Subdirectories
# Be sure to add all relevant definitions above this

add_subdirectory(src)


# CPack

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A free open-source voxel game engine with easy modding and game creation.")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "celeron55")
set(CPACK_PACKAGE_CONTACT "Perttu Ahola <celeron55@gmail.com>")

include(CPackComponent)

cpack_add_component(Docs
	DISPLAY_NAME "Documentation"
	DESCRIPTION "Documentation about Minetest and Minetest modding"
)

cpack_add_component(SUBGAME_MINETEST_GAME
	DISPLAY_NAME "Minetest Game"
	DESCRIPTION "The default game bundled in the Minetest engine. Mainly used as a modding base."
	GROUP "Games"
)

cpack_add_component(SUBGAME_MINIMAL
	DISPLAY_NAME "Development Test"
	DESCRIPTION "A basic testing environment used for engine development and sometimes for testing mods."
	DISABLED #DISABLED does not mean it is disabled, and is just not selected by default.
	GROUP "Games"
)

cpack_add_component_group(Subgames
	DESCRIPTION "Games for the Minetest engine."
)

if(WIN32)
	# Include all dynamically linked runtime libaries such as MSVCRxxx.dll
	include(InstallRequiredSystemLibraries)

	if(RUN_IN_PLACE)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win64")
		else()
			set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win32")
		endif()

		set(CPACK_GENERATOR ZIP)

	else()
		set(CPACK_GENERATOR WIX)
		set(CPACK_PACKAGE_NAME "${PROJECT_NAME_CAPITALIZED}")
		set(CPACK_PACKAGE_INSTALL_DIRECTORY ".")
		set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "${PROJECT_NAME_CAPITALIZED}")
		set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})
		set(CPACK_PACKAGING_INSTALL_PREFIX "/${PROJECT_NAME_CAPITALIZED}")

		set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/misc/minetest-icon.ico")
		# Supported languages can be found at
		# http://wixtoolset.org/documentation/manual/v3/wixui/wixui_localization.html
		#set(CPACK_WIX_CULTURES "ar-SA,bg-BG,ca-ES,hr-HR,cs-CZ,da-DK,nl-NL,en-US,et-EE,fi-FI,fr-FR,de-DE")
		set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/misc/CPACK_WIX_UI_BANNER.BMP")
		set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/misc/CPACK_WIX_UI_DIALOG.BMP")

		set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/lgpl-2.1.txt")

		# The correct way would be to include both x32 and x64 into one installer
		# and install the appropriate one.
		# CMake does not support that, so there are two separate GUID's
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(CPACK_WIX_UPGRADE_GUID "745A0FB3-5552-44CA-A587-A91C397CCC56")
		else()
			set(CPACK_WIX_UPGRADE_GUID "814A2E2D-2779-4BBD-9ACD-FC3BD51FBBA2")
		endif()
	endif()
elseif(APPLE)
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-osx")
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-linux")
	set(CPACK_GENERATOR TGZ)
	set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)


# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
			${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif()
