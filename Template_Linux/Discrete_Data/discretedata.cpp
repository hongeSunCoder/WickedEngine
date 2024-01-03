//-------------------------------------------------------------------------
// [SECTION] INCLUDES
//-------------------------------------------------------------------------

#include "discretedata.h"
#include "discretedata_internal.h"

namespace DiscreteData
{
    // Error Checking and Debug Tools
    static void ErrorCheckNewFrameSanityChecks();
    static void ErrorCheckEndFrameSanityChecks();

    // Misc
    // static void             UpdateSettings();
    // static bool             UpdateWindowManualResize(ImGuiWindow* window, const ImVec2& size_auto_fit, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect);
    // static void             RenderWindowOuterBorders(ImGuiWindow* window);
    // static void             RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size);
    // static void             RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open);
    // static void             RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col);
    static void RenderDimmedBackgrounds();
    // static ImGuiWindow*     FindBlockingModal(ImGuiWindow* window);

    // Viewports
    static void UpdateViewportsNewFrame();
}

//-----------------------------------------------------------------------------
// [SECTION] CONTEXT AND MEMORY ALLOCATORS
//-----------------------------------------------------------------------------

// DLL users:
// - Heaps and globals are not shared across DLL boundaries!
// - You will need to call SetCurrentContext() + SetAllocatorFunctions() for each static/DLL boundary you are calling from.
// - Same applies for hot-reloading mechanisms that are reliant on reloading DLL (note that many hot-reloading mechanisms work without DLL).
// - Using Dear DiscreteData via a shared library is not recommended, because of function call overhead and because we don't guarantee backward nor forward ABI compatibility.
// - Confused? In a debugger: add GDiscreteData to your watch window and notice how its value changes depending on your current location (which DLL boundary you are in).

// Current context pointer. Implicitly used by all Dear DiscreteData functions. Always assumed to be != NULL.
// - DiscreteData::CreateContext() will automatically set this pointer if it is NULL.
//   Change to a different context by calling DiscreteData::SetCurrentContext().
// - Important: Dear DiscreteData functions are not thread-safe because of this pointer.
//   If you want thread-safety to allow N threads to access N different contexts:
//   - Change this variable to use thread local storage so each thread can refer to a different context, in your imconfig.h:
//         struct DiscreteDataContext;
//         extern thread_local DiscreteDataContext* MyImGuiTLS;
//         #define GDiscreteData MyImGuiTLS
//     And then define MyImGuiTLS in one of your cpp files. Note that thread_local is a C++11 keyword, earlier C++ uses compiler-specific keyword.
//   - Future development aims to make this context pointer explicit to all calls. Also read https://github.com/ocornut/imgui/issues/586
//   - If you need a finite number of contexts, you may compile and use multiple instances of the DiscreteData code from a different namespace.
// - DLL users: read comments above.
#ifndef GDiscreteData
DiscreteDataContext *GDiscreteData = NULL;
#endif

// Memory Allocator functions. Use SetAllocatorFunctions() to change them.
// - You probably don't want to modify that mid-program, and if you use global/static e.g. ImVector<> instances you may need to keep them accessible during program destruction.
// - DLL users: read comments above.
#ifndef IMGUI_DISABLE_DEFAULT_ALLOCATORS
static void *MallocWrapper(size_t size, void *user_data)
{
    IM_UNUSED(user_data);
    return malloc(size);
}
static void FreeWrapper(void *ptr, void *user_data)
{
    IM_UNUSED(user_data);
    free(ptr);
}
#else
static void *MallocWrapper(size_t size, void *user_data)
{
    IM_UNUSED(user_data);
    IM_UNUSED(size);
    IM_ASSERT(0);
    return NULL;
}
static void FreeWrapper(void *ptr, void *user_data)
{
    IM_UNUSED(user_data);
    IM_UNUSED(ptr);
    IM_ASSERT(0);
}
#endif
static ImGuiMemAllocFunc GImAllocatorAllocFunc = MallocWrapper;
static ImGuiMemFreeFunc GImAllocatorFreeFunc = FreeWrapper;
static void *GImAllocatorUserData = NULL;

static void AddDrawListToDrawData(ImVector<DiscreteDrawList *> *out_list, DiscreteDrawList *draw_list)
{
    if (draw_list->CmdBuffer.Size == 0)
        return;
    if (draw_list->CmdBuffer.Size == 1 && draw_list->CmdBuffer[0].ElemCount == 0 && draw_list->CmdBuffer[0].UserCallback == NULL)
        return;

    // Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc.
    // May trigger for you if you are using PrimXXX functions incorrectly.
    IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
    IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
    // if (!(draw_list->Flags & ImDrawListFlags_AllowVtxOffset))
    //     IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

    // Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per ImDrawList = per window)
    // If this assert triggers because you are drawing lots of stuff manually:
    // - First, make sure you are coarse clipping yourself and not trying to draw many things outside visible bounds.
    //   Be mindful that the ImDrawList API doesn't filter vertices. Use the Metrics/Debugger window to inspect draw list contents.
    // - If you want large meshes with more than 64K vertices, you can either:
    //   (A) Handle the ImDrawCmd::VtxOffset value in your renderer backend, and set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset'.
    //       Most example backends already support this from 1.71. Pre-1.71 backends won't.
    //       Some graphics API such as GL ES 1/2 don't have a way to offset the starting vertex so it is not supported for them.
    //   (B) Or handle 32-bit indices in your renderer backend, and uncomment '#define ImDrawIdx unsigned int' line in imconfig.h.
    //       Most example backends already support this. For example, the OpenGL example code detect index size at compile-time:
    //         glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
    //       Your own engine or render API may use different parameters or function calls to specify index sizes.
    //       2 and 4 bytes indices are generally supported by most graphics API.
    // - If for some reason neither of those solutions works for you, a workaround is to call BeginChild()/EndChild() before reaching
    //   the 64K limit to split your draw commands in multiple draw lists.
    if (sizeof(ImDrawIdx) == 2)
        IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

    out_list->push_back(draw_list);
}

//-----------------------------------------------------------------------------
// [SECTION] INITIALIZATION, SHUTDOWN
//-----------------------------------------------------------------------------

// Internal state access - if you want to share Dear DiscreteData state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
DiscreteDataContext *DiscreteData::GetCurrentContext()
{
    return GDiscreteData;
}

void DiscreteData::SetCurrentContext(DiscreteDataContext *ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GDiscreteData = ctx;
#endif
}

DiscreteDataContext *DiscreteData::CreateContext(ImFontAtlas *shared_font_atlas)
{
    DiscreteDataContext *prev_ctx = GetCurrentContext();
    DiscreteDataContext *ctx = IM_NEW(DiscreteDataContext)(shared_font_atlas);
    SetCurrentContext(ctx);
    Initialize();
    if (prev_ctx != NULL)
        SetCurrentContext(prev_ctx); // Restore previous context if any, else keep new one.
    return ctx;
}
void DiscreteData::DestroyContext(DiscreteDataContext *ctx)
{
    DiscreteDataContext *prev_ctx = GetCurrentContext();
    if (ctx == NULL) //-V1051
        ctx = prev_ctx;
    SetCurrentContext(ctx);
    Shutdown();
    SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
    IM_DELETE(ctx);
}

void DiscreteData::Initialize()
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(!g.Initialized);

    // Create default viewport
    ImGuiViewportP *viewport = IM_NEW(ImGuiViewportP)();
    g.Viewports.push_back(viewport);
    // g.TempBuffer.resize(1024 * 3 + 1, 0);

#ifdef IMGUI_HAS_DOCK
#endif

    g.Initialized = true;
}

// This function is merely here to free heap allocations.
void DiscreteData::Shutdown()
{
    // The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
    DiscreteDataContext &g = *GDiscreteData;
    // if (g.IO.Fonts && g.FontAtlasOwnedByContext)
    // {
    //     g.IO.Fonts->Locked = false;
    //     IM_DELETE(g.IO.Fonts);
    // }
    // g.IO.Fonts = NULL;
    // g.DrawListSharedData.TempBuffer.clear();

    // Cleanup of other data are conditional on actually having initialized Dear ImGui.
    if (!g.Initialized)
        return;

    // Save settings (unless we haven't attempted to load them: CreateContext/DestroyContext without a call to NewFrame shouldn't save an empty file)
    if (g.SettingsLoaded && g.IO.IniFilename != NULL)
        // SaveIniSettingsToDisk(g.IO.IniFilename);

        CallContextHooks(&g, ImGuiContextHookType_Shutdown);

    // Clear everything else
    // g.Windows.clear_delete();
    // g.WindowsFocusOrder.clear();
    // g.WindowsTempSortBuffer.clear();
    // g.CurrentWindow = NULL;
    // g.CurrentWindowStack.clear();
    // g.WindowsById.Clear();
    // g.NavWindow = NULL;
    // g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
    // g.ActiveIdWindow = g.ActiveIdPreviousFrameWindow = NULL;
    // g.MovingWindow = NULL;

    // g.KeysRoutingTable.Clear();

    // g.ColorStack.clear();
    // g.StyleVarStack.clear();
    // g.FontStack.clear();
    // g.OpenPopupStack.clear();
    // g.BeginPopupStack.clear();

    // g.Viewports.clear_delete();

    // g.TabBars.Clear();
    // g.CurrentTabBarStack.clear();
    // g.ShrinkWidthBuffer.clear();

    // g.ClipperTempData.clear_destruct();

    // g.Tables.Clear();
    // g.TablesTempData.clear_destruct();
    // g.DrawChannelsTempMergeBuffer.clear();

    // g.ClipboardHandlerData.clear();
    // g.MenusIdSubmittedThisFrame.clear();
    // g.InputTextState.ClearFreeMemory();

    // g.SettingsWindows.clear();
    // g.SettingsHandlers.clear();

    //     if (g.LogFile)
    //     {
    // #ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    //         if (g.LogFile != stdout)
    // #endif
    //             ImFileClose(g.LogFile);
    //         g.LogFile = NULL;
    //     }
    //     g.LogBuffer.clear();
    //     g.DebugLogBuf.clear();
    //     g.DebugLogIndex.clear();

    g.Initialized = false;
}

void ImDrawDataBuilder::FlattenIntoSingleLayer()
{
    int n = Layers[0].Size;
    int size = n;
    for (int i = 1; i < IM_ARRAYSIZE(Layers); i++)
        size += Layers[i].Size;
    Layers[0].resize(size);
    for (int layer_n = 1; layer_n < IM_ARRAYSIZE(Layers); layer_n++)
    {
        ImVector<DiscreteDrawList *> &layer = Layers[layer_n];
        if (layer.empty())
            continue;
        memcpy(&Layers[0][n], &layer[0], layer.Size * sizeof(DiscreteDrawList *));
        n += layer.Size;
        layer.resize(0);
    }
}

