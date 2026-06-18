cmake_minimum_required(VERSION 3.20)

project(ChameleonChams LANGUAGES CXX)

add_library(ChameleonChams SHARED
    src/chameleon_chams.cpp
    src/sdk_minimal.cpp
    ../CppSDK/SDK/CoreUObject_functions.cpp
)

target_compile_features(ChameleonChams PRIVATE cxx_std_20)

target_include_directories(ChameleonChams PRIVATE
    ../CppSDK
    ../CppSDK/SDK
)

target_compile_definitions(ChameleonChams PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(ChameleonChams PRIVATE
        /permissive-
        /EHsc
        /bigobj
    )
endif()

# Link against DirectX 11 libraries so builds using DX11 succeed.
if(WIN32)
    target_compile_definitions(ChameleonChams PRIVATE CHAMELEON_USE_DX11=1)
    target_link_libraries(ChameleonChams PRIVATE d3d11 dxgi)
endif()

# Minimal chams-only DLL target (always-on glow-through-walls)
add_library(ChamsOnly SHARED
    src/chams_only.cpp
    src/sdk_minimal.cpp
    ../CppSDK/SDK/CoreUObject_functions.cpp
)

target_compile_features(ChamsOnly PRIVATE cxx_std_20)

target_include_directories(ChamsOnly PRIVATE
    ../CppSDK
    ../CppSDK/SDK
)

target_compile_definitions(ChamsOnly PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(ChamsOnly PRIVATE
        /permissive-
        /EHsc
        /bigobj
    )
endif()

if(WIN32)
    target_compile_definitions(ChamsOnly PRIVATE CHAMELEON_USE_DX11=1)
    target_link_libraries(ChamsOnly PRIVATE d3d11 dxgi)
endif()
