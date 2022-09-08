add_executable(${ClientName}
    client.cpp
    main.cpp
)

find_package(OpenSSL 1.1.0 REQUIRED)

target_link_libraries(${ClientName} ${GlobalProjectLibs} OpenSSL::SSL)