static void SetupViewportDrawData(ImGuiViewportP *viewport, ImVector<DiscreteDrawList *> *draw_lists)
{
    ImGuiIO &io = DiscreteData::GetIO();
    DiscreteDrawData *draw_data = &viewport->DrawDataP;
    draw_data->Valid = true;
    draw_data->CmdLists = (draw_lists->Size > 0) ? draw_lists->Data : NULL;
    draw_data->CmdListsCount = draw_lists->Size;
    draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
    draw_data->DisplayPos = viewport->Pos;
    draw_data->DisplaySize = viewport->Size;
    draw_data->FramebufferScale = io.DisplayFramebufferScale;
    for (int n = 0; n < draw_lists->Size; n++)
    {
        DiscreteDrawList *draw_list = draw_lists->Data[n];
        draw_list->_PopUnusedDrawCmd();
        draw_data->TotalVtxCount += draw_list->VtxBuffer.Size;
        draw_data->TotalIdxCount += draw_list->IdxBuffer.Size;
    }
}

// No specific ordering/dependency support, will see as needed
ImGuiID DiscreteData::AddContextHook(DiscreteDataContext *ctx, const ImGuiContextHook *hook)
{
    DiscreteDataContext &g = *ctx;
    IM_ASSERT(hook->Callback != NULL && hook->HookId == 0 && hook->Type != ImGuiContextHookType_PendingRemoval_);
    g.Hooks.push_back(*hook);
    g.Hooks.back().HookId = ++g.HookIdNext;
    return g.HookIdNext;
}

// Deferred removal, avoiding issue with changing vector while iterating it
void DiscreteData::RemoveContextHook(DiscreteDataContext *ctx, ImGuiID hook_id)
{
    DiscreteDataContext &g = *ctx;
    IM_ASSERT(hook_id != 0);
    for (int n = 0; n < g.Hooks.Size; n++)
        if (g.Hooks[n].HookId == hook_id)
            g.Hooks[n].Type = ImGuiContextHookType_PendingRemoval_;
}

// Call context hooks (used by e.g. test engine)
// We assume a small number of hooks so all stored in same array
void DiscreteData::CallContextHooks(DiscreteDataContext *ctx, ImGuiContextHookType hook_type)
{
    printf("------start DiscreteData::CallContextHooks: %d ------\n", hook_type);
    DiscreteDataContext &g = *ctx;
    for (int n = 0; n < g.Hooks.Size; n++)
        if (g.Hooks[n].Type == hook_type)
            g.Hooks[n].Callback(&g, &g.Hooks[n]);
}

static void DiscreteData::RenderDimmedBackgrounds()
{
    DiscreteDataContext &g = *GDiscreteData;
    printf("---- RenderDimmedBackgrounds ------");
    // ImGuiWindow *modal_window = GetTopMostAndVisiblePopupModal();
    // if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
    //     return;
    // const bool dim_bg_for_modal = (modal_window != NULL);
    // const bool dim_bg_for_window_list = (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
    // if (!dim_bg_for_modal && !dim_bg_for_window_list)
    //     return;

    // if (dim_bg_for_modal)
    // {
    //     // Draw dimming behind modal or a begin stack child, whichever comes first in draw order.
    //     ImGuiWindow *dim_behind_window = FindBottomMostVisibleWindowWithinBeginStack(modal_window);
    //     RenderDimmedBackgroundBehindWindow(dim_behind_window, GetColorU32(ImGuiCol_ModalWindowDimBg, g.DimBgRatio));
    // }
    // else if (dim_bg_for_window_list)
    // {
    // Draw dimming behind CTRL+Tab target window
    // RenderDimmedBackgroundBehindWindow(g.NavWindowingTargetAnim, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));

    // Draw border around CTRL+Tab target window
    // ImGuiWindow *window = g.NavWindowingTargetAnim;
    // ImGuiViewport *viewport = GetMainViewport();
    // float distance = g.FontSize;
    // ImRect bb = window->Rect();
    // bb.Expand(distance);
    // if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
    //     bb.Expand(-distance - 1.0f); // If a window fits the entire viewport, adjust its highlight inward
    // if (window->DrawList->CmdBuffer.Size == 0)
    //     window->DrawList->AddDrawCmd();
    // window->DrawList->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size);
    // window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), window->WindowRounding, 0, 3.0f);
    // window->DrawList->PopClipRect();
    // }
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void DiscreteData::EndFrame()
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(g.Initialized);

    // Don't process EndFrame() multiple times.
    if (g.FrameCountEnded == g.FrameCount)
        return;
    // IM_ASSERT(g.WithinFrameScope && "Forgot to call ImGui::NewFrame()?");

    // CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

    // ErrorCheckEndFrameSanityChecks();

    // // Notify Platform/OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
    // if (g.IO.SetPlatformImeDataFn && memcmp(&g.PlatformImeData, &g.PlatformImeDataPrev, sizeof(ImGuiPlatformImeData)) != 0)
    //     g.IO.SetPlatformImeDataFn(GetMainViewport(), &g.PlatformImeData);

    // // Hide implicit/fallback "Debug" window if it hasn't been used
    // g.WithinFrameScopeWithImplicitWindow = false;
    if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
        g.CurrentWindow->Active = false;
    End();

    // Update navigation: CTRL+Tab, wrap-around requests
    // NavEndFrame();

    // Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
    // if (g.DragDropActive)
    // {
    //     bool is_delivered = g.DragDropPayload.Delivery;
    //     bool is_elapsed = (g.DragDropPayload.DataFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & ImGuiDragDropFlags_SourceAutoExpirePayload) || !IsMouseDown(g.DragDropMouseButton));
    //     if (is_delivered || is_elapsed)
    //         ClearDragDrop();
    // }

    // Drag and Drop: Fallback for source tooltip. This is not ideal but better than nothing.
    // if (g.DragDropActive && g.DragDropSourceFrameCount < g.FrameCount && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    // {
    //     g.DragDropWithinSource = true;
    //     SetTooltip("...");
    //     g.DragDropWithinSource = false;
    // }

    // End frame
    // g.WithinFrameScope = false;
    g.FrameCountEnded = g.FrameCount;

    // Initiate moving window + handle left-click and right-click focus
    // UpdateMouseMovingWindowEndFrame();

    // Sort the window list so that all child windows are after their parent
    // We cannot do that on FocusWindow() because children may not exist yet
    // g.WindowsTempSortBuffer.resize(0);
    // g.WindowsTempSortBuffer.reserve(g.Windows.Size);
    // for (int i = 0; i != g.Windows.Size; i++)
    // {
    //     ImGuiWindow *window = g.Windows[i];
    //     if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow)) // if a child is active its parent will add it
    //         continue;
    //     AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
    // }

    // This usually assert if there is a mismatch between the ImGuiWindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in parents, aka we've done something wrong.
    IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
    g.Windows.swap(g.WindowsTempSortBuffer);
    g.IO.MetricsActiveWindows = g.WindowsActiveCount;

    // Unlock font atlas
    g.IO.Fonts->Locked = false;

    // Clear Input data for next frame
    g.IO.AppFocusLost = false;
    g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
    g.IO.InputQueueCharacters.resize(0);

    CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

void DiscreteData::Render()
{
    printf("-------start DiscreteData::Render -----------\n");
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    const bool first_render_of_frame = (g.FrameCountRendered != g.FrameCount);
    g.FrameCountRendered = g.FrameCount;
    g.IO.MetricsRenderWindows = 0;

    CallContextHooks(&g, ImGuiContextHookType_RenderPre);

    // Add background ImDrawList (for each active viewport)
    for (int n = 0; n != g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataBuilder.Clear();
        if (viewport->DrawLists[0] != NULL)
            AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
    }

    // Draw modal/window whitening backgrounds
    if (first_render_of_frame)
        RenderDimmedBackgrounds();

    // Add ImDrawList to render
    // ImGuiWindow *windows_to_render_top_most[2];
    // windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindow : NULL;
    // windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
    // for (int n = 0; n != g.Windows.Size; n++)
    // {
    //     ImGuiWindow *window = g.Windows[n];
    //     IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
    //     if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
    //         AddRootWindowToDrawData(window);
    // }
    // for (int n = 0; n < IM_ARRAYSIZE(windows_to_render_top_most); n++)
    //     if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily displayed as the top-most window
    //         AddRootWindowToDrawData(windows_to_render_top_most[n]);

    // Draw software mouse cursor if requested by io.MouseDrawCursor flag
    // if (g.IO.MouseDrawCursor && first_render_of_frame && g.MouseCursor != ImGuiMouseCursor_None)
    //     RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

    // Setup ImDrawData structures for end-user
    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataBuilder.FlattenIntoSingleLayer();

        // Add foreground ImDrawList (for each active viewport)
        if (viewport->DrawLists[1] != NULL)
            AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

        SetupViewportDrawData(viewport, &viewport->DrawDataBuilder.Layers[0]);
        DiscreteDrawData *draw_data = &viewport->DrawDataP;
        g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
        g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
    }

    CallContextHooks(&g, ImGuiContextHookType_RenderPost);

    printf("-------end DiscreteData::Render -----------\n");
}

// IM_ALLOC() == DiscreteData::MemAlloc()
void *DiscreteData::MemAlloc(size_t size)
{
    if (DiscreteDataContext *ctx = GDiscreteData)
        ctx->IO.MetricsActiveAllocations++;
    return (*GImAllocatorAllocFunc)(size, GImAllocatorUserData);
}

// IM_FREE() == DiscreteData::MemFree()
void DiscreteData::MemFree(void *ptr)
{
    if (ptr)
        if (DiscreteDataContext *ctx = GDiscreteData)
            ctx->IO.MetricsActiveAllocations--;
    return (*GImAllocatorFreeFunc)(ptr, GImAllocatorUserData);
}

ImGuiIO &DiscreteData::GetIO()
{
    IM_ASSERT(GDiscreteData != NULL && "No current context. Did you call DiscreteData::CreateContext() and DiscreteData::SetCurrentContext() ?");
    return GDiscreteData->IO;
}

