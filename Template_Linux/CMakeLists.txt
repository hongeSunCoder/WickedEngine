
if (NOT WIN32)
    find_package(Threads REQUIRED)
endif ()

set (SOURCE_FILES
    Example_DiscreteData.cpp
    Example_DiscreteData.h
    stdafx.h
    main.cpp
          
    Discrete/discrete.h
    Discrete/discrete.cpp
    Discrete/discrete_internal.h
    ImGui/imconfig.h
	ImGui/imgui.cpp
	ImGui/imgui.h
	ImGui/imgui_demo.cpp
	ImGui/imgui_draw.cpp
	ImGui/imgui_internal.h
	ImGui/imgui_tables.cpp
	ImGui/imgui_widgets.cpp
	ImGui/imstb_rectpack.h
	ImGui/imstb_textedit.h
	ImGui/imstb_truetype.h
	ImGui/imgui_impl_sdl.cpp
	ImGui/imgui_impl_sdl.h
)



add_executable(Template_Linux ${SOURCE_FILES})

target_link_libraries(Template_Linux PUBLIC
    WickedEngine
    Threads::Threads
)

# Copy shaders to build and source folders just to be safe:
add_custom_command(
    TARGET Template_Linux POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiPS.hlsl ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiVS.hlsl ${CMAKE_CURRENT_BINARY_DIR}
)

set(LIB_DXCOMPILER "libdxcompiler.so")

# Copy content to build folder:
add_custom_command(
	TARGET Template_Linux POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WICKED_ROOT_DIR}/WickedEngine/${LIB_DXCOMPILER} ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${WICKED_ROOT_DIR}/Content ${CMAKE_CURRENT_BINARY_DIR}/../Content
)
