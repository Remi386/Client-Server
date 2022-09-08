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

target_link_libraries(${ServerName} ${GlobalProjectLibs} ServerLib)