void DiscreteData::NewFrame()
{
    printf("------start DiscreteData::NewFrame -----------\n");
    IM_ASSERT(GDiscreteData != NULL && "No current context. Did you call DiscreteData::CreateContext() and DiscreteData::SetCurrentContext() ?");
    DiscreteDataContext &g = *GDiscreteData;

    // Remove pending delete hooks before frame start.
    // This deferred removal avoid issues of removal while iterating the hook vector
    for (int n = g.Hooks.Size - 1; n >= 0; n--)
        if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
            g.Hooks.erase(&g.Hooks[n]);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

    // Check and assert for various common IO and Configuration mistakes
    ErrorCheckNewFrameSanityChecks();

    // Load settings on first frame, save settings when modified (after a delay)
    // UpdateSettings();

    g.Time += g.IO.DeltaTime;
    // g.WithinFrameScope = true;
    g.FrameCount += 1;
    // g.TooltipOverrideCount = 0;
    // g.WindowsActiveCount = 0;
    // g.MenusIdSubmittedThisFrame.resize(0);

    // Calculate frame-rate for the user, as a purely luxurious feature
    // g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    // g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    // g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_ARRAYSIZE(g.FramerateSecPerFrame);
    // g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_ARRAYSIZE(g.FramerateSecPerFrame));
    // g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

    UpdateViewportsNewFrame();

    // Setup current font and draw list shared data
    g.IO.Fonts->Locked = true;
    // SetCurrentFont(GetDefaultFont());
    // IM_ASSERT(g.Font->IsLoaded());
    ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int n = 0; n < g.Viewports.Size; n++)
        virtual_space.Add(g.Viewports[n]->GetMainRect());
    // g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
    // g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
    // g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
    // g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
    // if (g.Style.AntiAliasedLines)
    //     g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
    // if (g.Style.AntiAliasedLinesUseTex && !(g.Font->ContainerAtlas->Flags & ImFontAtlasFlags_NoBakedLines))
    //     g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
    // if (g.Style.AntiAliasedFill)
    //     g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;

    // Mark rendering data as invalid to prevent user who may have a handle on it to use it.
    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataP.Clear();
    }

    // Drag and drop keep the source ID alive so even if the source disappear our state is consistent
    // if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
    //     KeepAliveID(g.DragDropPayload.SourceId);

    // // Update HoveredId data
    // if (!g.HoveredIdPreviousFrame)
    //     g.HoveredIdTimer = 0.0f;
    // if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
    //     g.HoveredIdNotActiveTimer = 0.0f;
    // if (g.HoveredId)
    //     g.HoveredIdTimer += g.IO.DeltaTime;
    // if (g.HoveredId && g.ActiveId != g.HoveredId)
    //     g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
    // g.HoveredIdPreviousFrame = g.HoveredId;
    // g.HoveredId = 0;
    // g.HoveredIdAllowOverlap = false;
    // g.HoveredIdDisabled = false;

    // Clear ActiveID if the item is not alive anymore.
    // In 1.87, the common most call to KeepAliveID() was moved from GetID() to ItemAdd().
    // As a result, custom widget using ButtonBehavior() _without_ ItemAdd() need to call KeepAliveID() themselves.
    // if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId)
    // {
    //     IMGUI_DEBUG_LOG_ACTIVEID("NewFrame(): ClearActiveID() because it isn't marked alive anymore!\n");
    //     ClearActiveID();
    // }

    // Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
    // if (g.ActiveId)
    //     g.ActiveIdTimer += g.IO.DeltaTime;
    // g.LastActiveIdTimer += g.IO.DeltaTime;
    // g.ActiveIdPreviousFrame = g.ActiveId;
    // g.ActiveIdPreviousFrameWindow = g.ActiveIdWindow;
    // g.ActiveIdPreviousFrameHasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
    // g.ActiveIdIsAlive = 0;
    // g.ActiveIdHasBeenEditedThisFrame = false;
    // g.ActiveIdPreviousFrameIsAlive = false;
    // g.ActiveIdIsJustActivated = false;
    // if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
    //     g.TempInputId = 0;
    //     if (g.ActiveId == 0)
    //     {
    //         g.ActiveIdUsingNavDirMask = 0x00;
    //         g.ActiveIdUsingAllKeyboardKeys = false;
    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //         g.ActiveIdUsingNavInputMask = 0x00;
    // #endif
    //     }

    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //     if (g.ActiveId == 0)
    //         g.ActiveIdUsingNavInputMask = 0;
    //     else if (g.ActiveIdUsingNavInputMask != 0)
    //     {
    //         // If your custom widget code used:                 { g.ActiveIdUsingNavInputMask |= (1 << ImGuiNavInput_Cancel); }
    //         // Since IMGUI_VERSION_NUM >= 18804 it should be:   { SetKeyOwner(ImGuiKey_Escape, g.ActiveId); SetKeyOwner(ImGuiKey_NavGamepadCancel, g.ActiveId); }
    //         if (g.ActiveIdUsingNavInputMask & (1 << ImGuiNavInput_Cancel))
    //             SetKeyOwner(ImGuiKey_Escape, g.ActiveId);
    //         if (g.ActiveIdUsingNavInputMask & ~(1 << ImGuiNavInput_Cancel))
    //             IM_ASSERT(0); // Other values unsupported
    //     }
    // #endif

    // Update hover delay for IsItemHovered() with delays and tooltips
    // g.HoverDelayIdPreviousFrame = g.HoverDelayId;
    // if (g.HoverDelayId != 0)
    // {
    //     // if (g.IO.MouseDelta.x == 0.0f && g.IO.MouseDelta.y == 0.0f) // Need design/flags
    //     g.HoverDelayTimer += g.IO.DeltaTime;
    //     g.HoverDelayClearTimer = 0.0f;
    //     g.HoverDelayId = 0;
    // }
    // else if (g.HoverDelayTimer > 0.0f)
    // {
    //     // This gives a little bit of leeway before clearing the hover timer, allowing mouse to cross gaps
    //     g.HoverDelayClearTimer += g.IO.DeltaTime;
    //     if (g.HoverDelayClearTimer >= ImMax(0.20f, g.IO.DeltaTime * 2.0f)) // ~6 frames at 30 Hz + allow for low framerate
    //         g.HoverDelayTimer = g.HoverDelayClearTimer = 0.0f;             // May want a decaying timer, in which case need to clamp at max first, based on max of caller last requested timer.
    // }

    // Drag and drop
    // g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
    // g.DragDropAcceptIdCurr = 0;
    // g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    // g.DragDropWithinSource = false;
    // g.DragDropWithinTarget = false;
    // g.DragDropHoldJustPressedId = 0;

    // Close popups on focus lost (currently wip/opt-in)
    // if (g.IO.AppFocusLost)
    //    ClosePopupsExceptModals();

    // Process input queue (trickle as many events as possible)
    // g.InputEventsTrail.resize(0);
    // UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

    // Update keyboard input state
    // UpdateKeyboardInputs();

    // IM_ASSERT(g.IO.KeyCtrl == IsKeyDown(ImGuiKey_LeftCtrl) || IsKeyDown(ImGuiKey_RightCtrl));
    // IM_ASSERT(g.IO.KeyShift == IsKeyDown(ImGuiKey_LeftShift) || IsKeyDown(ImGuiKey_RightShift));
    // IM_ASSERT(g.IO.KeyAlt == IsKeyDown(ImGuiKey_LeftAlt) || IsKeyDown(ImGuiKey_RightAlt));
    // IM_ASSERT(g.IO.KeySuper == IsKeyDown(ImGuiKey_LeftSuper) || IsKeyDown(ImGuiKey_RightSuper));

    // Update gamepad/keyboard navigation
    // NavUpdate();

    // // Update mouse input state
    // UpdateMouseInputs();

    // // Find hovered window
    // // (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse release frame)
    // UpdateHoveredWindowAndCaptureFlags();

    // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
    // UpdateMouseMovingWindowNewFrame();

    // Background darkening/whitening
    // if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
    //     g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
    // else
    //     g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

    // g.MouseCursor = ImGuiMouseCursor_Arrow;
    // g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;

    // // Platform IME data: reset for the frame
    // g.PlatformImeDataPrev = g.PlatformImeData;
    // g.PlatformImeData.WantVisible = false;

    // // Mouse wheel scrolling, scale
    // UpdateMouseWheel();

    // Mark all windows as not visible and compact unused memory.
    // IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
    // const float memory_compact_start_time = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f) ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    // for (int i = 0; i != g.Windows.Size; i++)
    // {
    //     ImGuiWindow *window = g.Windows[i];
    //     window->WasActive = window->Active;
    //     window->Active = false;
    //     window->WriteAccessed = false;
    //     window->BeginCountPreviousFrame = window->BeginCount;
    //     window->BeginCount = 0;

    //     // Garbage collect transient buffers of recently unused windows
    //     if (!window->WasActive && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
    //         GcCompactTransientWindowBuffers(window);
    // }

    // Garbage collect transient buffers of recently unused tables
    // for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
    //     if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
    //         TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
    // for (int i = 0; i < g.TablesTempData.Size; i++)
    //     if (g.TablesTempData[i].LastTimeActive >= 0.0f && g.TablesTempData[i].LastTimeActive < memory_compact_start_time)
    //         TableGcCompactTransientBuffers(&g.TablesTempData[i]);
    // if (g.GcCompactAll)
    //     GcCompactTransientMiscBuffers();
    // g.GcCompactAll = false;

    // // Closing the focused window restore focus to the first active root window in descending z-order
    // if (g.NavWindow && !g.NavWindow->WasActive)
    //     FocusTopMostWindowUnderOne(NULL, NULL);

    // No window should be open at the beginning of the frame.
    // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
    // g.CurrentWindowStack.resize(0);
    // g.BeginPopupStack.resize(0);
    // g.ItemFlagsStack.resize(0);
    // g.ItemFlagsStack.push_back(ImGuiItemFlags_None);
    // g.GroupStack.resize(0);

    // [DEBUG] Update debug features
    // UpdateDebugToolItemPicker();
    // UpdateDebugToolStackQueries();
    // if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
    //     g.DebugLocateId = 0;

    // Create implicit/fallback window - which we will only render it if the user has added something to it.
    // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
    // This fallback is particularly important as it prevents DiscreteData:: calls from crashing.
    // g.WithinFrameScopeWithImplicitWindow = true;
    // SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    Begin("Debug##Default");
    IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePost);

    printf("------end DiscreteData::NewFrame -----------\n");
}

