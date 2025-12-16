
cmake_minimum_required(VERSION 3.6)

set(APP_NAME Mygame)

project(${APP_NAME})

# Enable C++17 for ECS framework features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(XCODE)
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET)
        SET (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0)
    endif()
endif()

if(NOT DEFINED BUILD_ENGINE_DONE) # to test Mygame into root project
    set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)
    set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)

    include(CocosBuildSet)
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
endif()

# record sources, headers, resources...
set(GAME_SOURCE)
set(GAME_HEADER)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    )
if(APPLE OR WIN32)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# add cross-platforms source files and header files 
list(APPEND GAME_SOURCE
     Classes/AppDelegate.cpp
     Classes/SplashScene.cpp
     Classes/MainMenuScene.cpp
     Classes/ItemsTestScene.cpp
     Classes/PlayerTestScene.cpp
     Classes/items/ItemManager.cpp
     Classes/items/Inventory.cpp
     Classes/items/InventoryLayer.cpp
     Classes/items/RecipeManager.cpp
     Classes/items/StationDetector.cpp
     Classes/items/CraftingMatcher.cpp
     Classes/items/CraftingExecutor.cpp
     Classes/items/CraftBar.cpp
     Classes/player/PlayerInput.cpp
     Classes/player/PlayerFactory.cpp
     Classes/NPC/BlueSlime.cpp
     Classes/NPC/Enemy.cpp
     Classes/NPC/GameScene.cpp
     Classes/NPC/GreenSlime.cpp
     Classes/NPC/RedSlime.cpp
     Classes/NPC/SlimeEnemy.cpp
     Classes/NPC/YellowSlime.cpp
     )
list(APPEND GAME_HEADER
     Classes/AppDelegate.h
     Classes/SplashScene.h
     Classes/MainMenuScene.h
     Classes/ItemsTestScene.h
     Classes/PlayerTestScene.h
     Classes/items/InventoryDef.h
     Classes/items/ItemManager.h
     Classes/items/Inventory.h
     Classes/items/InventoryLayer.h
     Classes/items/CraftingDef.h
     Classes/items/RecipeManager.h
     Classes/items/StationDetector.h
     Classes/items/CraftingMatcher.h
     Classes/items/CraftingExecutor.h
     Classes/items/CraftBar.h
     Classes/player/PlayerComponents.h
     Classes/player/PlayerInput.h
     Classes/player/PlayerFactory.h
     Classes/NPC/BlueSlime.h
     Classes/NPC/Enemy.h
     Classes/NPC/GameScene.h
     Classes/NPC/GreenSlime.h
     Classes/NPC/RedSlime.h
     Classes/NPC/SlimeEnemy.h
     Classes/NPC/YellowSlime.h
     )

if(ANDROID)
    # change APP_NAME to the share library name for Android, it's value depend on AndroidManifest.xml
    set(APP_NAME MyGame)
    list(APPEND GAME_SOURCE
         proj.android/app/jni/hellocpp/main.cpp
         )
elseif(LINUX)
    list(APPEND GAME_SOURCE
         proj.linux/main.cpp
         )
elseif(WIN32)
    list(APPEND GAME_HEADER
         proj.win32/main.h
         proj.win32/resource.h
         )
    list(APPEND GAME_SOURCE
         proj.win32/main.cpp
         proj.win32/game.rc
         ${common_res_files}
         )
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
             proj.ios_mac/ios/AppController.h
             proj.ios_mac/ios/RootViewController.h
             )
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/ios/main.m
             proj.ios_mac/ios/AppController.mm
             proj.ios_mac/ios/RootViewController.mm
             proj.ios_mac/ios/Prefix.pch
             ${APP_UI_RES}
             )
    elseif(MACOSX)
        set(APP_UI_RES
            proj.ios_mac/mac/Icon.icns
            proj.ios_mac/mac/Info.plist
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/mac/main.cpp
             proj.ios_mac/mac/Prefix.pch
             ${APP_UI_RES}
             )
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

# mark app complie info and libs info
set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
    )
if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

target_link_libraries(${APP_NAME} cocos2d)

# Set C++17 standard for the game target (required for ECS framework)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(${APP_NAME}
        PRIVATE Classes
        PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/entt-main/entt-main/single_include
)

# mark app resources
setup_cocos_app_config(${APP_NAME})
if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")

    if(MACOSX)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist")
    elseif(IOS)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/Info.plist")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
    endif()

    # For code-signing, set the DEVELOPMENT_TEAM:
    #set_xcode_property(${APP_NAME} DEVELOPMENT_TEAM "GRLXXXX2K9")
elseif(WIN32)
    cocos_copy_target_dll(${APP_NAME})
endif()

if(LINUX OR WIN32)
    cocos_get_resource_path(APP_RES_DIR ${APP_NAME})
    cocos_copy_target_res(${APP_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()
