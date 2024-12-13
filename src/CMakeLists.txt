set(ONLINE_SERVER_LIBNAME "cpsv")

file(GLOB CURRENT_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")
file(GLOB THREADPOOL_INC_FILES "${ONLINE_SERVER_INC_PATH}/threadpool/**.hpp")
file(GLOB CURRENT_INC_FILES "${ONLINE_SERVER_INC_PATH}/**.hpp")
file(GLOB THREADPOOL_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/threadpool/**.cpp")
file(GLOB LOGGER_INC_FILES "${ONLINE_SERVER_INC_PATH}/log/**.hpp")
file(GLOB LOGGER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/log/**.cpp")
file(GLOB_RECURSE OPENGAUSS_LIB_FILES "${ONLINE_SERVER_INC_PATH}/opengauss/**.h")
file(GLOB OPENGAUSS_INC_FILES "${ONLINE_SERVER_INC_PATH}/opengauss/**.hpp")
file(GLOB OPENGAUSS_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/opengauss/**.cpp")

add_library(
    "${ONLINE_SERVER_LIBNAME}"
    "${CURRENT_SRC_FILES}"
    "${CURRENT_INC_FILES}"
    "${THREADPOOL_INC_FILES}"
    "${THREADPOOL_SRC_FILES}"
    "${LOGGER_INC_FILES}"
    "${LOGGER_SRC_FILES}"
    "${OPENGAUSS_LIB_FILES}"
    "${OPENGAUSS_INC_FILES}"
    "${OPENGAUSS_SRC_FILES}"
)

target_include_directories(
    "${ONLINE_SERVER_LIBNAME}" PUBLIC
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${ONLINE_SERVER_INC_PATH}"
)

set_target_properties(
    "${ONLINE_SERVER_LIBNAME}" PROPERTIES
    VERSION ${CMAKE_VERSION}
    SOVERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}
    EXPORT_NAME core
)

add_library(
    cpsv::core ALIAS "${ONLINE_SERVER_LIBNAME}"
)