// Pass this to your backend rendering function! Valid after Render() and until the next call to NewFrame()
DiscreteDrawData *DiscreteData::GetDrawData()
{
    DiscreteDataContext &g = *GDiscreteData;
    ImGuiViewportP *viewport = g.Viewports[0];
    return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

static DiscreteDrawList *GetViewportDrawList(ImGuiViewportP *viewport, size_t drawlist_no, const char *drawlist_name)
{
    // Create the draw list on demand, because they are not frequently used for all viewports
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(drawlist_no < IM_ARRAYSIZE(viewport->DrawLists));
    DiscreteDrawList *draw_list = viewport->DrawLists[drawlist_no];
    if (draw_list == NULL)
    {
        draw_list = IM_NEW(DiscreteDrawList)(&g.DrawListSharedData);
        draw_list->_OwnerName = drawlist_name;
        viewport->DrawLists[drawlist_no] = draw_list;
    }

    // Our ImDrawList system requires that there is always a command
    if (viewport->DrawListsLastFrame[drawlist_no] != g.FrameCount)
    {
        draw_list->_ResetForNewFrame();
        // draw_list->PushTextureID(g.IO.Fonts->TexID);
        // draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size, false);
        viewport->DrawListsLastFrame[drawlist_no] = g.FrameCount;
    }
    return draw_list;
}

DiscreteDrawList *DiscreteData::GetBackgroundDrawList(ImGuiViewport *viewport)
{
    return GetViewportDrawList((ImGuiViewportP *)viewport, 0, "##Background");
}

DiscreteDrawList *DiscreteData::GetBackgroundDrawList()
{
    DiscreteDataContext &g = *GDiscreteData;
    return GetBackgroundDrawList(g.Viewports[0]);
}

DiscreteDrawList *DiscreteData::GetForegroundDrawList(ImGuiViewport *viewport)
{
    return GetViewportDrawList((ImGuiViewportP *)viewport, 1, "##Foreground");
}

DiscreteDrawList *DiscreteData::GetForegroundDrawList()
{
    DiscreteDataContext &g = *GDiscreteData;
    return GetForegroundDrawList(g.Viewports[0]);
}
double DiscreteData::GetTime()
{
    return GDiscreteData->Time;
}

int DiscreteData::GetFrameCount()
{
    return GDiscreteData->FrameCount;
}

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool DiscreteData::Begin(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    // DiscreteDataContext &g = *GDiscreteData;
    // const ImGuiStyle &style = g.Style;
    // IM_ASSERT(name != NULL && name[0] != '\0');   // Window name required
    // IM_ASSERT(g.WithinFrameScope);                // Forgot to call ImGui::NewFrame()
    // IM_ASSERT(g.FrameCountEnded != g.FrameCount); // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

    // // Find or create
    // ImGuiWindow *window = FindWindowByName(name);
    // const bool window_just_created = (window == NULL);
    // if (window_just_created)
    //     window = CreateNewWindow(name, flags);

    // // Automatically disable manual moving/resizing when NoInputs is set
    // if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
    //     flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    // if (flags & ImGuiWindowFlags_NavFlattened)
    //     IM_ASSERT(flags & ImGuiWindowFlags_ChildWindow);

    // const int current_frame = g.FrameCount;
    // const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    // window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

    // // Update the Appearing flag
    // bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1); // Not using !WasActive because the implicit "Debug" window would always toggle off->on
    // if (flags & ImGuiWindowFlags_Popup)
    // {
    //     ImGuiPopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
    //     window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
    //     window_just_activated_by_user |= (window != popup_ref.Window);
    // }
    // window->Appearing = window_just_activated_by_user;
    // if (window->Appearing)
    //     SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);

    // // Update Flags, LastFrameActive, BeginOrderXXX fields
    // if (first_begin_of_the_frame)
    // {
    //     UpdateWindowInFocusOrderList(window, window_just_created, flags);
    //     window->Flags = (ImGuiWindowFlags)flags;
    //     window->LastFrameActive = current_frame;
    //     window->LastTimeActive = (float)g.Time;
    //     window->BeginOrderWithinParent = 0;
    //     window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
    // }
    // else
    // {
    //     flags = window->Flags;
    // }

    // // Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
    // ImGuiWindow *parent_window_in_stack = g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back().Window;
    // ImGuiWindow *parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)) ? parent_window_in_stack : NULL) : window->ParentWindow;
    // IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    // // We allow window memory to be compacted so recreate the base stack when needed.
    // if (window->IDStack.Size == 0)
    //     window->IDStack.push_back(window->ID);

    // // Add to stack
    // // We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call SetCurrentWindow()
    // g.CurrentWindow = window;
    // ImGuiWindowStackData window_stack_data;
    // window_stack_data.Window = window;
    // window_stack_data.ParentLastItemDataBackup = g.LastItemData;
    // window_stack_data.StackSizesOnBegin.SetToCurrentState();
    // g.CurrentWindowStack.push_back(window_stack_data);
    // if (flags & ImGuiWindowFlags_ChildMenu)
    //     g.BeginMenuCount++;

    // // Update ->RootWindow and others pointers (before any possible call to FocusWindow)
    // if (first_begin_of_the_frame)
    // {
    //     UpdateWindowParentAndRootLinks(window, flags, parent_window);
    //     window->ParentWindowInBeginStack = parent_window_in_stack;
    // }

    // // Add to focus scope stack
    // PushFocusScope(window->ID);
    // window->NavRootFocusScopeId = g.CurrentFocusScopeId;
    // g.CurrentWindow = NULL;

    // // Add to popup stack
    // if (flags & ImGuiWindowFlags_Popup)
    // {
    //     ImGuiPopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
    //     popup_ref.Window = window;
    //     popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
    //     g.BeginPopupStack.push_back(popup_ref);
    //     window->PopupId = popup_ref.PopupId;
    // }

    // // Process SetNextWindow***() calls
    // // (FIXME: Consider splitting the HasXXX flags into X/Y components
    // bool window_pos_set_by_api = false;
    // bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasPos)
    // {
    //     window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
    //     if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
    //     {
    //         // May be processed on the next frame if this is our first frame and we are measuring size
    //         // FIXME: Look into removing the branch so everything can go through this same code path for consistency.
    //         window->SetWindowPosVal = g.NextWindowData.PosVal;
    //         window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
    //         window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
    //     }
    //     else
    //     {
    //         SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
    //     }
    // }
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize)
    // {
    //     window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
    //     window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
    //     SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
    // }
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasScroll)
    // {
    //     if (g.NextWindowData.ScrollVal.x >= 0.0f)
    //     {
    //         window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
    //         window->ScrollTargetCenterRatio.x = 0.0f;
    //     }
    //     if (g.NextWindowData.ScrollVal.y >= 0.0f)
    //     {
    //         window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
    //         window->ScrollTargetCenterRatio.y = 0.0f;
    //     }
    // }
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasContentSize)
    //     window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
    // else if (first_begin_of_the_frame)
    //     window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasCollapsed)
    //     SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
    // if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasFocus)
    //     FocusWindow(window);
    // if (window->Appearing)
    //     SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

    // // When reusing window again multiple times a frame, just append content (don't need to setup again)
    // if (first_begin_of_the_frame)
    // {
    //     // Initialize
    //     const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
    //     const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
    //     window->Active = true;
    //     window->HasCloseButton = (p_open != NULL);
    //     window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
    //     window->IDStack.resize(1);
    //     window->DrawList->_ResetForNewFrame();
    //     window->DC.CurrentTableIdx = -1;

    //     // Restore buffer capacity when woken from a compacted state, to avoid
    //     if (window->MemoryCompacted)
    //         GcAwakeTransientWindowBuffers(window);

    //     // Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would stay unchanged).
    //     // The title bar always display the 'name' parameter, so we only update the string storage if it needs to be visible to the end-user elsewhere.
    //     bool window_title_visible_elsewhere = false;
    //     if (g.NavWindowingListWindow != NULL && (window->Flags & ImGuiWindowFlags_NoNavFocus) == 0) // Window titles visible when using CTRL+TAB
    //         window_title_visible_elsewhere = true;
    //     if (window_title_visible_elsewhere && !window_just_created && strcmp(name, window->Name) != 0)
    //     {
    //         size_t buf_len = (size_t)window->NameBufLen;
    //         window->Name = ImStrdupcpy(window->Name, &buf_len, name);
    //         window->NameBufLen = (int)buf_len;
    //     }

    //     // UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

    //     // Update contents size from last frame for auto-fitting (or use explicit size)
    //     CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);
    //     if (window->HiddenFramesCanSkipItems > 0)
    //         window->HiddenFramesCanSkipItems--;
    //     if (window->HiddenFramesCannotSkipItems > 0)
    //         window->HiddenFramesCannotSkipItems--;
    //     if (window->HiddenFramesForRenderOnly > 0)
    //         window->HiddenFramesForRenderOnly--;

    //     // Hide new windows for one frame until they calculate their size
    //     if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
    //         window->HiddenFramesCannotSkipItems = 1;

    //     // Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
    //     // We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be tempted to use the old size.
    //     if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
    //     {
    //         window->HiddenFramesCannotSkipItems = 1;
    //         if (flags & ImGuiWindowFlags_AlwaysAutoResize)
    //         {
    //             if (!window_size_x_set_by_api)
    //                 window->Size.x = window->SizeFull.x = 0.f;
    //             if (!window_size_y_set_by_api)
    //                 window->Size.y = window->SizeFull.y = 0.f;
    //             window->ContentSize = window->ContentSizeIdeal = ImVec2(0.f, 0.f);
    //         }
    //     }

    //     // SELECT VIEWPORT
    //     // FIXME-VIEWPORT: In the docking/viewport branch, this is the point where we select the current viewport (which may affect the style)

    //     ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)GetMainViewport();
    //     SetWindowViewport(window, viewport);
    //     SetCurrentWindow(window);

    //     // LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)

    //     if (flags & ImGuiWindowFlags_ChildWindow)
    //         window->WindowBorderSize = style.ChildBorderSize;
    //     else
    //         window->WindowBorderSize = ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
    //     window->WindowPadding = style.WindowPadding;
    //     if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & (ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_Popup)) && window->WindowBorderSize == 0.0f)
    //         window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

    //     // Lock menu offset so size calculation can use it as menu-bar windows need a minimum size.
    //     window->DC.MenuBarOffset.x = ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
    //     window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;

    //     bool use_current_size_for_scrollbar_x = window_just_created;
    //     bool use_current_size_for_scrollbar_y = window_just_created;

    //     // Collapse window by double-clicking on title bar
    //     // At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
    //     if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
    //     {
    //         // We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason, could be fixed), so verify that we don't have items over the title bar.
    //         ImRect title_bar_rect = window->TitleBarRect();
    //         if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseClickedCount[0] == 2)
    //             window->WantCollapseToggle = true;
    //         if (window->WantCollapseToggle)
    //         {
    //             window->Collapsed = !window->Collapsed;
    //             if (!window->Collapsed)
    //                 use_current_size_for_scrollbar_y = true;
    //             MarkIniSettingsDirty(window);
    //         }
    //     }
    //     else
    //     {
    //         window->Collapsed = false;
    //     }
    //     window->WantCollapseToggle = false;

    //     // SIZE

    //     // Outer Decoration Sizes
    //     // (we need to clear ScrollbarSize immediatly as CalcWindowAutoFitSize() needs it and can be called from other locations).
    //     const ImVec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
    //     window->DecoOuterSizeX1 = 0.0f;
    //     window->DecoOuterSizeX2 = 0.0f;
    //     window->DecoOuterSizeY1 = window->TitleBarHeight() + window->MenuBarHeight();
    //     window->DecoOuterSizeY2 = 0.0f;
    //     window->ScrollbarSizes = ImVec2(0.0f, 0.0f);

    //     // Calculate auto-fit size, handle automatic resize
    //     const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal);
    //     if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed)
    //     {
    //         // Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
    //         if (!window_size_x_set_by_api)
    //         {
    //             window->SizeFull.x = size_auto_fit.x;
    //             use_current_size_for_scrollbar_x = true;
    //         }
    //         if (!window_size_y_set_by_api)
    //         {
    //             window->SizeFull.y = size_auto_fit.y;
    //             use_current_size_for_scrollbar_y = true;
    //         }
    //     }
    //     else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
    //     {
    //         // Auto-fit may only grow window during the first few frames
    //         // We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
    //         if (!window_size_x_set_by_api && window->AutoFitFramesX > 0)
    //         {
    //             window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
    //             use_current_size_for_scrollbar_x = true;
    //         }
    //         if (!window_size_y_set_by_api && window->AutoFitFramesY > 0)
    //         {
    //             window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
    //             use_current_size_for_scrollbar_y = true;
    //         }
    //         if (!window->Collapsed)
    //             MarkIniSettingsDirty(window);
    //     }

    //     // Apply minimum/maximum window size constraints and final size
    //     window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
    //     window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

    //     // POSITION

    //     // Popup latch its initial position, will position itself when it appears next frame
    //     if (window_just_activated_by_user)
    //     {
    //         window->AutoPosLastDirection = ImGuiDir_None;
    //         if ((flags & ImGuiWindowFlags_Popup) != 0 && !(flags & ImGuiWindowFlags_Modal) && !window_pos_set_by_api) // FIXME: BeginPopup() could use SetNextWindowPos()
    //             window->Pos = g.BeginPopupStack.back().OpenPopupPos;
    //     }

    //     // Position child window
    //     if (flags & ImGuiWindowFlags_ChildWindow)
    //     {
    //         IM_ASSERT(parent_window && parent_window->Active);
    //         window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
    //         parent_window->DC.ChildWindows.push_back(window);
    //         if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
    //             window->Pos = parent_window->DC.CursorPos;
    //     }

    // const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
    // if (window_pos_with_pivot)
    //     SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot, 0); // Position given a pivot (e.g. for centering)
    // else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
    //     window->Pos = FindBestWindowPosForPopup(window);
    // else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
    //     window->Pos = FindBestWindowPosForPopup(window);
    // else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
    //     window->Pos = FindBestWindowPosForPopup(window);

    // // Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
    // // When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
    // ImRect viewport_rect(viewport->GetMainRect());
    // ImRect viewport_work_rect(viewport->GetWorkRect());
    // ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
    // ImRect visibility_rect(viewport_work_rect.Min + visibility_padding, viewport_work_rect.Max - visibility_padding);

    // // Clamp position/size so window stays visible within its viewport or monitor
    // // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
    // if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow))
    //     if (viewport_rect.GetWidth() > 0.0f && viewport_rect.GetHeight() > 0.0f)
    //         ClampWindowPos(window, visibility_rect);
    // window->Pos = ImFloor(window->Pos);

    // // Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
    // // Large values tend to lead to variety of artifacts and are not recommended.
    // window->WindowRounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildRounding : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding
    //                                                                                                                                                                 : style.WindowRounding;

    // // For windows with title bar or menu bar, we clamp to FrameHeight(FontSize + FramePadding.y * 2.0f) to completely hide artifacts.
    // // if ((window->Flags & ImGuiWindowFlags_MenuBar) || !(window->Flags & ImGuiWindowFlags_NoTitleBar))
    // //    window->WindowRounding = ImMin(window->WindowRounding, g.FontSize + style.FramePadding.y * 2.0f);

    // // Apply window focus (new and reactivated windows are moved to front)
    // bool want_focus = false;
    // if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
    // {
    //     if (flags & ImGuiWindowFlags_Popup)
    //         want_focus = true;
    //     else if ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)) == 0)
    //         want_focus = true;

    //     ImGuiWindow *modal = GetTopMostPopupModal();
    //     if (modal != NULL && !IsWindowWithinBeginStackOf(window, modal))
    //     {
    //         // Avoid focusing a window that is created outside of active modal. This will prevent active modal from being closed.
    //         // Since window is not focused it would reappear at the same display position like the last time it was visible.
    //         // In case of completely new windows it would go to the top (over current modal), but input to such window would still be blocked by modal.
    //         // Position window behind a modal that is not a begin-parent of this window.
    //         want_focus = false;
    //         if (window == window->RootWindow)
    //         {
    //             ImGuiWindow *blocking_modal = FindBlockingModal(window);
    //             IM_ASSERT(blocking_modal != NULL);
    //             BringWindowToDisplayBehind(window, blocking_modal);
    //         }
    //     }
    // }

    // [Test Engine] Register whole window in the item system
