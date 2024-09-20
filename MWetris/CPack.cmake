install(TARGETS MWetris
	RUNTIME DESTINATION .
	COMPONENT application
)

install(DIRECTORY ${MWetrisData_SOURCE_DIR}/fonts/
	DESTINATION fonts
	COMPONENT data
)

install(DIRECTORY ${MWetrisData_SOURCE_DIR}/images/
	DESTINATION images
	COMPONENT data
)

install(DIRECTORY ${MWetrisData_SOURCE_DIR}/package/
	DESTINATION .
	COMPONENT data
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/
	DESTINATION .
	COMPONENT data
)

# Replace the USE_APPLICATION_JSON file, use default saving place.
install(FILES ${MWetris_SOURCE_DIR}/data/package/USE_APPLICATION_JSON
	DESTINATION .
	COMPONENT data
)

if (MSVC)
	# Tell CMake to install the windows runtime libraries to the programs
	# directory and tell CPack that they belong to the "application" component
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .) # May not work for debug dlls
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT true)
	include(InstallRequiredSystemLibraries)
	
	install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
		DESTINATION .
		COMPONENT application
	)
endif ()

set(CPACK_PACKAGE_CONTACT "mwthinker@yahoo.com")
set(CPACK_PACKAGE_NAME "MWetris")

set(CPACK_PACKAGE_VENDOR "mwthinker")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Installs MWetris")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MWetris")

set(CPACK_GENERATOR ZIP)

# Tell CPack about the components and group the data components together.
set(CPACK_COMPONENTS_ALL application data)

# More descriptive names for each of the components, and component groups.
set(CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "Application")
set(CPACK_COMPONENT_APPLICATION_REQUIRED true)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data")
set(CPACK_COMPONENT_DATA_REQUIRED true)

set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime")
set(CPACK_COMPONENT_RUNTIME_REQUIRED true)

# Text from "LICENSE.txt" is displayed in the installer's license tab.
file(COPY_FILE "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/WIX_LICENSE.txt") # WIX does not support empty extension
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/WIX_LICENSE.txt")
set(CPACK_PACKAGE_ICON "${CMAKE_BINARY_DIR}/images/icon.png")

set(CPACK_PACKAGE_EXECUTABLES "MWetris" "MWetris")
set(CPACK_CREATE_DESKTOP_LINKS "MWetris")

# ------------------- WIX ---------------------------------------

if (MSVC)
	option(USE_WIX_GENERATOR "Use WIX generator" OFF)
	if (USE_WIX_GENERATOR)
		set(CPACK_GENERATOR ${CPACK_GENERATOR} WIX)
		set(CPACK_WIX_PRODUCT_GUID "${GUID}")
		set(CPACK_WIX_UPGRADE_GUID "bf1ee6e4-389a-4567-ad2c-61a4d3c71d66")

		set(CPACK_WIX_UI_DIALOG "${MWetris_BINARY_DIR}/package/dialog.png")
		set(CPACK_WIX_UI_BANNER "${MWetris_BINARY_DIR}/package/banner.png")
		
		set(CPACK_WIX_PRODUCT_ICON "${MWetris_BINARY_DIR}/package/mwetris.ico")
		
		message(STATUS ${CPACK_PACKAGE_ICON})
		set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/mwthinker/MWetris")
	endif ()
endif ()

include(CPack)
