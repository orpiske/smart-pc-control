if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DLINUX_BUILD -D_GNU_SOURCE)

	find_library(RT_LIB NAMES rt)
	message(STATUS "RT library found at ${RT_LIB}")

	SET(SYSTEMD_SUPPORT ON CACHE BOOL "Enable systemd support")
	# Fixed directory for service files
	set(SERVICE_INSTALL_PREFIX "" CACHE STRING "Install prefix for service files (for packaging only)")

	set(CMAKE_INSTALL_SYSTEMD_UNIT_PATH ${SERVICE_INSTALL_PREFIX}/usr/lib/systemd/system)
	set(CMAKE_BUILD_SYSTEMD_UNIT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_SYSTEMD_UNIT_PATH})

	# Fixed directory
	set(CMAKE_INSTALL_SYSCONFIG_PATH ${SERVICE_INSTALL_PREFIX}/etc/sysconfig)
	set(CMAKE_BUILD_SYSCONFIG_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_SYSCONFIG_PATH})

else (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "Compiling for " ${CMAKE_SYSTEM_NAME} "")

	set(RT_LIB "")

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		set(CMAKE_MACOSX_RPATH TRUE)
		add_definitions(-D__OSX__)
	endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_BINDIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_LIBDIR})
set (CTEST_BINARY_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/${CMAKE_INSTALL_BINDIR})

find_library(MATH_LIB NAMES m)
message(STATUS "Math library found at ${MATH_LIB}")


# GRU
find_path(GRU_INCLUDE_DIR common/gru_base.h
        PATH_SUFFIXES gru-0
				HINTS ${GRU_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(GRU_LIB NAMES gru-0
				HINTS ${GRU_DIR}/${CMAKE_INSTALL_LIBDIR})

message(STATUS "GRU headers found at ${GRU_INCLUDE_DIR}")
message(STATUS "GRU library found at ${GRU_LIB}")

# JSON C
find_path(JSON_INCLUDE_DIR json.h
          PATH_SUFFIXES json json-c)

find_library(JSON_LIB NAMES json json-c)
link_libraries(${JSON_LIB})

message(STATUS "JSON headers found at ${JSON_INCLUDE_DIR}")
message(STATUS "JSON library found at ${JSON_LIB}")

# URI Parser
find_path(URIPARSER_INCLUDE_DIR uriparser/Uri.h)
find_library(URIPARSER_LIB NAMES uriparser liburiparser)

message(STATUS "URIParser found on ${URIPARSER_INCLUDE_DIR}")
message(STATUS "URIParser library found at ${URIPARSER_LIB}")

# UUID
find_path(UUID_INCLUDE_DIR uuid/uuid.h)
find_library(UUID_LIB NAMES uuid libuuid)

message(STATUS "UUID headers found on ${UUID_INCLUDE_DIR}")
message(STATUS "UUID library found at ${UUID_LIB}")

# ZLib
find_path(ZLIB_INCLUDE_DIR zlib.h)
find_library(ZLIB_LIB NAMES z libz)

message(STATUS "zlib headers found on ${ZLIB_INCLUDE_DIR}")
message(STATUS "zlib library found at ${ZLIB_LIB}")

# Paho
set(PAHOC_DIR "/usr" CACHE STRING "Proton directory")

find_path(PAHOC_INCLUDE_DIR MQTTClient.h
		HINTS ${PAHOC_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(PAHOC_LIB NAMES paho-mqtt3c
		HINTS ${PAHOC_DIR}/${CMAKE_INSTALL_LIBDIR})

link_libraries(${PAHOC_LIB})

message(STATUS "Paho C hint ${PAHOC_DIR}")
message(STATUS "Paho C headers found at ${PAHOC_INCLUDE_DIR}")
message(STATUS "Paho C library found at ${PAHOC_LIB}")

include_directories(${PAHOC_INCLUDE_DIR})


# Installs service configuration files (ie.: for systemd daemons). For systemd daemons
# It requires 2 files: a <service_name>.in file, containing the service startup
# configuration and a <service_name.service.in, which is a systemd-compliant service
# file.
macro(AddService SERVICE_CONFIG_SOURCE SERVICE_NAME)
	if (${SYSTEMD_SUPPORT})
		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}@.service.in
				${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/${SERVICE_NAME}@.service
				@ONLY
				)

		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}.in
				${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}.set-me-up
				@ONLY
				)

		install(FILES
				${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/${SERVICE_NAME}@.service
				DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
				)

		install(FILES
				${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}.set-me-up
				DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
				)
	endif (${SYSTEMD_SUPPORT})
endmacro(AddService)

macro(AddScript SENSOR_NAME SCRIPT_NAME)
	configure_file(src/${SENSOR_NAME}/scripts/${SCRIPT_NAME}.in
		${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_FULL_LIBEXECDIR}/smart-pc-control/${SENSOR_NAME}/${SCRIPT_NAME}.sh
		@ONLY
	)

	install(PROGRAMS ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_FULL_LIBEXECDIR}/smart-pc-control/${SENSOR_NAME}/${SCRIPT_NAME}.sh
		DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/smart-pc-control/
	)
endmacro(AddScript)