#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (g.TestEngineHookItems)
    {
        IM_ASSERT(window->IDStack.Size == 1);
        window->IDStack.Size = 0; // As window->IDStack[0] == window->ID here, make sure TestEngine doesn't erroneously see window as parent of itself.
        IMGUI_TEST_ENGINE_ITEM_ADD(window->Rect(), window->ID);
        IMGUI_TEST_ENGINE_ITEM_INFO(window->ID, window->Name, (g.HoveredWindow == window) ? ImGuiItemStatusFlags_HoveredRect : 0);
        window->IDStack.Size = 1;
    }
#endif

    // // Handle manual resize: Resize Grips, Borders, Gamepad
    // int border_held = -1;
    // ImU32 resize_grip_col[4] = {};
    // const int resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.
    // const float resize_grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    // if (!window->Collapsed)
    //     if (UpdateWindowManualResize(window, size_auto_fit, &border_held, resize_grip_count, &resize_grip_col[0], visibility_rect))
    //         use_current_size_for_scrollbar_x = use_current_size_for_scrollbar_y = true;
    // window->ResizeBorderHeld = (signed char)border_held;

    // // SCROLLBAR VISIBILITY

    // // Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized Size).
    // if (!window->Collapsed)
    // {
    //     // When reading the current size we need to read it after size constraints have been applied.
    //     // Intentionally use previous frame values for InnerRect and ScrollbarSizes.
    //     // And when we use window->DecorationUp here it doesn't have ScrollbarSizes.y applied yet.
    //     ImVec2 avail_size_from_current_frame = ImVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
    //     ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
    //     ImVec2 needed_size_from_last_frame = window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
    //     float size_x_for_scrollbars = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
    //     float size_y_for_scrollbars = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
    //     // bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX expression? How many pros vs cons?
    //     window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar));
    //     window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));
    //     if (window->ScrollbarX && !window->ScrollbarY)
    //         window->ScrollbarY = (needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar);
    //     window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);

    // Amend the partially filled window->DecorationXXX values.
    // window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
    // window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
    // }

    // UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
    // Update various regions. Variables they depend on should be set above in this function.
    // We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

    // Outer rectangle
    // Not affected by window border size. Used by:
    // - FindHoveredWindow() (w/ extra padding when border resize is enabled)
    // - Begin() initial clipping rect for drawing window background and borders.
    // - Begin() clipping whole child
    // const ImRect host_rect = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
    // const ImRect outer_rect = window->Rect();
    // const ImRect title_bar_rect = window->TitleBarRect();
    // window->OuterRectClipped = outer_rect;
    // window->OuterRectClipped.ClipWith(host_rect);

    // // Inner rectangle
    // // Not affected by window border size. Used by:
    // // - InnerClipRect
    // // - ScrollToRectEx()
    // // - NavUpdatePageUpPageDown()
    // // - Scrollbar()
    // window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
    // window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
    // window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
    // window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

    // // Inner clipping rectangle.
    // // Will extend a little bit outside the normal work region.
    // // This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
    // // Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
    // // Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which is the correct behavior.
    // // Affected by window/frame border size. Used by:
    // // - Begin() initial clip rect
    // float top_border_size = (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);
    // window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + ImMax(ImFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
    // window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size);
    // window->InnerClipRect.Max.x = ImFloor(0.5f + window->InnerRect.Max.x - ImMax(ImFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
    // window->InnerClipRect.Max.y = ImFloor(0.5f + window->InnerRect.Max.y - window->WindowBorderSize);
    // window->InnerClipRect.ClipWithFull(host_rect);

    // // Default item width. Make it proportional to window size if window manually resizes
    // if (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) && !(flags & ImGuiWindowFlags_AlwaysAutoResize))
    //     window->ItemWidthDefault = ImFloor(window->Size.x * 0.65f);
    // else
    //     window->ItemWidthDefault = ImFloor(g.FontSize * 16.0f);

    // // SCROLLING

    // // Lock down maximum scrolling
    // // The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from previous rect in order to accommodate
    // // for right/bottom aligned items without creating a scrollbar.
    // window->ScrollMax.x = ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
    // window->ScrollMax.y = ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

    // // Apply scrolling
    // window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
    // window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
    // window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

    // // DRAWING

    // // Setup draw list and outer clipping rectangle
    // IM_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0);
    // window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
    // PushClipRect(host_rect.Min, host_rect.Max, false);

    // // Child windows can render their decoration (bg color, border, scrollbars, etc.) within their parent to save a draw call (since 1.71)
    // // When using overlapping child windows, this will break the assumption that child z-order is mapped to submission order.
    // // FIXME: User code may rely on explicit sorting of overlapping child window and would need to disable this somehow. Please get in contact if you are affected (github #4493)
    // {
    //     bool render_decorations_in_parent = false;
    //     if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
    //     {
    //         // - We test overlap with the previous child window only (testing all would end up being O(log N) not a good investment here)
    //         // - We disable this when the parent window has zero vertices, which is a common pattern leading to laying out multiple overlapping childs
    //         ImGuiWindow *previous_child = parent_window->DC.ChildWindows.Size >= 2 ? parent_window->DC.ChildWindows[parent_window->DC.ChildWindows.Size - 2] : NULL;
    //         bool previous_child_overlapping = previous_child ? previous_child->Rect().Overlaps(window->Rect()) : false;
    //         bool parent_is_empty = parent_window->DrawList->VtxBuffer.Size > 0;
    //         if (window->DrawList->CmdBuffer.back().ElemCount == 0 && parent_is_empty && !previous_child_overlapping)
    //             render_decorations_in_parent = true;
    //     }
    //     if (render_decorations_in_parent)
    //         window->DrawList = parent_window->DrawList;

    //     // Handle title bar, scrollbar, resize grips and resize borders
    //     const ImGuiWindow *window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
    //     const bool title_bar_is_highlight = want_focus || (window_to_highlight && window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight);
    //     const bool handle_borders_and_resize_grips = true; // This exists to facilitate merge with 'docking' branch.
    //     RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, handle_borders_and_resize_grips, resize_grip_count, resize_grip_col, resize_grip_draw_size);

    //     if (render_decorations_in_parent)
    //         window->DrawList = &window->DrawListInst;
    // }

    // UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

    // Work rectangle.
    // Affected by window padding and border size. Used by:
    // - Columns() for right-most edge
    // - TreeNode(), CollapsingHeader() for right-most edge
    // - BeginTabBar() for right-most edge
    // const bool allow_scrollbar_x = !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
    // const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
    // const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
    // const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
    // window->WorkRect.Min.x = ImFloor(window->InnerRect.Min.x - window->Scroll.x + ImMax(window->WindowPadding.x, window->WindowBorderSize));
    // window->WorkRect.Min.y = ImFloor(window->InnerRect.Min.y - window->Scroll.y + ImMax(window->WindowPadding.y, window->WindowBorderSize));
    // window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
    // window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
    // window->ParentWorkRect = window->WorkRect;

    // // [LEGACY] Content Region
    // // FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very misleading / partly faulty, but some BeginChild() patterns relies on it.
    // // Used by:
    // // - Mouse wheel scrolling + many other things
    // window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x + window->DecoOuterSizeX1;
    // window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + window->DecoOuterSizeY1;
    // window->ContentRegionRect.Max.x = window->ContentRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
    // window->ContentRegionRect.Max.y = window->ContentRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

    // Setup drawing context
    // (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
    // window->DC.Indent.x = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
    // window->DC.GroupOffset.x = 0.0f;
    // window->DC.ColumnsOffset.x = 0.0f;

    // // Record the loss of precision of CursorStartPos which can happen due to really large scrolling amount.
    // // This is used by clipper to compensate and fix the most common use case of large scroll area. Easy and cheap, next best thing compared to switching everything to double or ImU64.
    // double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x - (double)window->Scroll.x + window->DecoOuterSizeX1 + window->DC.ColumnsOffset.x;
    // double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y - (double)window->Scroll.y + window->DecoOuterSizeY1;
    // window->DC.CursorStartPos = ImVec2((float)start_pos_highp_x, (float)start_pos_highp_y);
    // window->DC.CursorStartPosLossyness = ImVec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x), (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
    // window->DC.CursorPos = window->DC.CursorStartPos;
    // window->DC.CursorPosPrevLine = window->DC.CursorPos;
    // window->DC.CursorMaxPos = window->DC.CursorStartPos;
    // window->DC.IdealMaxPos = window->DC.CursorStartPos;
    // window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
    // window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
    // window->DC.IsSameLine = window->DC.IsSetPos = false;

    // window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    // window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
    // window->DC.NavLayersActiveMaskNext = 0x00;
    // window->DC.NavHideHighlightOneFrame = false;
    // window->DC.NavHasScroll = (window->ScrollMax.y > 0.0f);

    // window->DC.MenuBarAppending = false;
    // window->DC.MenuColumns.Update(style.ItemSpacing.x, window_just_activated_by_user);
    // window->DC.TreeDepth = 0;
    // window->DC.TreeJumpToParentOnPopMask = 0x00;
    // window->DC.ChildWindows.resize(0);
    // window->DC.StateStorage = &window->StateStorage;
    // window->DC.CurrentColumns = NULL;
    // window->DC.LayoutType = ImGuiLayoutType_Vertical;
    // window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;

    // window->DC.ItemWidth = window->ItemWidthDefault;
    // window->DC.TextWrapPos = -1.0f; // disabled
    // window->DC.ItemWidthStack.resize(0);
    // window->DC.TextWrapPosStack.resize(0);

    // if (window->AutoFitFramesX > 0)
    //     window->AutoFitFramesX--;
    // if (window->AutoFitFramesY > 0)
    //     window->AutoFitFramesY--;

    // Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
    // if (want_focus)
    // {
    //     FocusWindow(window);
    //     NavInitWindow(window, false); // <-- this is in the way for us to be able to defer and sort reappearing FocusWindow() calls
    // }

    // // Title bar
    // if (!(flags & ImGuiWindowFlags_NoTitleBar))
    //     RenderWindowTitleBarContents(window, ImRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y, title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y), name, p_open);

    // // Clear hit test shape every frame
    // window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

    // Pressing CTRL+C while holding on a window copy its content to the clipboard
    // This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
    // Maybe we can support CTRL+C on every element?
    /*
    //if (g.NavWindow == window && g.ActiveId == 0)
    if (g.ActiveId == window->MoveId)
        if (g.IO.KeyCtrl && IsKeyPressed(ImGuiKey_C))
            LogToClipboard();
    */

    // We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable after Begin().
    // This is useful to allow creating context menus on title bar only, etc.
    // SetLastItemData(window->MoveId, g.CurrentItemFlags, IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0, title_bar_rect);

    // [DEBUG]
    // #ifndef IMGUI_DISABLE_DEBUG_TOOLS
    //         if (g.DebugLocateId != 0 && (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
    //             DebugLocateItemResolveWithLastItem();
    // #endif

    //             // [Test Engine] Register title bar / tab
    // #ifdef IMGUI_ENABLE_TEST_ENGINE
    //         if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
    //             IMGUI_TEST_ENGINE_ITEM_ADD(g.LastItemData.Rect, g.LastItemData.ID);
    // #endif
    //     }
    //     else
    //     {
    //         // Append
    //         SetCurrentWindow(window);
    //     }

    // PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

    // // Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
    // window->WriteAccessed = false;
    // window->BeginCount++;
    // g.NextWindowData.ClearFlags();

    // Update visibility
    // if (first_begin_of_the_frame)
    // {
    //     if (flags & ImGuiWindowFlags_ChildWindow)
    //     {
    //         // Child window can be out of sight and have "negative" clip windows.
    //         // Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have no title bar).
    //         IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
    //         if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0) // FIXME: Doesn't make sense for ChildWindow??
    //         {
    //             const bool nav_request = (flags & ImGuiWindowFlags_NavFlattened) && (g.NavAnyRequest && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
    //             if (!g.LogEnabled && !nav_request)
    //                 if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
    //                     window->HiddenFramesCanSkipItems = 1;
    //         }

    //         // Hide along with parent or if parent is collapsed
    //         if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
    //             window->HiddenFramesCanSkipItems = 1;
    //         if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCannotSkipItems > 0))
    //             window->HiddenFramesCannotSkipItems = 1;
    //     }

    //     // Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been there for a long while (may remove at some point)
    //     if (style.Alpha <= 0.0f)
    //         window->HiddenFramesCanSkipItems = 1;

    //     // Update the Hidden flag
    //     bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);
    //     window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

    //     // Disable inputs for requested number of frames
    //     if (window->DisableInputsFrames > 0)
    //     {
    //         window->DisableInputsFrames--;
    //         window->Flags |= ImGuiWindowFlags_NoInputs;
    //     }

    //     // Update the SkipItems flag, used to early out of all items functions (no layout required)
    //     bool skip_items = false;
    //     if (window->Collapsed || !window->Active || hidden_regular)
    //         if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
    //             skip_items = true;
    //     window->SkipItems = skip_items;
    // }

    // return !window->SkipItems;
}

