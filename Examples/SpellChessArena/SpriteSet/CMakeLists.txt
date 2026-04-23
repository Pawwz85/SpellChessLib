cmake_minimum_required(VERSION 3.16) 
project( SpriteSet 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( sources 
        "SpriteSet.cpp"
        "SpriteSetScheme.cpp"
)


qt_add_library(SpriteSet STATIC ${sources})

target_link_libraries(SpriteSet PRIVATE Qt6::Gui)
target_include_directories( SpriteSet INTERFACE include )
target_include_directories( SpriteSet PRIVATE include/SpriteSet )

