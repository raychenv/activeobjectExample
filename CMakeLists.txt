# define the project name
PROJECT (active-object)

set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)

message ("== Set pthread")
set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
set (THREADS_PREFER_PTHREAD_FLAG TRUE)
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   find_package (Threads REQUIRED)
endif ()

# define current component
SET (SOURCES
    "main.cpp"
    )

ADD_EXECUTABLE (${PROJECT_NAME}
    ${SOURCES}
    )

target_link_libraries (${PROJECT_NAME} PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    )