void DiscreteData::End() {}

ImGuiIO::ImGuiIO()
{
    // Most fields are initialized with zero
    memset(this, 0, sizeof(*this));
    // IM_STATIC_ASSERT(IM_ARRAYSIZE(ImGuiIO::MouseDown) == ImGuiMouseButton_COUNT && IM_ARRAYSIZE(ImGuiIO::MouseClicked) == ImGuiMouseButton_COUNT);

    // Settings
    ConfigFlags = ImGuiConfigFlags_None;
    BackendFlags = ImGuiBackendFlags_None;
    DisplaySize = ImVec2(-1.0f, -1.0f);
    DeltaTime = 1.0f / 60.0f;
    IniSavingRate = 5.0f;
    IniFilename = "imgui.ini"; // Important: "imgui.ini" is relative to current working dir, most apps will want to lock this to an absolute path (e.g. same path as executables).
    LogFilename = "imgui_log.txt";
    MouseDoubleClickTime = 0.30f;
    MouseDoubleClickMaxDist = 6.0f;
    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //     for (int i = 0; i < ImGuiKey_COUNT; i++)
    //         KeyMap[i] = -1;
    // #endif
    KeyRepeatDelay = 0.275f;
    KeyRepeatRate = 0.050f;
    HoverDelayNormal = 0.30f;
    HoverDelayShort = 0.10f;
    UserData = NULL;

    Fonts = NULL;
    FontGlobalScale = 1.0f;
    FontDefault = NULL;
    FontAllowUserScaling = false;
    DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Miscellaneous options
    MouseDrawCursor = false;
#ifdef __APPLE__
    ConfigMacOSXBehaviors = true; // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
    ConfigMacOSXBehaviors = false;
#endif
    ConfigInputTrickleEventQueue = true;
    ConfigInputTextCursorBlink = true;
    ConfigInputTextEnterKeepActive = false;
    ConfigDragClickToInputText = false;
    ConfigWindowsResizeFromEdges = true;
    ConfigWindowsMoveFromTitleBarOnly = false;
    ConfigMemoryCompactTimer = 60.0f;

    // Platform Functions
    // BackendPlatformName = BackendRendererName = NULL;
    // BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData = NULL;
    // GetClipboardTextFn = GetClipboardTextFn_DefaultImpl;   // Platform dependent default implementations
    // SetClipboardTextFn = SetClipboardTextFn_DefaultImpl;
    // ClipboardUserData = NULL;
    // SetPlatformImeDataFn = SetPlatformImeDataFn_DefaultImpl;

    // Input (NB: we already have memset zero the entire structure!)
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
    MouseDragThreshold = 6.0f;
    for (int i = 0; i < IM_ARRAYSIZE(MouseDownDuration); i++)
        MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
    for (int i = 0; i < IM_ARRAYSIZE(KeysData); i++)
    {
        KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f;
    }
    AppAcceptingEvents = true;
    BackendUsingLegacyKeyArrays = (ImS8)-1;
    BackendUsingLegacyNavInputArray = true; // assume using legacy array until proven wrong
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with new API
void ImGuiIO::AddInputCharacter(unsigned int c)
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(&g.IO == this && "Can only add events to current context.");
    if (c == 0 || !AppAcceptingEvents)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Text;
    e.Source = ImGuiInputSource_Keyboard;
    e.Text.Char = c;
    // g.InputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void ImGuiIO::AddInputCharacterUTF16(ImWchar16 c)
{
    if ((c == 0 && InputQueueSurrogate == 0) || !AppAcceptingEvents)
        return;

    if ((c & 0xFC00) == 0xD800) // High surrogate, must save
    {
        if (InputQueueSurrogate != 0)
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        InputQueueSurrogate = c;
        return;
    }

    ImWchar cp = c;
    if (InputQueueSurrogate != 0)
    {
        if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
        {
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        }
        else
        {
#if IM_UNICODE_CODEPOINT_MAX == 0xFFFF
            cp = IM_UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in ImWchar
#else
            cp = (ImWchar)(((InputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
#endif
        }

        InputQueueSurrogate = 0;
    }
    AddInputCharacter((unsigned)cp);
}

void ImGuiIO::AddInputCharactersUTF8(const char *utf8_chars)
{
    // if (!AppAcceptingEvents)
    //     return;
    // while (*utf8_chars != 0)
    // {
    //     unsigned int c = 0;
    //     utf8_chars += ImTextCharFromUtf8(&c, utf8_chars, NULL);
    //     if (c != 0)
    //         AddInputCharacter(c);
    // }
}

// FIXME: Perhaps we could clear queued events as well?
void ImGuiIO::ClearInputCharacters()
{
    InputQueueCharacters.resize(0);
}

// FIXME: Perhaps we could clear queued events as well?
void ImGuiIO::ClearInputKeys()
{
    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //     memset(KeysDown, 0, sizeof(KeysDown));
    // #endif
    for (int n = 0; n < IM_ARRAYSIZE(KeysData); n++)
    {
        KeysData[n].Down = false;
        KeysData[n].DownDuration = -1.0f;
        KeysData[n].DownDurationPrev = -1.0f;
    }
    KeyCtrl = KeyShift = KeyAlt = KeySuper = false;
    KeyMods = ImGuiMod_None;
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    for (int n = 0; n < IM_ARRAYSIZE(MouseDown); n++)
    {
        MouseDown[n] = false;
        MouseDownDuration[n] = MouseDownDurationPrev[n] = -1.0f;
    }
    MouseWheel = MouseWheelH = 0.0f;
}

static ImGuiInputEvent *FindLatestInputEvent(ImGuiInputEventType type, int arg = -1)
{
    // DiscreteDataContext &g = *GDiscreteData;
    // for (int n = g.InputEventsQueue.Size - 1; n >= 0; n--)
    // {
    //     ImGuiInputEvent *e = &g.InputEventsQueue[n];
    //     if (e->Type != type)
    //         continue;
    //     if (type == ImGuiInputEventType_Key && e->Key.Key != arg)
    //         continue;
    //     if (type == ImGuiInputEventType_MouseButton && e->MouseButton.Button != arg)
    //         continue;
    //     return e;
    // }
    // return NULL;
}

// Queue a new key down/up event.
// - ImGuiKey key:       Translated key (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
// - bool down:          Is the key down? use false to signify a key release.
// - float analog_value: 0.0f..1.0f
void ImGuiIO::AddKeyAnalogEvent(ImGuiKey key, bool down, float analog_value)
{
    // if (e->Down) { IMGUI_DEBUG_LOG_IO("AddKeyEvent() Key='%s' %d, NativeKeycode = %d, NativeScancode = %d\n", DiscreteData::GetKeyName(e->Key), e->Down, e->NativeKeycode, e->NativeScancode); }
    if (key == ImGuiKey_None || !AppAcceptingEvents)
        return;
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(&g.IO == this && "Can only add events to current context.");
    // IM_ASSERT(DiscreteData::IsNamedKeyOrModKey(key)); // Backend needs to pass a valid ImGuiKey_ constant. 0..511 values are legacy native key codes which are not accepted by this API.
    // IM_ASSERT(!DiscreteData::IsAliasKey(key));        // Backend cannot submit ImGuiKey_MouseXXX values they are automatically inferred from AddMouseXXX() events.
    IM_ASSERT(key != ImGuiMod_Shortcut); // We could easily support the translation here but it seems saner to not accept it (TestEngine perform a translation itself)

    // Verify that backend isn't mixing up using new io.AddKeyEvent() api and old io.KeysDown[] + io.KeyMap[] data.
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    IM_ASSERT((BackendUsingLegacyKeyArrays == -1 || BackendUsingLegacyKeyArrays == 0) && "Backend needs to either only use io.AddKeyEvent(), either only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
    if (BackendUsingLegacyKeyArrays == -1)
        for (int n = ImGuiKey_NamedKey_BEGIN; n < ImGuiKey_NamedKey_END; n++)
            // IM_ASSERT(KeyMap[n] == -1 && "Backend needs to either only use io.AddKeyEvent(), either only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
            BackendUsingLegacyKeyArrays = 0;
#endif
    // if (DiscreteData::IsGamepadKey(key))
    //     BackendUsingLegacyNavInputArray = false;

    // Filter duplicate (in particular: key mods and gamepad analog values are commonly spammed)
    // const ImGuiInputEvent *latest_event = FindLatestInputEvent(ImGuiInputEventType_Key, (int)key);
    // const ImGuiKeyData *key_data = DiscreteData::GetKeyData(key);
    // const bool latest_key_down = latest_event ? latest_event->Key.Down : key_data->Down;
    // const float latest_key_analog = latest_event ? latest_event->Key.AnalogValue : key_data->AnalogValue;
    // if (latest_key_down == down && latest_key_analog == analog_value)
    //     return;

    // // Add event
    // ImGuiInputEvent e;
    // e.Type = ImGuiInputEventType_Key;
    // e.Source = DiscreteData::IsGamepadKey(key) ? ImGuiInputSource_Gamepad : ImGuiInputSource_Keyboard;
    // e.Key.Key = key;
    // e.Key.Down = down;
    // e.Key.AnalogValue = analog_value;
    // g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddKeyEvent(ImGuiKey key, bool down)
{
    if (!AppAcceptingEvents)
        return;
    AddKeyAnalogEvent(key, down, down ? 1.0f : 0.0f);
}

// [Optional] Call after AddKeyEvent().
// Specify native keycode, scancode + Specify index for legacy <1.87 IsKeyXXX() functions with native indices.
// If you are writing a backend in 2022 or don't use IsKeyXXX() with native values that are not ImGuiKey values, you can avoid calling this.
void ImGuiIO::SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index)
{
    if (key == ImGuiKey_None)
        return;
    //     IM_ASSERT(DiscreteData::IsNamedKey(key));                                                         // >= 512
    //     IM_ASSERT(native_legacy_index == -1 || DiscreteData::IsLegacyKey((ImGuiKey)native_legacy_index)); // >= 0 && <= 511
    //     IM_UNUSED(native_keycode);                                                                        // Yet unused
    //     IM_UNUSED(native_scancode);                                                                       // Yet unused

    //     // Build native->imgui map so old user code can still call key functions with native 0..511 values.
    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //     const int legacy_key = (native_legacy_index != -1) ? native_legacy_index : native_keycode;
    //     if (!DiscreteData::IsLegacyKey((ImGuiKey)legacy_key))
    //         return;
    //     KeyMap[legacy_key] = key;
    //     KeyMap[key] = legacy_key;
    // #else
    //     IM_UNUSED(key);
    //     IM_UNUSED(native_legacy_index);
    // #endif
}

// Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
void ImGuiIO::SetAppAcceptingEvents(bool accepting_events)
{
    AppAcceptingEvents = accepting_events;
}

// Queue a mouse move event
void ImGuiIO::AddMousePosEvent(float x, float y)
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(&g.IO == this && "Can only add events to current context.");
    if (!AppAcceptingEvents)
        return;

    // Apply same flooring as UpdateMouseInputs()
    ImVec2 pos((x > -FLT_MAX) ? ImFloorSigned(x) : x, (y > -FLT_MAX) ? ImFloorSigned(y) : y);

    // Filter duplicate
    const ImGuiInputEvent *latest_event = FindLatestInputEvent(ImGuiInputEventType_MousePos);
    const ImVec2 latest_pos = latest_event ? ImVec2(latest_event->MousePos.PosX, latest_event->MousePos.PosY) : g.IO.MousePos;
    if (latest_pos.x == pos.x && latest_pos.y == pos.y)
        return;

    // ImGuiInputEvent e;
    // e.Type = ImGuiInputEventType_MousePos;
    // e.Source = ImGuiInputSource_Mouse;
    // e.MousePos.PosX = pos.x;
    // e.MousePos.PosY = pos.y;
    // g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddMouseButtonEvent(int mouse_button, bool down)
{
    // DiscreteDataContext &g = *GDiscreteData;
    // IM_ASSERT(&g.IO == this && "Can only add events to current context.");
    // IM_ASSERT(mouse_button >= 0 && mouse_button < ImGuiMouseButton_COUNT);
    // if (!AppAcceptingEvents)
    //     return;

    // // Filter duplicate
    // const ImGuiInputEvent *latest_event = FindLatestInputEvent(ImGuiInputEventType_MouseButton, (int)mouse_button);
    // const bool latest_button_down = latest_event ? latest_event->MouseButton.Down : g.IO.MouseDown[mouse_button];
    // if (latest_button_down == down)
    //     return;

    // ImGuiInputEvent e;
    // e.Type = ImGuiInputEventType_MouseButton;
    // e.Source = ImGuiInputSource_Mouse;
    // e.MouseButton.Button = mouse_button;
    // e.MouseButton.Down = down;
    // g.InputEventsQueue.push_back(e);
}

// Queue a mouse wheel event (most mouse/API will only have a Y component)
void ImGuiIO::AddMouseWheelEvent(float wheel_x, float wheel_y)
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(&g.IO == this && "Can only add events to current context.");

    // Filter duplicate (unlike most events, wheel values are relative and easy to filter)
    if (!AppAcceptingEvents || (wheel_x == 0.0f && wheel_y == 0.0f))
        return;

    ImGuiInputEvent e;
    // e.Type = ImGuiInputEventType_MouseWheel;
    // e.Source = ImGuiInputSource_Mouse;
    // e.MouseWheel.WheelX = wheel_x;
    // e.MouseWheel.WheelY = wheel_y;
    // g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddFocusEvent(bool focused)
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(&g.IO == this && "Can only add events to current context.");

    // Filter duplicate
    const ImGuiInputEvent *latest_event = FindLatestInputEvent(ImGuiInputEventType_Focus);
    const bool latest_focused = latest_event ? latest_event->AppFocused.Focused : !g.IO.AppFocusLost;
    if (latest_focused == focused)
        return;

    // ImGuiInputEvent e;
    // e.Type = ImGuiInputEventType_Focus;
    // e.AppFocused.Focused = focused;
    // g.InputEventsQueue.push_back(e);
}

//-----------------------------------------------------------------------------
// [SECTION] VIEWPORTS, PLATFORM WINDOWS
//-----------------------------------------------------------------------------
// - GetMainViewport()
// - SetWindowViewport() [Internal]
// - UpdateViewportsNewFrame() [Internal]
// (this section is more complete in the 'docking' branch)
//-----------------------------------------------------------------------------

ImGuiViewport *DiscreteData::GetMainViewport()
{
    DiscreteDataContext &g = *GDiscreteData;
    return g.Viewports[0];
}

void DiscreteData::SetWindowViewport(ImGuiWindow *window, ImGuiViewportP *viewport)
{
    window->Viewport = viewport;
}

// Update viewports and monitor infos
static void DiscreteData::UpdateViewportsNewFrame()
{
    DiscreteDataContext &g = *GDiscreteData;
    IM_ASSERT(g.Viewports.Size == 1);

    // Update main viewport with current platform position.
    // FIXME-VIEWPORT: Size is driven by backend/user code for backward-compatibility but we should aim to make this more consistent.
    ImGuiViewportP *main_viewport = g.Viewports[0];
    main_viewport->Flags = ImGuiViewportFlags_IsPlatformWindow | ImGuiViewportFlags_OwnedByApp;
    main_viewport->Pos = ImVec2(0.0f, 0.0f);
    main_viewport->Size = g.IO.DisplaySize;

    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];

        // Lock down space taken by menu bars and status bars, reset the offset for fucntions like BeginMainMenuBar() to alter them again.
        viewport->WorkOffsetMin = viewport->BuildWorkOffsetMin;
        viewport->WorkOffsetMax = viewport->BuildWorkOffsetMax;
        viewport->BuildWorkOffsetMin = viewport->BuildWorkOffsetMax = ImVec2(0.0f, 0.0f);
        viewport->UpdateWorkRect();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ERROR CHECKING
//-----------------------------------------------------------------------------

// Helper function to verify ABI compatibility between caller code and compiled version of Dear ImGui.
// Verify that the type sizes are matching between the calling file's compilation unit and imgui.cpp's compilation unit
// If this triggers you have an issue:
// - Most commonly: mismatched headers and compiled code version.
// - Or: mismatched configuration #define, compilation settings, packing pragma etc.
//   The configuration settings mentioned in imconfig.h must be set for all compilation units involved with Dear ImGui,
//   which is way it is required you put them in your imconfig file (and not just before including imgui.h).
//   Otherwise it is possible that different compilation units would see different structure layout
static void DiscreteData::ErrorCheckNewFrameSanityChecks()
{
    DiscreteDataContext &g = *GDiscreteData;

    // Check user IM_ASSERT macro
    // (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is incorrectly defined!
    //  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do { ... } while (0)' block.
    //  This is a common C/C++ idiom to allow multiple statements macros to be used in control flow blocks.)
    // #define IM_ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode();                    // Wrong!
    // #define IM_ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while (0)   // Correct!
    if (true)
        IM_ASSERT(1);
    else
        IM_ASSERT(0);

    // Check user data
    // (We pass an error message in the assert expression to make it visible to programmers who are not using a debugger, as most assert handlers display their argument)
    IM_ASSERT(g.Initialized);
    IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0) && "Need a positive DeltaTime!");
    IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount) && "Forgot to call Render() or EndFrame() at the end of the previous frame?");
    IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f && "Invalid DisplaySize value!");
    IM_ASSERT(g.IO.Fonts->IsBuilt() && "Font Atlas not built! Make sure you called ImGui_ImplXXXX_NewFrame() function for renderer backend, which should call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()");
    //     IM_ASSERT(g.Style.CurveTessellationTol > 0.0f && "Invalid style setting!");
    //     IM_ASSERT(g.Style.CircleTessellationMaxError > 0.0f && "Invalid style setting!");
    //     IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f && "Invalid style setting!"); // Allows us to avoid a few clamps in color computations
    //     IM_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f && "Invalid style setting.");
    //     IM_ASSERT(g.Style.WindowMenuButtonPosition == ImGuiDir_None || g.Style.WindowMenuButtonPosition == ImGuiDir_Left || g.Style.WindowMenuButtonPosition == ImGuiDir_Right);
    //     IM_ASSERT(g.Style.ColorButtonPosition == ImGuiDir_Left || g.Style.ColorButtonPosition == ImGuiDir_Right);
    // #ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    //     for (int n = ImGuiKey_NamedKey_BEGIN; n < ImGuiKey_COUNT; n++)
    //         IM_ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < ImGuiKey_LegacyNativeKey_END && "io.KeyMap[] contains an out of bound value (need to be 0..511, or -1 for unmapped key)");

    // Check: required key mapping (we intentionally do NOT check all keys to not pressure user into setting up everything, but Space is required and was only added in 1.60 WIP)
    //     if ((g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) && g.IO.BackendUsingLegacyKeyArrays == 1)
    //         IM_ASSERT(g.IO.KeyMap[ImGuiKey_Space] != -1 && "ImGuiKey_Space is not mapped, required for keyboard navigation.");
    // #endif

    // Check: the io.ConfigWindowsResizeFromEdges option requires backend to honor mouse cursor changes and set the ImGuiBackendFlags_HasMouseCursors flag accordingly.
    if (g.IO.ConfigWindowsResizeFromEdges && !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseCursors))
        g.IO.ConfigWindowsResizeFromEdges = false;
}

