set(APP_NAME "online-chat-server")
add_executable(
    "${APP_NAME}" server.cpp
)

add_executable(client client.cpp)

# 测试opengauss数据库连接:
add_executable(opengauss-test test-main.cpp)
target_link_directories(
    opengauss-test PUBLIC
    "${OPENGAUSSCONNECTOR_PATH}"
)
target_include_directories(
    opengauss-test PUBLIC
    "{CMAKE_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "{ONLINE_SERVER_INC_PATH}"
)
target_link_libraries(
    opengauss-test 
    cpsv::core
    pq
)

target_link_directories(
    "${APP_NAME}" PUBLIC
    "${OPENGAUSSCONNECTOR_PATH}"
)
target_include_directories(
    "${APP_NAME}" PUBLIC
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${ONLINE_SERVER_INC_PATH}"
)

target_link_libraries(
    "${APP_NAME}"
    cpsv::core
    pq
)
