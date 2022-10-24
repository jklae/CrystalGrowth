CMAKE_MINIMUM_REQUIRED(VERSION 3.19.0)

# Set the project name
SET( PROJECT_NAME CrystalGrowth )
PROJECT(${PROJECT_NAME})

# Define character set as Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Set configuration types
Set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Set the include/lib directory
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ext/DXViewer/DXViewer-3.1.0/include)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/ext/DXViewer/DXViewer-3.1.0/lib)

# Copy DLLs
FILE(GLOB DLL ${CMAKE_SOURCE_DIR}/ext/DXViewer/DXViewer-3.1.0/bin/*.dll)
FILE(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR})

# Copy CSOs
FILE(GLOB CSO ${CMAKE_SOURCE_DIR}/ext/DXViewer/DXViewer-3.1.0/*.cso)
FILE(COPY ${CSO} DESTINATION ${CMAKE_BINARY_DIR})

# Collect source files
FILE( GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cpp )
FILE( GLOB HDR ${CMAKE_SOURCE_DIR}/src/*.h )

# Link Source files
ADD_EXECUTABLE( ${PROJECT_NAME} WIN32 ${SRC} ${HDR} )

# Set 'Additional Dependencies'
SET(LIB $<$<CONFIG:DEBUG>:DXViewer.lib> $<$<CONFIG:RELEASE>:DXViewerRel.lib>)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${LIB})