set(APP_NAME "online-chat-server")
add_executable(
    "${APP_NAME}" main.cpp
)

add_executable(client client.cpp)

target_include_directories(
    "${APP_NAME}" PUBLIC
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${ONLINE_SERVER_INC_PATH}"
)

target_link_libraries(
    "${APP_NAME}"
    cpsv::core
)
