#Create object library, its needed for Server executable and ServerTest project
add_library(ServerLib
    OBJECT
    Server.cpp
    Marketplace.cpp
    CompletedTradeRequest.cpp
    DataBase.cpp
    Session.cpp
)

add_executable(${ServerName}
    main.cpp
)

include(FetchContent) 

FetchContent_Declare(
    libpqxx 
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git 
    GIT_TAG 7.7.3
) 

FetchContent_GetProperties(libpqxx) 

set(SKIP_BUILD_TEST On) # Skip building tests for libpqxx

if (NOT libpqxx_POPULATE) 
    FetchContent_Populate(libpqxx) 
    add_subdirectory(${libpqxx_SOURCE_DIR} ${libpqxx_BINARY_DIR}) 
endif()

target_link_libraries(ServerLib pqxx)

target_link_libraries(${ServerName} ${GlobalProjectLibs} ServerLib)