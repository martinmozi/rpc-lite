add_executable(client client.cpp)
add_executable(server server.cpp)

target_link_libraries(client RpcLite::rpc-client_static)
target_link_libraries(server RpcLite::rpc-server_static)
