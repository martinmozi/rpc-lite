set(rpc-lite_VERSION 1.0.0)

if(WIN32)
   set(Sock_libs ws2_32 IPHLPAPI)
   add_definitions(-DWIN32)
   set(wepoll wepoll/wepoll.c)
endif()

add_library(rpc-client SHARED client.cpp os.cpp ${wepoll})
add_library(rpc-client_static STATIC client.cpp os.cpp ${wepoll})
set_property(TARGET rpc-client_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(rpc-client PROPERTIES PUBLIC_HEADER "iClient.h")
set_target_properties(rpc-client_static PROPERTIES PUBLIC_HEADER "iClient.h")
target_link_libraries(rpc-client ${Sock_libs} Threads::Threads)
target_link_libraries(rpc-client_static ${Sock_libs} Threads::Threads)

add_library(rpc-server SHARED balancer.cpp server.cpp os.cpp ${wepoll})
add_library(rpc-server_static STATIC balancer.cpp server.cpp os.cpp ${wepoll})
set_property(TARGET rpc-server_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(rpc-server PROPERTIES PUBLIC_HEADER "iServer.h")
set_target_properties(rpc-server_static PROPERTIES PUBLIC_HEADER "iServer.h")
target_link_libraries(rpc-server ${Sock_libs} Threads::Threads)
target_link_libraries(rpc-server_static ${Sock_libs} Threads::Threads)

configure_package_config_file(rpc-lite.cmake.in rpc-lite.cmake INSTALL_DESTINATION lib/cmake/rpc-lite)
write_basic_package_version_file(rpc-lite-config-version.cmake COMPATIBILITY SameMajorVersion)

install(TARGETS rpc-client_static
    EXPORT ${PROJECT_NAME}-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rpc_client)
    
install(TARGETS rpc-server_static
    EXPORT ${PROJECT_NAME}-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rpc_server)
    
install(TARGETS rpc-client
    EXPORT ${PROJECT_NAME}-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rpc_client)
    
install(TARGETS rpc-server
    EXPORT ${PROJECT_NAME}-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rpc_server)
    
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rpc-lite-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/rpc-lite-config-version.cmake DESTINATION lib/cmake/rpc-lite)
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE RpcLite:: DESTINATION lib/cmake/wb_mon)

add_library(RpcLite::rpc-client ALIAS rpc-client)
add_library(RpcLite::rpc-client_static ALIAS rpc-client_static)
add_library(RpcLite::rpc-server ALIAS rpc-server)
add_library(RpcLite::rpc-server_static ALIAS rpc-server_static
)