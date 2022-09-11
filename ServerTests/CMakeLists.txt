add_executable(${ServerTestName} 
    ClientInfoTest.cpp
    CompletedTradeRequestTest.cpp 
    DataBaseTest.cpp 
    MarketplaceTest.cpp
    SessionTest.cpp 
    TradeRequestTest.cpp
    Source.cpp
)

add_dependencies(${ServerTestName} ${ServerName})

#Getting google test sources
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

target_include_directories(${ServerTestName}  
    PRIVATE 
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(${ServerTestName} 
    PRIVATE 
    ServerLib
    ${GlobalProjectLibs}
    GTest::gtest
)

gtest_discover_tests(${ServerTestName})