static void DiscreteData::ErrorCheckEndFrameSanityChecks()
{
    DiscreteDataContext &g = *GDiscreteData;

    // Verify that io.KeyXXX fields haven't been tampered with. Key mods should not be modified between NewFrame() and EndFrame()
    // One possible reason leading to this assert is that your backends update inputs _AFTER_ NewFrame().
    // It is known that when some modal native windows called mid-frame takes focus away, some backends such as GLFW will
    // send key release events mid-frame. This would normally trigger this assertion and lead to sheared inputs.
    // We silently accommodate for this case by ignoring the case where all io.KeyXXX modifiers were released (aka key_mod_flags == 0),
    // while still correctly asserting on mid-frame key press events.
    // const ImGuiKeyChord key_mods = GetMergedModsFromKeys();
    // IM_ASSERT((key_mods == 0 || g.IO.KeyMods == key_mods) && "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
    // IM_UNUSED(key_mods);

    // // [EXPERIMENTAL] Recover from errors: You may call this yourself before EndFrame().
    // // ErrorCheckEndFrameRecover();

    // // Report when there is a mismatch of Begin/BeginChild vs End/EndChild calls. Important: Remember that the Begin/BeginChild API requires you
    // // to always call End/EndChild even if Begin/BeginChild returns false! (this is unfortunately inconsistent with most other Begin* API).
    // if (g.CurrentWindowStack.Size != 1)
    // {
    //     if (g.CurrentWindowStack.Size > 1)
    //     {
    //         IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size == 1, "Mismatched Begin/BeginChild vs End/EndChild calls: did you forget to call End/EndChild?");
    //         while (g.CurrentWindowStack.Size > 1)
    //             End();
    //     }
    //     else
    //     {
    //         IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size == 1, "Mismatched Begin/BeginChild vs End/EndChild calls: did you call End/EndChild too much?");
    //     }
    // }

    // IM_ASSERT_USER_ERROR(g.GroupStack.Size == 0, "Missing EndGroup call!");
}

//-----------------------------------------------------------------------------
// [SECTION] ImGuiStorage
// Helper: Key->value storage
//-----------------------------------------------------------------------------

// std::lower_bound but without the bullshit
static ImGuiStorage::ImGuiStoragePair *LowerBound(ImVector<ImGuiStorage::ImGuiStoragePair> &data, ImGuiID key)
{
    ImGuiStorage::ImGuiStoragePair *first = data.Data;
    ImGuiStorage::ImGuiStoragePair *last = data.Data + data.Size;
    size_t count = (size_t)(last - first);
    while (count > 0)
    {
        size_t count2 = count >> 1;
        ImGuiStorage::ImGuiStoragePair *mid = first + count2;
        if (mid->key < key)
        {
            first = ++mid;
            count -= count2 + 1;
        }
        else
        {
            count = count2;
        }
    }
    return first;
}

// For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
void ImGuiStorage::BuildSortByKey()
{
    struct StaticFunc
    {
        static int IMGUI_CDECL PairComparerByID(const void *lhs, const void *rhs)
        {
            // We can't just do a subtraction because qsort uses signed integers and subtracting our ID doesn't play well with that.
            if (((const ImGuiStoragePair *)lhs)->key > ((const ImGuiStoragePair *)rhs)->key)
                return +1;
            if (((const ImGuiStoragePair *)lhs)->key < ((const ImGuiStoragePair *)rhs)->key)
                return -1;
            return 0;
        }
    };
    ImQsort(Data.Data, (size_t)Data.Size, sizeof(ImGuiStoragePair), StaticFunc::PairComparerByID);
}

int ImGuiStorage::GetInt(ImGuiID key, int default_val) const
{
    ImGuiStoragePair *it = LowerBound(const_cast<ImVector<ImGuiStoragePair> &>(Data), key);
    if (it == Data.end() || it->key != key)
        return default_val;
    return it->val_i;
}

bool ImGuiStorage::GetBool(ImGuiID key, bool default_val) const
{
    return GetInt(key, default_val ? 1 : 0) != 0;
}

float ImGuiStorage::GetFloat(ImGuiID key, float default_val) const
{
    ImGuiStoragePair *it = LowerBound(const_cast<ImVector<ImGuiStoragePair> &>(Data), key);
    if (it == Data.end() || it->key != key)
        return default_val;
    return it->val_f;
}

void *ImGuiStorage::GetVoidPtr(ImGuiID key) const
{
    ImGuiStoragePair *it = LowerBound(const_cast<ImVector<ImGuiStoragePair> &>(Data), key);
    if (it == Data.end() || it->key != key)
        return NULL;
    return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
int *ImGuiStorage::GetIntRef(ImGuiID key, int default_val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_i;
}

bool *ImGuiStorage::GetBoolRef(ImGuiID key, bool default_val)
{
    return (bool *)GetIntRef(key, default_val ? 1 : 0);
}

float *ImGuiStorage::GetFloatRef(ImGuiID key, float default_val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_f;
}

void **ImGuiStorage::GetVoidPtrRef(ImGuiID key, void *default_val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing GetInt()/SetInt() - not too bad because it only happens on explicit interaction (maximum one a frame)
void ImGuiStorage::SetInt(ImGuiID key, int val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, ImGuiStoragePair(key, val));
        return;
    }
    it->val_i = val;
}

void ImGuiStorage::SetBool(ImGuiID key, bool val)
{
    SetInt(key, val ? 1 : 0);
}

void ImGuiStorage::SetFloat(ImGuiID key, float val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, ImGuiStoragePair(key, val));
        return;
    }
    it->val_f = val;
}

void ImGuiStorage::SetVoidPtr(ImGuiID key, void *val)
{
    ImGuiStoragePair *it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, ImGuiStoragePair(key, val));
        return;
    }
    it->val_p = val;
}

void ImGuiStorage::SetAllInt(int v)
{
    for (int i = 0; i < Data.Size; i++)
        Data[i].val_i = v;
}
