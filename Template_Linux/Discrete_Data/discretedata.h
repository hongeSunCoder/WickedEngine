// Library Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals, e.g. '#if IMGUI_VERSION_NUM > 12345')
#define IMGUI_VERSION "1.89.2"
#define IMGUI_VERSION_NUM 18920
#define IMGUI_HAS_TABLE

#pragma once

#ifndef IMGUI_DISABLE
//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

// Includes
#include <float.h>  // FLT_MIN, FLT_MAX
#include <stdarg.h> // va_list, va_start, va_end
#include <stddef.h> // ptrdiff_t, NULL
#include <string.h> // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

#ifndef DISCRETE_API
#define DISCRETE_API
#endif
#ifndef DISCRETE_IMPL_API
#define DISCRETE_IMPL_API DISCRETE_API
#endif

// Helper Macros
#ifndef IM_ASSERT
#include <assert.h>
#define IM_ASSERT(_EXPR) assert(_EXPR) // You can override the default assert handler by editing imconfig.h
#endif
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) // Size of a static C-style array. Don't use on pointers!
#define IM_UNUSED(_VAR) ((void)(_VAR))                             // Used to silence "unused variable warnings". Often useful as asserts may be stripped out from final builds.
#define IM_OFFSETOF(_TYPE, _MEMBER) offsetof(_TYPE, _MEMBER)       // Offset of _MEMBER within _TYPE. Standardized as offsetof() in C++11

#define IM_MSVC_RUNTIME_CHECKS_OFF
#define IM_MSVC_RUNTIME_CHECKS_RESTORE

// Forward declarations
struct DiscreteDrawCmd;
struct DiscreteDrawList;
struct DiscreteDrawData;
struct DiscreteDrawListSharedData; // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
struct DiscreteDataContext;        // Dear ImGui context (opaque structure, unless including imgui_internal.h)
struct ImFont;                     // Runtime data for a single font within a parent ImFontAtlas
struct ImFontAtlas;                // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct ImFontBuilderIO;            // Opaque interface to a font builder (stb_truetype or FreeType).
struct ImFontConfig;               // Configuration data when adding a font or merging fonts
struct ImFontGlyph;                // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
struct ImFontGlyphRangesBuilder;   // Helper to build glyph ranges from text/string data
struct ImGuiIO;                    // Main configuration and I/O between your application and ImGui
struct ImGuiViewport;              // A Platform Window (always only one in 'master' branch), in the future may represent Platform Monitor

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols in comments.
enum ImGuiKey : int;            // -> enum ImGuiKey              // Enum: A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value)
typedef int ImGuiCol;           // -> enum ImGuiCol_             // Enum: A color identifier for styling
typedef int ImGuiCond;          // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
typedef int ImGuiDataType;      // -> enum ImGuiDataType_        // Enum: A primary data type
typedef int ImGuiDir;           // -> enum ImGuiDir_             // Enum: A cardinal direction
typedef int ImGuiMouseButton;   // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int ImGuiMouseCursor;   // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor shape
typedef int ImGuiSortDirection; // -> enum ImGuiSortDirection_   // Enum: A sorting direction (ascending or descending)
typedef int ImGuiStyleVar;      // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
typedef int ImGuiTableBgTarget; // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()

// Flags (declared as int for compatibility with old C++, to allow using as flags without overhead, and to not pollute the top of this file)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols in comments.
typedef int ImDrawFlags;           // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
typedef int ImDrawListFlags;       // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
typedef int ImFontAtlasFlags;      // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas build
typedef int ImGuiBackendFlags;     // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
typedef int ImGuiButtonFlags;      // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()
typedef int ImGuiColorEditFlags;   // -> enum ImGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int ImGuiConfigFlags;      // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags
typedef int ImGuiComboFlags;       // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
typedef int ImGuiDragDropFlags;    // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int ImGuiFocusedFlags;     // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
typedef int ImGuiHoveredFlags;     // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int ImGuiInputTextFlags;   // -> enum ImGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int ImGuiKeyChord;         // -> ImGuiKey | ImGuiMod_XXX    // Flags: for storage only for now: an ImGuiKey optionally OR-ed with one or more ImGuiMod_XXX values.
typedef int ImGuiPopupFlags;       // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int ImGuiSelectableFlags;  // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()
typedef int ImGuiSliderFlags;      // -> enum ImGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int ImGuiTabBarFlags;      // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
typedef int ImGuiTabItemFlags;     // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
typedef int ImGuiTableFlags;       // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
typedef int ImGuiTableColumnFlags; // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
typedef int ImGuiTableRowFlags;    // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()
typedef int ImGuiTreeNodeFlags;    // -> enum ImGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int ImGuiViewportFlags;    // -> enum ImGuiViewportFlags_   // Flags: for ImGuiViewport
typedef int ImGuiWindowFlags;      // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

// Scalar data types
typedef unsigned int ImGuiID;     // A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char ImS8;         // 8-bit signed integer
typedef unsigned char ImU8;       // 8-bit unsigned integer
typedef signed short ImS16;       // 16-bit signed integer
typedef unsigned short ImU16;     // 16-bit unsigned integer
typedef signed int ImS32;         // 32-bit signed integer == int
typedef unsigned int ImU32;       // 32-bit unsigned integer (often used to store packed colors)
typedef signed long long ImS64;   // 64-bit signed integer
typedef unsigned long long ImU64; // 64-bit unsigned integer

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
typedef unsigned short ImWchar16; // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned int ImWchar32;   // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef IMGUI_USE_WCHAR32          // ImWchar [configurable type: override in imconfig.h with '#define IMGUI_USE_WCHAR32' to support Unicode planes 1-16]
typedef ImWchar32 ImWchar;
#else
typedef ImWchar16 ImWchar;
#endif

#ifndef ImTextureID
typedef void *ImTextureID;
#endif

// Callback and functions types
typedef void *(*ImGuiMemAllocFunc)(size_t sz, void *user_data); // Function signature for ImGui::SetAllocatorFunctions()
typedef void (*ImGuiMemFreeFunc)(void *ptr, void *user_data);   // Function signature for ImGui::SetAllocatorFunctions()

// ImVec2: 2D vector used to store positions, sizes etc. [Compile-time configurable type]
// This is a frequently used type in the API. Consider using IM_VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec2
{
    float x, y;
    constexpr ImVec2() : x(0.0f), y(0.0f) {}
    constexpr ImVec2(float _x, float _y) : x(_x), y(_y) {}
    float operator[](size_t idx) const
    {
        IM_ASSERT(idx == 0 || idx == 1);
        return (&x)[idx];
    } // We very rarely use this [] operator, the assert overhead is fine.
    float &operator[](size_t idx)
    {
        IM_ASSERT(idx == 0 || idx == 1);
        return (&x)[idx];
    } // We very rarely use this [] operator, the assert overhead is fine.
#ifdef IM_VEC2_CLASS_EXTRA
    IM_VEC2_CLASS_EXTRA // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec2.
#endif
};

// ImVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct ImVec4
{
    float x, y, z, w;
    constexpr ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
#ifdef IM_VEC4_CLASS_EXTRA
    IM_VEC4_CLASS_EXTRA // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec4.
#endif
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Dear ImGui end-user API functions
// (Note that ImGui:: being a namespace, you can add extra ImGui:: functions in your own separate file. Please don't modify imgui source files!)
//-----------------------------------------------------------------------------
namespace DiscreteData
{

    // Context creation and access
    // - Each context create its own ImFontAtlas by default. You may instance one yourself and pass it to CreateContext() to share a font atlas between contexts.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for details.
    DISCRETE_API DiscreteDataContext *CreateContext(ImFontAtlas *shared_font_atlas = NULL);
    DISCRETE_API void DestroyContext(DiscreteDataContext *ctx = NULL); // NULL = destroy current context
    DISCRETE_API DiscreteDataContext *GetCurrentContext();
    DISCRETE_API void SetCurrentContext(DiscreteDataContext *ctx);

    // Main
    DISCRETE_API ImGuiIO &GetIO();                // access the IO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
    DISCRETE_API void NewFrame();                 // start a new Dear ImGui frame, you can submit any command from this point until Render()/EndFrame().
    DISCRETE_API void EndFrame();                 // ends the Dear ImGui frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
    DISCRETE_API void Render();                   // ends the Dear ImGui frame, finalize the draw data. You can then get call GetDrawData().
    DISCRETE_API DiscreteDrawData *GetDrawData(); // valid after Render() and until the next call to NewFrame(). this is what you have to render.

    // Viewports
    // - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
    // - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
    // - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
    DISCRETE_API ImGuiViewport *GetMainViewport(); // return primary/default viewport. This can never be NULL.

    // Background/Foreground Draw Lists
    DISCRETE_API DiscreteDrawList *GetBackgroundDrawList(); // this draw list will be the first rendered one. Useful to quickly draw shapes/text behind dear imgui contents.
    DISCRETE_API DiscreteDrawList *GetForegroundDrawList(); // this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.

    // Memory Allocators
    // - Those functions are not reliant on the current context.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for more details.
    DISCRETE_API void SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void *user_data = NULL);
    DISCRETE_API void GetAllocatorFunctions(ImGuiMemAllocFunc *p_alloc_func, ImGuiMemFreeFunc *p_free_func, void **p_user_data);
    DISCRETE_API void *MemAlloc(size_t size);
    DISCRETE_API void MemFree(void *ptr);

    // Miscellaneous Utilities
    DISCRETE_API bool IsRectVisible(const ImVec2 &size);                             // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
    DISCRETE_API bool IsRectVisible(const ImVec2 &rect_min, const ImVec2 &rect_max); // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
    DISCRETE_API double GetTime();                                                   // get global imgui time. incremented by io.DeltaTime every frame.
    DISCRETE_API int GetFrameCount();                                                // get global imgui frame count. incremented by 1 every frame.
    DISCRETE_API DiscreteDrawListSharedData *GetDrawListSharedData();                // you may use this when creating your own ImDrawList instances.
    DISCRETE_API const char *GetStyleColorName(ImGuiCol idx);                        // get a string corresponding to the enum value (for display, saving, etc.).
    // DISCRETE_API void SetStateStorage(ImGuiStorage *storage);                        // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
    // DISCRETE_API ImGuiStorage *GetStateStorage();
    // DISCRETE_API bool BeginChildFrame(ImGuiID id, const ImVec2 &size, ImGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that looks like a normal widget frame
    DISCRETE_API void EndChildFrame(); // always call EndChildFrame() regardless of BeginChildFrame() return values (which indicates a collapsed/clipped window)

    // Inputs Utilities: Mouse specific
    // - To refer to a mouse button, you may use named enums in your code e.g. ImGuiMouseButton_Left, ImGuiMouseButton_Right.
    // - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
    // - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
    DISCRETE_API bool IsMouseDown(ImGuiMouseButton button);                                            // is mouse button held?
    DISCRETE_API bool IsMouseClicked(ImGuiMouseButton button, bool repeat = false);                    // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    DISCRETE_API bool IsMouseReleased(ImGuiMouseButton button);                                        // did mouse button released? (went from Down to !Down)
    DISCRETE_API bool IsMouseDoubleClicked(ImGuiMouseButton button);                                   // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    DISCRETE_API int GetMouseClickedCount(ImGuiMouseButton button);                                    // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    DISCRETE_API bool IsMouseHoveringRect(const ImVec2 &r_min, const ImVec2 &r_max, bool clip = true); // is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
    DISCRETE_API bool IsMousePosValid(const ImVec2 *mouse_pos = NULL);                                 // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    DISCRETE_API bool IsAnyMouseDown();                                                                // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
    DISCRETE_API ImVec2 GetMousePos();                                                                 // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
    DISCRETE_API ImVec2 GetMousePosOnOpeningCurrentPopup();                                            // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
    DISCRETE_API bool IsMouseDragging(ImGuiMouseButton button, float lock_threshold = -1.0f);          // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    DISCRETE_API ImVec2 GetMouseDragDelta(ImGuiMouseButton button = 0, float lock_threshold = -1.0f);  // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    DISCRETE_API void ResetMouseDragDelta(ImGuiMouseButton button = 0);                                //
    DISCRETE_API ImGuiMouseCursor GetMouseCursor();                                                    // get desired mouse cursor shape. Important: reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
    DISCRETE_API void SetMouseCursor(ImGuiMouseCursor cursor_type);                                    // set desired mouse cursor shape
    DISCRETE_API void SetNextFrameWantCaptureMouse(bool want_capture_mouse);                           // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instucts your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

    // Windows
    // - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
    // - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
    //   which clicking will set the boolean to false when clicked.
    // - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
    //   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
    // - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
    //   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
    //   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
    //    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
    //    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    // - Note that the bottom of window stack always contains a window called "Debug".
    DISCRETE_API bool Begin(const char *name, bool *p_open = NULL, ImGuiWindowFlags flags = 0);
    DISCRETE_API void End();
} // namespace DiscreteData

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------
// A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value): can represent Keyboard, Mouse and Gamepad values.
// All our named keys are >= 512. Keys value 0 to 511 are left unused as legacy native/opaque key values (< 1.87).
// Since >= 1.89 we increased typing (went from int to enum), some legacy code may need a cast to ImGuiKey.
// Read details about the 1.87 and 1.89 transition : https://github.com/ocornut/imgui/issues/4921
enum ImGuiKey : int
{
    // Keyboard
    ImGuiKey_None = 0,
    ImGuiKey_Tab = 512, // == ImGuiKey_NamedKey_BEGIN
    ImGuiKey_LeftArrow,
    ImGuiKey_RightArrow,
    ImGuiKey_UpArrow,
    ImGuiKey_DownArrow,
    ImGuiKey_PageUp,
    ImGuiKey_PageDown,
    ImGuiKey_Home,
    ImGuiKey_End,
    ImGuiKey_Insert,
    ImGuiKey_Delete,
    ImGuiKey_Backspace,
    ImGuiKey_Space,
    ImGuiKey_Enter,
    ImGuiKey_Escape,
    ImGuiKey_LeftCtrl,
    ImGuiKey_LeftShift,
    ImGuiKey_LeftAlt,
    ImGuiKey_LeftSuper,
    ImGuiKey_RightCtrl,
    ImGuiKey_RightShift,
    ImGuiKey_RightAlt,
    ImGuiKey_RightSuper,
    ImGuiKey_Menu,
    ImGuiKey_0,
    ImGuiKey_1,
    ImGuiKey_2,
    ImGuiKey_3,
    ImGuiKey_4,
    ImGuiKey_5,
    ImGuiKey_6,
    ImGuiKey_7,
    ImGuiKey_8,
    ImGuiKey_9,
    ImGuiKey_A,
    ImGuiKey_B,
    ImGuiKey_C,
    ImGuiKey_D,
    ImGuiKey_E,
    ImGuiKey_F,
    ImGuiKey_G,
    ImGuiKey_H,
    ImGuiKey_I,
    ImGuiKey_J,
    ImGuiKey_K,
    ImGuiKey_L,
    ImGuiKey_M,
    ImGuiKey_N,
    ImGuiKey_O,
    ImGuiKey_P,
    ImGuiKey_Q,
    ImGuiKey_R,
    ImGuiKey_S,
    ImGuiKey_T,
    ImGuiKey_U,
    ImGuiKey_V,
    ImGuiKey_W,
    ImGuiKey_X,
    ImGuiKey_Y,
    ImGuiKey_Z,
    ImGuiKey_F1,
    ImGuiKey_F2,
    ImGuiKey_F3,
    ImGuiKey_F4,
    ImGuiKey_F5,
    ImGuiKey_F6,
    ImGuiKey_F7,
    ImGuiKey_F8,
    ImGuiKey_F9,
    ImGuiKey_F10,
    ImGuiKey_F11,
    ImGuiKey_F12,
    ImGuiKey_Apostrophe,   // '
    ImGuiKey_Comma,        // ,
    ImGuiKey_Minus,        // -
    ImGuiKey_Period,       // .
    ImGuiKey_Slash,        // /
    ImGuiKey_Semicolon,    // ;
    ImGuiKey_Equal,        // =
    ImGuiKey_LeftBracket,  // [
    ImGuiKey_Backslash,    // \ (this text inhibit multiline comment caused by backslash)
    ImGuiKey_RightBracket, // ]
    ImGuiKey_GraveAccent,  // `
    ImGuiKey_CapsLock,
    ImGuiKey_ScrollLock,
    ImGuiKey_NumLock,
    ImGuiKey_PrintScreen,
    ImGuiKey_Pause,
    ImGuiKey_Keypad0,
    ImGuiKey_Keypad1,
    ImGuiKey_Keypad2,
    ImGuiKey_Keypad3,
    ImGuiKey_Keypad4,
    ImGuiKey_Keypad5,
    ImGuiKey_Keypad6,
    ImGuiKey_Keypad7,
    ImGuiKey_Keypad8,
    ImGuiKey_Keypad9,
    ImGuiKey_KeypadDecimal,
    ImGuiKey_KeypadDivide,
    ImGuiKey_KeypadMultiply,
    ImGuiKey_KeypadSubtract,
    ImGuiKey_KeypadAdd,
    ImGuiKey_KeypadEnter,
    ImGuiKey_KeypadEqual,

    // Gamepad (some of those are analog values, 0.0f to 1.0f)                          // NAVIGATION ACTION
    // (download controller mapping PNG/PSD at http://dearimgui.org/controls_sheets)
    ImGuiKey_GamepadStart,       // Menu (Xbox)      + (Switch)   Start/Options (PS)
    ImGuiKey_GamepadBack,        // View (Xbox)      - (Switch)   Share (PS)
    ImGuiKey_GamepadFaceLeft,    // X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
    ImGuiKey_GamepadFaceRight,   // B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
    ImGuiKey_GamepadFaceUp,      // Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
    ImGuiKey_GamepadFaceDown,    // A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
    ImGuiKey_GamepadDpadLeft,    // D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadRight,   // D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadUp,      // D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadDown,    // D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadL1,          // L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
    ImGuiKey_GamepadR1,          // R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
    ImGuiKey_GamepadL2,          // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
    ImGuiKey_GamepadR2,          // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
    ImGuiKey_GamepadL3,          // L Stick (Xbox)   L3 (Switch)  L3 (PS)
    ImGuiKey_GamepadR3,          // R Stick (Xbox)   R3 (Switch)  R3 (PS)
    ImGuiKey_GamepadLStickLeft,  // [Analog]                                         // Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickRight, // [Analog]                                         // Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickUp,    // [Analog]                                         // Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickDown,  // [Analog]                                         // Move Window (in Windowing mode)
    ImGuiKey_GamepadRStickLeft,  // [Analog]
    ImGuiKey_GamepadRStickRight, // [Analog]
    ImGuiKey_GamepadRStickUp,    // [Analog]
    ImGuiKey_GamepadRStickDown,  // [Analog]

    // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
    // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
    ImGuiKey_MouseLeft,
    ImGuiKey_MouseRight,
    ImGuiKey_MouseMiddle,
    ImGuiKey_MouseX1,
    ImGuiKey_MouseX2,
    ImGuiKey_MouseWheelX,
    ImGuiKey_MouseWheelY,

    // [Internal] Reserved for mod storage
    ImGuiKey_ReservedForModCtrl,
    ImGuiKey_ReservedForModShift,
    ImGuiKey_ReservedForModAlt,
    ImGuiKey_ReservedForModSuper,
    ImGuiKey_COUNT,

    // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
    // - This is mirroring the data also written to io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper, in a format allowing
    //   them to be accessed via standard key API, allowing calls such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
    // - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
    //   and prefer using the real keys (e.g. ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl instead of ImGuiMod_Ctrl).
    // - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
    //   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
    //   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
    ImGuiMod_None = 0,
    ImGuiMod_Ctrl = 1 << 12,     // Ctrl
    ImGuiMod_Shift = 1 << 13,    // Shift
    ImGuiMod_Alt = 1 << 14,      // Option/Menu
    ImGuiMod_Super = 1 << 15,    // Cmd/Super/Windows
    ImGuiMod_Shortcut = 1 << 11, // Alias for Ctrl (non-macOS) _or_ Super (macOS).
    ImGuiMod_Mask_ = 0xF800,     // 5-bits

    // [Internal] Prior to 1.87 we required user to fill io.KeysDown[512] using their own native index + the io.KeyMap[] array.
    // We are ditching this method but keeping a legacy path for user code doing e.g. IsKeyPressed(MY_NATIVE_KEY_CODE)
    ImGuiKey_NamedKey_BEGIN = 512,
    ImGuiKey_NamedKey_END = ImGuiKey_COUNT,
    ImGuiKey_NamedKey_COUNT = ImGuiKey_NamedKey_END - ImGuiKey_NamedKey_BEGIN,
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
    ImGuiKey_KeysData_SIZE = ImGuiKey_NamedKey_COUNT,   // Size of KeysData[]: only hold named keys
    ImGuiKey_KeysData_OFFSET = ImGuiKey_NamedKey_BEGIN, // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - ImGuiKey_KeysData_OFFSET).
#else
    ImGuiKey_KeysData_SIZE = ImGuiKey_COUNT, // Size of KeysData[]: hold legacy 0..512 keycodes + named keys
    ImGuiKey_KeysData_OFFSET = 0,            // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - ImGuiKey_KeysData_OFFSET).
#endif

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiKey_ModCtrl = ImGuiMod_Ctrl,
    ImGuiKey_ModShift = ImGuiMod_Shift,
    ImGuiKey_ModAlt = ImGuiMod_Alt,
    ImGuiKey_ModSuper = ImGuiMod_Super,          // Renamed in 1.89
    ImGuiKey_KeyPadEnter = ImGuiKey_KeypadEnter, // Renamed in 1.87
#endif
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why we have some cursors that are marked unused here
enum ImGuiMouseCursor_
{
    ImGuiMouseCursor_None = -1,
    ImGuiMouseCursor_Arrow = 0,
    ImGuiMouseCursor_TextInput,  // When hovering over InputText, etc.
    ImGuiMouseCursor_ResizeAll,  // (Unused by Dear ImGui functions)
    ImGuiMouseCursor_ResizeNS,   // When hovering over a horizontal border
    ImGuiMouseCursor_ResizeEW,   // When hovering over a vertical border or a column
    ImGuiMouseCursor_ResizeNESW, // When hovering over the bottom-left corner of a window
    ImGuiMouseCursor_ResizeNWSE, // When hovering over the bottom-right corner of a window
    ImGuiMouseCursor_Hand,       // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
    ImGuiMouseCursor_NotAllowed, // When hovering something with disallowed interaction. Usually a crossed circle.
    ImGuiMouseCursor_COUNT
};

// Backend capabilities flags stored in io.BackendFlags. Set by imgui_impl_xxx or custom backend.
enum ImGuiBackendFlags_
{
    ImGuiBackendFlags_None = 0,
    ImGuiBackendFlags_HasGamepad = 1 << 0,           // Backend Platform supports gamepad and currently has one connected.
    ImGuiBackendFlags_HasMouseCursors = 1 << 1,      // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    ImGuiBackendFlags_HasSetMousePos = 1 << 2,       // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if ImGuiConfigFlags_NavEnableSetMousePos is set).
    ImGuiBackendFlags_RendererHasVtxOffset = 1 << 3, // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
};

// Configuration flags stored in io.ConfigFlags. Set by user/application.
enum ImGuiConfigFlags_
{
    ImGuiConfigFlags_None = 0,
    ImGuiConfigFlags_NavEnableKeyboard = 1 << 0,    // Master keyboard navigation enable flag.
    ImGuiConfigFlags_NavEnableGamepad = 1 << 1,     // Master gamepad navigation enable flag. Backend also needs to set ImGuiBackendFlags_HasGamepad.
    ImGuiConfigFlags_NavEnableSetMousePos = 1 << 2, // Instruct navigation to move the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is awkward. Will update io.MousePos and set io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in your backend, otherwise ImGui will react as if the mouse is jumping around back and forth.
    ImGuiConfigFlags_NavNoCaptureKeyboard = 1 << 3, // Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.
    ImGuiConfigFlags_NoMouse = 1 << 4,              // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
    ImGuiConfigFlags_NoMouseCursorChange = 1 << 5,  // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

    // User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear ImGui)
    ImGuiConfigFlags_IsSRGB = 1 << 20,        // Application is SRGB-aware.
    ImGuiConfigFlags_IsTouchScreen = 1 << 21, // Application is using a touch screen instead of a mouse.
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Memory allocations macros, ImVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IM_MALLOC(), IM_FREE(), IM_NEW(), IM_PLACEMENT_NEW(), IM_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new> which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct ImNewWrapper
{
};
inline void *operator new(size_t, ImNewWrapper, void *ptr) { return ptr; }
inline void operator delete(void *, ImNewWrapper, void *) {} // This is only required so we can use the symmetrical new()
#define IM_ALLOC(_SIZE) DiscreteData::MemAlloc(_SIZE)
#define IM_FREE(_PTR) DiscreteData::MemFree(_PTR)
#define IM_PLACEMENT_NEW(_PTR) new (ImNewWrapper(), _PTR)
#define IM_NEW(_TYPE) new (ImNewWrapper(), DiscreteData::MemAlloc(sizeof(_TYPE))) _TYPE
template <typename T>
void IM_DELETE(T *p)
{
    if (p)
    {
        p->~T();
        DiscreteData::MemFree(p);
    }
}

//-----------------------------------------------------------------------------
// ImVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in imgui.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures used by dear imgui can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------

IM_MSVC_RUNTIME_CHECKS_OFF
template <typename T>
struct ImVector
{
    int Size;
    int Capacity;
    T *Data;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T value_type;
    typedef value_type *iterator;
    typedef const value_type *const_iterator;

    // Constructors, destructor
    inline ImVector()
    {
        Size = Capacity = 0;
        Data = NULL;
    }
    inline ImVector(const ImVector<T> &src)
    {
        Size = Capacity = 0;
        Data = NULL;
        operator=(src);
    }
    inline ImVector<T> &operator=(const ImVector<T> &src)
    {
        clear();
        resize(src.Size);
        if (src.Data)
            memcpy(Data, src.Data, (size_t)Size * sizeof(T));
        return *this;
    }
    inline ~ImVector()
    {
        if (Data)
            IM_FREE(Data);
    } // Important: does not destruct anything

    inline void clear()
    {
        if (Data)
        {
            Size = Capacity = 0;
            IM_FREE(Data);
            Data = NULL;
        }
    } // Important: does not destruct anything
    inline void clear_delete()
    {
        for (int n = 0; n < Size; n++)
            IM_DELETE(Data[n]);
        clear();
    } // Important: never called automatically! always explicit.
    inline void clear_destruct()
    {
        for (int n = 0; n < Size; n++)
            Data[n].~T();
        clear();
    } // Important: never called automatically! always explicit.

    inline bool empty() const { return Size == 0; }
    inline int size() const { return Size; }
    inline int size_in_bytes() const { return Size * (int)sizeof(T); }
    inline int max_size() const { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int capacity() const { return Capacity; }
    inline T &operator[](int i)
    {
        IM_ASSERT(i >= 0 && i < Size);
        return Data[i];
    }
    inline const T &operator[](int i) const
    {
        IM_ASSERT(i >= 0 && i < Size);
        return Data[i];
    }

    inline T *begin() { return Data; }
    inline const T *begin() const { return Data; }
    inline T *end() { return Data + Size; }
    inline const T *end() const { return Data + Size; }
    inline T &front()
    {
        IM_ASSERT(Size > 0);
        return Data[0];
    }
    inline const T &front() const
    {
        IM_ASSERT(Size > 0);
        return Data[0];
    }
    inline T &back()
    {
        IM_ASSERT(Size > 0);
        return Data[Size - 1];
    }
    inline const T &back() const
    {
        IM_ASSERT(Size > 0);
        return Data[Size - 1];
    }
    inline void swap(ImVector<T> &rhs)
    {
        int rhs_size = rhs.Size;
        rhs.Size = Size;
        Size = rhs_size;
        int rhs_cap = rhs.Capacity;
        rhs.Capacity = Capacity;
        Capacity = rhs_cap;
        T *rhs_data = rhs.Data;
        rhs.Data = Data;
        Data = rhs_data;
    }

    inline int _grow_capacity(int sz) const
    {
        int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8;
        return new_capacity > sz ? new_capacity : sz;
    }
    inline void resize(int new_size)
    {
        if (new_size > Capacity)
            reserve(_grow_capacity(new_size));
        Size = new_size;
    }
    inline void resize(int new_size, const T &v)
    {
        if (new_size > Capacity)
            reserve(_grow_capacity(new_size));
        if (new_size > Size)
            for (int n = Size; n < new_size; n++)
                memcpy(&Data[n], &v, sizeof(v));
        Size = new_size;
    }
    inline void shrink(int new_size)
    {
        IM_ASSERT(new_size <= Size);
        Size = new_size;
    } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void reserve(int new_capacity)
    {
        if (new_capacity <= Capacity)
            return;
        T *new_data = (T *)IM_ALLOC((size_t)new_capacity * sizeof(T));
        if (Data)
        {
            memcpy(new_data, Data, (size_t)Size * sizeof(T));
            IM_FREE(Data);
        }
        Data = new_data;
        Capacity = new_capacity;
    }
    inline void reserve_discard(int new_capacity)
    {
        if (new_capacity <= Capacity)
            return;
        if (Data)
            IM_FREE(Data);
        Data = (T *)IM_ALLOC((size_t)new_capacity * sizeof(T));
        Capacity = new_capacity;
    }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the ImVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void push_back(const T &v)
    {
        if (Size == Capacity)
            reserve(_grow_capacity(Size + 1));
        memcpy(&Data[Size], &v, sizeof(v));
        Size++;
    }
    inline void pop_back()
    {
        IM_ASSERT(Size > 0);
        Size--;
    }
    inline void push_front(const T &v)
    {
        if (Size == 0)
            push_back(v);
        else
            insert(Data, v);
    }
    inline T *erase(const T *it)
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T));
        Size--;
        return Data + off;
    }
    inline T *erase(const T *it, const T *it_last)
    {
        IM_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size);
        const ptrdiff_t count = it_last - it;
        const ptrdiff_t off = it - Data;
        memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T));
        Size -= (int)count;
        return Data + off;
    }
    inline T *erase_unsorted(const T *it)
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        if (it < Data + Size - 1)
            memcpy(Data + off, Data + Size - 1, sizeof(T));
        Size--;
        return Data + off;
    }
    inline T *insert(const T *it, const T &v)
    {
        IM_ASSERT(it >= Data && it <= Data + Size);
        const ptrdiff_t off = it - Data;
        if (Size == Capacity)
            reserve(_grow_capacity(Size + 1));
        if (off < (int)Size)
            memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T));
        memcpy(&Data[off], &v, sizeof(v));
        Size++;
        return Data + off;
    }
    inline bool contains(const T &v) const
    {
        const T *data = Data;
        const T *data_end = Data + Size;
        while (data < data_end)
            if (*data++ == v)
                return true;
        return false;
    }
    inline T *find(const T &v)
    {
        T *data = Data;
        const T *data_end = Data + Size;
        while (data < data_end)
            if (*data == v)
                break;
            else
                ++data;
        return data;
    }
    inline const T *find(const T &v) const
    {
        const T *data = Data;
        const T *data_end = Data + Size;
        while (data < data_end)
            if (*data == v)
                break;
            else
                ++data;
        return data;
    }
    inline bool find_erase(const T &v)
    {
        const T *it = find(v);
        if (it < Data + Size)
        {
            erase(it);
            return true;
        }
        return false;
    }
    inline bool find_erase_unsorted(const T &v)
    {
        const T *it = find(v);
        if (it < Data + Size)
        {
            erase_unsorted(it);
            return true;
        }
        return false;
    }
    inline int index_from_ptr(const T *it) const
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        return (int)off;
    }
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] ImGuiIO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to Dear ImGui using this structure.
// Access via ImGui::GetIO(). Read 'Programmer guide' section in .cpp file for general usage.
//-----------------------------------------------------------------------------

// [Internal] Storage used by IsKeyDown(), IsKeyPressed() etc functions.
// If prior to 1.87 you used io.KeysDownDuration[] (which was marked as internal), you should use GetKeyData(key)->DownDuration and *NOT* io.KeysData[key]->DownDuration.
struct ImGuiKeyData
{
    bool Down;              // True for if key is down
    float DownDuration;     // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float DownDurationPrev; // Last frame duration the key has been down
    float AnalogValue;      // 0.0f..1.0f for gamepad values
};

struct ImGuiIO
{
    //------------------------------------------------------------------
    // Configuration                            // Default value
    //------------------------------------------------------------------

    ImGuiConfigFlags ConfigFlags;   // = 0              // See ImGuiConfigFlags_ enum. Set by user/application. Gamepad/keyboard navigation options, etc.
    ImGuiBackendFlags BackendFlags; // = 0              // See ImGuiBackendFlags_ enum. Set by backend (imgui_impl_xxx files or custom backend) to communicate features supported by the backend.
    ImVec2 DisplaySize;             // <unset>          // Main display size, in pixels (generally == GetMainViewport()->Size). May change every frame.
    float DeltaTime;                // = 1.0f/60.0f     // Time elapsed since last frame, in seconds. May change every frame.
    float IniSavingRate;            // = 5.0f           // Minimum time between saving positions/sizes to .ini file, in seconds.
    const char *IniFilename;        // = "imgui.ini"    // Path to .ini file (important: default "imgui.ini" is relative to current working dir!). Set NULL to disable automatic .ini loading/saving or if you want to manually call LoadIniSettingsXXX() / SaveIniSettingsXXX() functions.
    const char *LogFilename;        // = "imgui_log.txt"// Path to .log file (default parameter to ImGui::LogToFile when no file is specified).
    float MouseDoubleClickTime;     // = 0.30f          // Time for a double-click, in seconds.
    float MouseDoubleClickMaxDist;  // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float MouseDragThreshold;       // = 6.0f           // Distance threshold before considering we are dragging.
    float KeyRepeatDelay;           // = 0.275f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float KeyRepeatRate;            // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.
    float HoverDelayNormal;         // = 0.30 sec       // Delay on hovering before IsItemHovered(ImGuiHoveredFlags_DelayNormal) returns true.
    float HoverDelayShort;          // = 0.10 sec       // Delay on hovering before IsItemHovered(ImGuiHoveredFlags_DelayShort) returns true.
    void *UserData;                 // = NULL           // Store your own data.

    ImFontAtlas *Fonts;             // <auto>           // Font atlas: load, rasterize and pack one or more fonts into a single texture.
    float FontGlobalScale;          // = 1.0f           // Global scale all fonts
    bool FontAllowUserScaling;      // = false          // Allow user scaling text of individual window with CTRL+Wheel.
    ImFont *FontDefault;            // = NULL           // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
    ImVec2 DisplayFramebufferScale; // = (1, 1)         // For retina display or other situations where window coordinates are different from framebuffer coordinates. This generally ends up in ImDrawData::FramebufferScale.

    // Miscellaneous options
    bool MouseDrawCursor;                   // = false          // Request ImGui to draw a mouse cursor for you (if you are on a platform without a mouse cursor). Cannot be easily renamed to 'io.ConfigXXX' because this is frequently used by backend implementations.
    bool ConfigMacOSXBehaviors;             // = defined(__APPLE__) // OS X style: Text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
    bool ConfigInputTrickleEventQueue;      // = true           // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    bool ConfigInputTextCursorBlink;        // = true           // Enable blinking cursor (optional as some users consider it to be distracting).
    bool ConfigInputTextEnterKeepActive;    // = false          // [BETA] Pressing Enter will keep item active and select contents (single-line only).
    bool ConfigDragClickToInputText;        // = false          // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.
    bool ConfigWindowsResizeFromEdges;      // = true           // Enable resizing of windows from their edges and from the lower-left corner. This requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback. (This used to be a per-window ImGuiWindowFlags_ResizeFromAnySide flag)
    bool ConfigWindowsMoveFromTitleBarOnly; // = false       // Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.
    float ConfigMemoryCompactTimer;         // = 60.0f          // Timer (in seconds) to free transient windows/tables memory buffers when unused. Set to -1.0f to disable.

    //------------------------------------------------------------------
    // Platform Functions
    // (the imgui_impl_xxxx backend files are setting those up for you)
    //------------------------------------------------------------------

    // Optional: Platform/Renderer backend name (informational only! will be displayed in About Window) + User data for backend/wrappers to store their own stuff.
    const char *BackendPlatformName; // = NULL
    const char *BackendRendererName; // = NULL
    void *BackendPlatformUserData;   // = NULL           // User data for platform backend
    void *BackendRendererUserData;   // = NULL           // User data for renderer backend
    void *BackendLanguageUserData;   // = NULL           // User data for non C++ programming language backend

    // Optional: Access OS clipboard
    // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard. Override to access OS clipboard on other architectures)
    const char *(*GetClipboardTextFn)(void *user_data);
    void (*SetClipboardTextFn)(void *user_data, const char *text);
    void *ClipboardUserData;

    // Optional: Notify OS Input Method Editor of the screen position of your cursor for text input position (e.g. when using Japanese/Chinese IME on Windows)
    // (default to use native imm32 api on Windows)
    // void (*SetPlatformImeDataFn)(ImGuiViewport *viewport, ImGuiPlatformImeData *data);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    void *ImeWindowHandle; // = NULL           // [Obsolete] Set ImGuiViewport::PlatformHandleRaw instead. Set this to your HWND to get automatic IME cursor positioning.
#else
    void *_UnusedPadding; // Unused field to keep data structure the same size.
#endif

    //------------------------------------------------------------------
    // Input - Call before calling NewFrame()
    //------------------------------------------------------------------

    // Input Functions
    DISCRETE_API void AddKeyEvent(ImGuiKey key, bool down);                // Queue a new key down/up event. Key should be "translated" (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
    DISCRETE_API void AddKeyAnalogEvent(ImGuiKey key, bool down, float v); // Queue a new key down/up event for analog values (e.g. ImGuiKey_Gamepad_ values). Dead-zones should be handled by the backend.
    DISCRETE_API void AddMousePosEvent(float x, float y);                  // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    DISCRETE_API void AddMouseButtonEvent(int button, bool down);          // Queue a mouse button change
    DISCRETE_API void AddMouseWheelEvent(float wh_x, float wh_y);          // Queue a mouse wheel update
    DISCRETE_API void AddFocusEvent(bool focused);                         // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    DISCRETE_API void AddInputCharacter(unsigned int c);                   // Queue a new character input
    DISCRETE_API void AddInputCharacterUTF16(ImWchar16 c);                 // Queue a new character input from a UTF-16 character, it can be a surrogate
    DISCRETE_API void AddInputCharactersUTF8(const char *str);             // Queue a new characters input from a UTF-8 string

    DISCRETE_API void SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index = -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions with native indices + specify native keycode, scancode.
    DISCRETE_API void SetAppAcceptingEvents(bool accepting_events);                                                               // Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
    DISCRETE_API void ClearInputCharacters();                                                                                     // [Internal] Clear the text input buffer manually
    DISCRETE_API void ClearInputKeys();                                                                                           // [Internal] Release all keys

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------

    bool WantCaptureMouse;        // Set when Dear ImGui will use mouse inputs, in this case do not dispatch them to your main game/application (either way, always pass on mouse inputs to imgui). (e.g. unclicked mouse is hovering over an imgui window, widget is active, mouse was clicked over an imgui window, etc.).
    bool WantCaptureKeyboard;     // Set when Dear ImGui will use keyboard inputs, in this case do not dispatch them to your main game/application (either way, always pass keyboard inputs to imgui). (e.g. InputText active, or an imgui window is focused and navigation is enabled, etc.).
    bool WantTextInput;           // Mobile/console: when set, you may display an on-screen keyboard. This is set by Dear ImGui when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
    bool WantSetMousePos;         // MousePos has been altered, backend should reposition mouse on next frame. Rarely used! Set only when ImGuiConfigFlags_NavEnableSetMousePos flag is enabled.
    bool WantSaveIniSettings;     // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. Important: clear io.WantSaveIniSettings yourself after saving!
    bool NavActive;               // Keyboard/Gamepad navigation is currently allowed (will handle ImGuiKey_NavXXX events) = a window is focused and it doesn't use the ImGuiWindowFlags_NoNavInputs flag.
    bool NavVisible;              // Keyboard/Gamepad navigation is visible and allowed (will handle ImGuiKey_NavXXX events).
    float Framerate;              // Estimate of application framerate (rolling average over 60 frames, based on io.DeltaTime), in frame per second. Solely for convenience. Slow applications may not want to use a moving average or may want to reset underlying buffers occasionally.
    int MetricsRenderVertices;    // Vertices output during last call to Render()
    int MetricsRenderIndices;     // Indices output during last call to Render() = number of triangles * 3
    int MetricsRenderWindows;     // Number of visible windows
    int MetricsActiveWindows;     // Number of active windows
    int MetricsActiveAllocations; // Number of active allocations, updated by MemAlloc/MemFree based on current context. May be off if you have multiple imgui contexts.
    ImVec2 MouseDelta;            // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    // Legacy: before 1.87, we required backend to fill io.KeyMap[] (imgui->native map) during initialization and io.KeysDown[] (native indices) every frame.
    // This is still temporarily supported as a legacy feature. However the new preferred scheme is for backend to call io.AddKeyEvent().
    //   Old (<1.87):  ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Space]) --> New (1.87+) ImGui::IsKeyPressed(ImGuiKey_Space)
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    // honge:
    // int KeyMap[ImGuiKey_COUNT];           // [LEGACY] Input: map of indices into the KeysDown[512] entries array which represent your "native" keyboard state. The first 512 are now unused and should be kept zero. Legacy backend will write into KeyMap[] using ImGuiKey_ indices which are always >512.
    // bool KeysDown[ImGuiKey_COUNT];        // [LEGACY] Input: Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys). This used to be [512] sized. It is now ImGuiKey_COUNT to allow legacy io.KeysDown[GetKeyIndex(...)] to work without an overflow.
    // float NavInputs[ImGuiNavInput_COUNT]; // [LEGACY] Since 1.88, NavInputs[] was removed. Backends from 1.60 to 1.86 won't build. Feed gamepad inputs via io.AddKeyEvent() and ImGuiKey_GamepadXXX enums.
    // honge:
#endif

    //------------------------------------------------------------------
    // [Internal] Dear ImGui will maintain those fields. Forward compatibility not guaranteed!
    //------------------------------------------------------------------

    // Main Input State
    // (this block used to be written by backend, since 1.87 it is best to NOT write to those directly, call the AddXXX functions above instead)
    // (reading from those variables is fair game, as they are extremely unlikely to be moving anywhere)
    ImVec2 MousePos;   // Mouse position, in pixels. Set to ImVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool MouseDown[5]; // Mouse buttons: 0=left, 1=right, 2=middle + extras (ImGuiMouseButton_COUNT == 5). Dear ImGui mostly uses left and right buttons. Other buttons allow us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float MouseWheel;  // Mouse wheel Vertical: 1 unit scrolls about 5 lines text.
    float MouseWheelH; // Mouse wheel Horizontal. Most users don't have a mouse with a horizontal wheel, may not be filled by all backends.
    bool KeyCtrl;      // Keyboard modifier down: Control
    bool KeyShift;     // Keyboard modifier down: Shift
    bool KeyAlt;       // Keyboard modifier down: Alt
    bool KeySuper;     // Keyboard modifier down: Cmd/Super/Windows

    // Other state maintained from data above + IO function calls
    ImGuiKeyChord KeyMods;                         // Key mods flags (any of ImGuiMod_Ctrl/ImGuiMod_Shift/ImGuiMod_Alt/ImGuiMod_Super flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags. DOES NOT CONTAINS ImGuiMod_Shortcut which is pretranslated). Read-only, updated by NewFrame()
    ImGuiKeyData KeysData[ImGuiKey_KeysData_SIZE]; // Key state for all known keys. Use IsKeyXXX() functions to access this.
    bool WantCaptureMouseUnlessPopupClose;         // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    ImVec2 MousePosPrev;                           // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    ImVec2 MouseClickedPos[5];                     // Position at time of clicking
    double MouseClickedTime[5];                    // Time of last click (used to figure out double-click)
    bool MouseClicked[5];                          // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool MouseDoubleClicked[5];                    // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    ImU16 MouseClickedCount[5];                    // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    ImU16 MouseClickedLastCount[5];                // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool MouseReleased[5];                         // Mouse button went from Down to !Down
    bool MouseDownOwned[5];                        // Track if button was clicked inside a dear imgui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside ImGui bounds.
    bool MouseDownOwnedUnlessPopupClose[5];        // Track if button was clicked inside a dear imgui window.
    float MouseDownDuration[5];                    // Duration the mouse button has been down (0.0f == just clicked)
    float MouseDownDurationPrev[5];                // Previous time the mouse button has been down
    float MouseDragMaxDistanceSqr[5];              // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    float PenPressure;                             // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear ImGui.
    bool AppFocusLost;                             // Only modify via AddFocusEvent()
    bool AppAcceptingEvents;                       // Only modify via SetAppAcceptingEvents()
    ImS8 BackendUsingLegacyKeyArrays;              // -1: unknown, 0: using AddKeyEvent(), 1: using legacy io.KeysDown[]
    bool BackendUsingLegacyNavInputArray;          // 0: using AddKeyAnalogEvent(), 1: writing to legacy io.NavInputs[] directly
    ImWchar16 InputQueueSurrogate;                 // For AddInputCharacterUTF16()
    ImVector<ImWchar> InputQueueCharacters;        // Queue of _characters_ input (obtained by platform backend). Fill using AddInputCharacter() helper.

    DISCRETE_API ImGuiIO();
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (ImDrawCmd, ImDrawIdx, ImDrawVert, ImDrawChannel, ImDrawListSplitter, ImDrawListFlags, ImDrawList, ImDrawData)
// Hold a series of drawing commands. The user provides a renderer for ImDrawData which essentially contains an array of ImDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with ImFontAtlasFlags_NoBakedLines to disable baking.
#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX (63)
#endif

// ImDrawCallback: Draw callbacks for advanced uses [configurable type: override in imconfig.h]
// NB: You most likely do NOT need to use draw callbacks just to create your own widget or customized UI rendering,
// you can poke into the draw list for that! Draw callback may be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate texture/render target, etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) { cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }'
// If you want to override the signature of ImDrawCallback, you can simply use e.g. '#define ImDrawCallback MyDrawCallback' (in imconfig.h) + update rendering backend accordingly.
#ifndef ImDrawCallback
typedef void (*ImDrawCallback)(const DiscreteDrawList *parent_list, const DiscreteDrawCmd *cmd);
#endif

// Special Draw callback value to request renderer backend to reset the graphics/render state.
// The renderer backend needs to handle this special value, otherwise it will crash trying to call a function at this address.
// This is useful for example if you submitted callbacks which you know have altered the render state and you want it to be restored.
// It is not done by default because they are many perfectly useful way of altering render state for imgui contents (e.g. changing shader/blending settings before an Image call).
#define ImDrawCallback_ResetRenderState (ImDrawCallback)(-1)

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset' is enabled,
//   this fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Backends made for <1.71. will typically ignore the VtxOffset fields.
// - The ClipRect/TextureId/VtxOffset fields must be contiguous as we memcmp() them together (this is asserted for).
struct DiscreteDrawCmd
{
    ImVec4 ClipRect;             // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    ImTextureID TextureId;       // 4-8  // User-provided texture ID. Set by user in ImfontAtlas::SetTexID() for fonts or passed to Image*() functions. Ignore if never using images or multiple fonts atlas.
    unsigned int VtxOffset;      // 4    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int IdxOffset;      // 4    // Start offset in index buffer.
    unsigned int ElemCount;      // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[] array, indices in idx_buffer[].
    ImDrawCallback UserCallback; // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void *UserCallbackData;      // 4-8  // The draw callback code can access this.

    DiscreteDrawCmd() { memset(this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    // Since 1.83: returns ImTextureID associated with this draw call. Warning: DO NOT assume this is always same as 'TextureId' (we will change this function for an upcoming feature)
    inline ImTextureID GetTexID() const { return TextureId; }
};

// ImDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset' and handle ImDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define ImDrawIdx unsigned int' in your imconfig.h file.
#ifndef ImDrawIdx
typedef unsigned short ImDrawIdx; // Default: 16-bit (for maximum compatibility with renderer backends)
#endif

#ifndef IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct ImDrawVert
{
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
};
#endif

// [Internal] For use by ImDrawList
struct ImDrawCmdHeader
{
    ImVec4 ClipRect;
    ImTextureID TextureId;
    unsigned int VtxOffset;
};

// Flags for ImDrawList functions
// (Legacy: bit 0 must always correspond to ImDrawFlags_Closed to be backward compatible with old API using a bool. Bits 1..3 must be unused)
enum ImDrawFlags_
{
    ImDrawFlags_None = 0,
    ImDrawFlags_Closed = 1 << 0,                  // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
    ImDrawFlags_RoundCornersTopLeft = 1 << 4,     // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
    ImDrawFlags_RoundCornersTopRight = 1 << 5,    // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
    ImDrawFlags_RoundCornersBottomLeft = 1 << 6,  // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
    ImDrawFlags_RoundCornersBottomRight = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
    ImDrawFlags_RoundCornersNone = 1 << 8,        // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
    ImDrawFlags_RoundCornersTop = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersBottom = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersLeft = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
    ImDrawFlags_RoundCornersRight = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersAll = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersDefault_ = ImDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
    ImDrawFlags_RoundCornersMask_ = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone,
};

// Flags for ImDrawList instance. Those are set automatically by ImGui:: functions from ImGuiIO settings, and generally not manipulated directly.
// It is however possible to temporarily alter flags between calls to ImDrawList:: functions.
enum ImDrawListFlags_
{
    ImDrawListFlags_None = 0,
    ImDrawListFlags_AntiAliasedLines = 1 << 0,       // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    ImDrawListFlags_AntiAliasedLinesUseTex = 1 << 1, // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    ImDrawListFlags_AntiAliasedFill = 1 << 2,        // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    ImDrawListFlags_AllowVtxOffset = 1 << 3,         // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
};

// Draw command list
// This is the low-level list of polygons that ImGui:: functions are filling. At the end of the frame,
// all command lists are passed to your ImGuiIO::RenderDrawListFn function for rendering.
// Each dear imgui window contains its own ImDrawList. You can use ImGui::GetWindowDrawList() to
// access the current window draw list and draw custom primitives.
// You can interleave normal ImGui:: calls and adding primitives to the current draw list.
// In single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0), bottom-right is == GetMainViewport()->Pos+Size (generally io.DisplaySize).
// You are totally free to apply whatever transformation matrix to want to the data (depending on the use of the transformation you may want to apply it to ClipRect as well!)
// Important: Primitives are always added to the list and not culled (culling is done at higher-level by ImGui:: functions), if you use this API a lot consider coarse culling your drawn objects.
struct DiscreteDrawList
{
    // This is what you have to render
    ImVector<DiscreteDrawCmd> CmdBuffer; // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    ImVector<ImDrawIdx> IdxBuffer;       // Index buffer. Each command consume ImDrawCmd::ElemCount of those
    ImVector<ImDrawVert> VtxBuffer;      // Vertex buffer.

    // internal, used while building lists
    unsigned int _VtxCurrentIdx; // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    DiscreteDrawListSharedData *_Data;
    const char *_OwnerName;          // Pointer to owner window's name for debugging
    ImDrawVert *_VtxWritePtr;        // [Internal] point within VtxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImDrawIdx *_IdxWritePtr;         // [Internal] point within IdxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImVector<ImVec4> _ClipRectStack; // [Internal]
    ImDrawCmdHeader _CmdHeader;      // [Internal] template of active commands. Fields should match those of CmdBuffer.back().

    // If you want to create ImDrawList instances, pass them ImGui::GetDrawListSharedData() or create and use your own ImDrawListSharedData (so you can use ImDrawList without ImGui)
    DiscreteDrawList(DiscreteDrawListSharedData *shared_data)
    {
        memset(this, 0, sizeof(*this));
        _Data = shared_data;
    }

    ~DiscreteDrawList() { _ClearFreeMemory(); }

    DISCRETE_API void PushClipRect(const ImVec2 &clip_rect_min, const ImVec2 &clip_rect_max, bool intersect_with_current_clip_rect = false); // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
    DISCRETE_API void PushClipRectFullScreen();
    DISCRETE_API void PopClipRect();
    DISCRETE_API void PushTextureID(ImTextureID texture_id);
    DISCRETE_API void PopTextureID();

    DISCRETE_API void AddLine(const ImVec2 &p1, const ImVec2 &p2, ImU32 col, float thickness = 1.0f);
    DISCRETE_API void AddCircle(const ImVec2 &center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
    DISCRETE_API void AddCircleFilled(const ImVec2 &center, float radius, ImU32 col, int num_segments = 0);

    // Advanced
    DISCRETE_API void AddCallback(ImDrawCallback callback, void *callback_data); // Your rendering function must check for 'UserCallback' in ImDrawCmd and call the function instead of rendering triangles.
    DISCRETE_API void AddDrawCmd();                                              // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
    DISCRETE_API DiscreteDrawList *CloneOutput() const;                          // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

    // internal helper
    DISCRETE_API void _ResetForNewFrame();
    DISCRETE_API void _ClearFreeMemory();
    DISCRETE_API void _PopUnusedDrawCmd();
    DISCRETE_API void _TryMergeDrawCmds();
    DISCRETE_API void _OnChangedClipRect();
    DISCRETE_API void _OnChangedTextureID();
    DISCRETE_API void _OnChangedVtxOffset();
};

// All draw data to render a Dear ImGui frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve them for backward compatibility purpose,
// as this is one of the oldest structure exposed by the library! Basically, ImDrawList == CmdList)
struct DiscreteDrawData
{
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;

    DiscreteDrawList **CmdLists; // Array of ImDrawList* to render. The ImDrawList are owned by ImGuiContext and only pointed to from here
    ImVec2 DisplayPos;           // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    ImVec2 DisplaySize;          // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    ImVec2 FramebufferScale;     // Amount of pixels for each unit of DisplaySize. Based on io.DisplayFramebufferScale. Generally (1,1) on normal display, (2,2) on OSX with Retina display.

    // Functions
    DiscreteDrawData() { Clear(); }
    void Clear() { memset(this, 0, sizeof(*this)); }          // The ImDrawList are owned by ImGuiContext!
    DISCRETE_API void DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
    DISCRETE_API void ScaleClipRects(const ImVec2 &fb_scale); // Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than Dear ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in ImGuiViewport::Flags, giving indications to the platform backends.
enum ImGuiViewportFlags_
{
    ImGuiViewportFlags_None = 0,
    ImGuiViewportFlags_IsPlatformWindow = 1 << 0,  // Represent a Platform Window
    ImGuiViewportFlags_IsPlatformMonitor = 1 << 1, // Represent a Platform Monitor (unused yet)
    ImGuiViewportFlags_OwnedByApp = 1 << 2,        // Platform Window: is created/managed by the application (rather than a dear imgui backend)
};

// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct ImGuiViewport
{
    ImGuiViewportFlags Flags; // See ImGuiViewportFlags_
    ImVec2 Pos;               // Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)
    ImVec2 Size;              // Main Area: Size of the viewport.
    ImVec2 WorkPos;           // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    ImVec2 WorkSize;          // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)

    // Platform/Backend Dependent Data
    void *PlatformHandleRaw; // void* to hold lower-level, platform-native window handle (under Win32 this is expected to be a HWND, unused for other platforms)

    ImGuiViewport() { memset(this, 0, sizeof(*this)); }

    // Helpers
    ImVec2 GetCenter() const { return ImVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    ImVec2 GetWorkCenter() const { return ImVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
};

//-----------------------------------------------------------------------------
// [SECTION] Obsolete functions and types
// (Will be removed! Read 'API BREAKING CHANGES' section in imgui.cpp for details)
// Please keep your copy of dear imgui up to date! Occasionally set '#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS' in imconfig.h to stay ahead.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// [SECTION] Helpers (ImGuiOnceUponAFrame, ImGuiTextFilter, ImGuiTextBuffer, ImGuiStorage, ImGuiListClipper, ImColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define IM_UNICODE_CODEPOINT_INVALID 0xFFFD // Invalid Unicode code point (standard value).
#ifdef IMGUI_USE_WCHAR32
#define IM_UNICODE_CODEPOINT_MAX 0x10FFFF // Maximum Unicode code point supported by this build.
#else
#define IM_UNICODE_CODEPOINT_MAX 0xFFFF // Maximum Unicode code point supported by this build.
#endif

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion (typically tied to user interactions aka max once a frame)
// You can use it as custom user storage for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code (probably not efficient, but convenient)
// Types are NOT stored, so it is up to you to make sure your Key don't collide with different types.
struct ImGuiStorage
{
    // [Internal]
    struct ImGuiStoragePair
    {
        ImGuiID key;
        union
        {
            int val_i;
            float val_f;
            void *val_p;
        };
        ImGuiStoragePair(ImGuiID _key, int _val_i)
        {
            key = _key;
            val_i = _val_i;
        }
        ImGuiStoragePair(ImGuiID _key, float _val_f)
        {
            key = _key;
            val_f = _val_f;
        }
        ImGuiStoragePair(ImGuiID _key, void *_val_p)
        {
            key = _key;
            val_p = _val_p;
        }
    };

    ImVector<ImGuiStoragePair> Data;

    // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
    // - Set***() functions find pair, insertion on demand if missing.
    // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new pair.
    void Clear() { Data.clear(); }
    DISCRETE_API int GetInt(ImGuiID key, int default_val = 0) const;
    DISCRETE_API void SetInt(ImGuiID key, int val);
    DISCRETE_API bool GetBool(ImGuiID key, bool default_val = false) const;
    DISCRETE_API void SetBool(ImGuiID key, bool val);
    DISCRETE_API float GetFloat(ImGuiID key, float default_val = 0.0f) const;
    DISCRETE_API void SetFloat(ImGuiID key, float val);
    DISCRETE_API void *GetVoidPtr(ImGuiID key) const; // default_val is NULL
    DISCRETE_API void SetVoidPtr(ImGuiID key, void *val);

    // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you intend to do Get+Set.
    // - References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
    // - A typical use case where this is convenient for quick hacking (e.g. add storage during a live Edit&Continue session if you can't modify existing struct)
    //      float* pvar = ImGui::GetFloatRef(key); ImGui::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    DISCRETE_API int *GetIntRef(ImGuiID key, int default_val = 0);
    DISCRETE_API bool *GetBoolRef(ImGuiID key, bool default_val = false);
    DISCRETE_API float *GetFloatRef(ImGuiID key, float default_val = 0.0f);
    DISCRETE_API void **GetVoidPtrRef(ImGuiID key, void *default_val = NULL);

    // Use on your own storage if you know only integer are being stored (open/close all tree nodes)
    DISCRETE_API void SetAllInt(int val);

    // For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
    DISCRETE_API void BuildSortByKey();
};

// Helpers macros to generate 32-bit encoded colors
// User can declare their own format by #defining the 5 _SHIFT/_MASK macros in their imconfig file.
#ifndef IM_COL32_R_SHIFT
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT 16
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 0
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#else
#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#endif
#endif
#define IM_COL32(R, G, B, A) (((ImU32)(A) << IM_COL32_A_SHIFT) | ((ImU32)(B) << IM_COL32_B_SHIFT) | ((ImU32)(G) << IM_COL32_G_SHIFT) | ((ImU32)(R) << IM_COL32_R_SHIFT))
#define IM_COL32_WHITE IM_COL32(255, 255, 255, 255) // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK IM_COL32(0, 0, 0, 255)       // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0, 0, 0, 0)   // Transparent black = 0x00000000

//-----------------------------------------------------------------------------
// [SECTION] Font API (ImFontConfig, ImFontGlyph, ImFontAtlasFlags, ImFontAtlas, ImFontGlyphRangesBuilder, ImFont)
//-----------------------------------------------------------------------------

struct ImFontConfig
{
    void *FontData;                //          // TTF/OTF data
    int FontDataSize;              //          // TTF/OTF data size
    bool FontDataOwnedByAtlas;     // true     // TTF/OTF data ownership taken by the container ImFontAtlas (will delete memory itself).
    int FontNo;                    // 0        // Index of font within TTF/OTF file
    float SizePixels;              //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
    int OversampleH;               // 3        // Rasterize at higher quality for sub-pixel positioning. Note the difference between 2 and 3 is minimal so you can reduce this to 2 to save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
    int OversampleV;               // 1        // Rasterize at higher quality for sub-pixel positioning. This is not really useful as we don't use sub-pixel positions on the Y axis.
    bool PixelSnapH;               // false    // Align every glyph to pixel boundary. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
    ImVec2 GlyphExtraSpacing;      // 0, 0     // Extra spacing (in pixels) between glyphs. Only X axis is supported for now.
    ImVec2 GlyphOffset;            // 0, 0     // Offset all glyphs from this font input.
    const ImWchar *GlyphRanges;    // NULL     // Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list). THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE.
    float GlyphMinAdvanceX;        // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font
    float GlyphMaxAdvanceX;        // FLT_MAX  // Maximum AdvanceX for glyphs
    bool MergeMode;                // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    unsigned int FontBuilderFlags; // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    float RasterizerMultiply;      // 1.0f     // Brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable.
    ImWchar EllipsisChar;          // -1       // Explicitly specify unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.

    // [Internal]
    char Name[40]; // Name (strictly to ease debugging)
    ImFont *DstFont;

    DISCRETE_API ImFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the 31+1 bitfield members, in this case maybe drop store a single u32 or we can rework this)
struct ImFontGlyph
{
    unsigned int Colored : 1;    // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int Visible : 1;    // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int Codepoint : 30; // 0x0000..0x10FFFF
    float AdvanceX;              // Distance to next character (= data from font + ImFontConfig::GlyphExtraSpacing.x baked in)
    float X0, Y0, X1, Y1;        // Glyph corners
    float U0, V0, U1, V1;        // Texture coordinates
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
// This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
struct ImFontGlyphRangesBuilder
{
    ImVector<ImU32> UsedChars; // Store 1-bit per Unicode code point (0=unused, 1=used)

    ImFontGlyphRangesBuilder() { Clear(); }
    inline void Clear()
    {
        int size_in_bytes = (IM_UNICODE_CODEPOINT_MAX + 1) / 8;
        UsedChars.resize(size_in_bytes / (int)sizeof(ImU32));
        memset(UsedChars.Data, 0, (size_t)size_in_bytes);
    }
    inline bool GetBit(size_t n) const
    {
        int off = (int)(n >> 5);
        ImU32 mask = 1u << (n & 31);
        return (UsedChars[off] & mask) != 0;
    } // Get bit n in the array
    inline void SetBit(size_t n)
    {
        int off = (int)(n >> 5);
        ImU32 mask = 1u << (n & 31);
        UsedChars[off] |= mask;
    }                                                                         // Set bit n in the array
    inline void AddChar(ImWchar c) { SetBit(c); }                             // Add character
    DISCRETE_API void AddText(const char *text, const char *text_end = NULL); // Add string (each character of the UTF-8 string are added)
    DISCRETE_API void AddRanges(const ImWchar *ranges);                       // Add ranges, e.g. builder.AddRanges(ImFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
    DISCRETE_API void BuildRanges(ImVector<ImWchar> *out_ranges);             // Output new ranges
};

// See ImFontAtlas::AddCustomRectXXX functions.
struct ImFontAtlasCustomRect
{
    unsigned short Width, Height; // Input    // Desired rectangle dimension
    unsigned short X, Y;          // Output   // Packed position in Atlas
    unsigned int GlyphID;         // Input    // For custom font glyphs only (ID < 0x110000)
    float GlyphAdvanceX;          // Input    // For custom font glyphs only: glyph xadvance
    ImVec2 GlyphOffset;           // Input    // For custom font glyphs only: glyph display offset
    ImFont *Font;                 // Input    // For custom font glyphs only: target font
    ImFontAtlasCustomRect()
    {
        Width = Height = 0;
        X = Y = 0xFFFF;
        GlyphID = 0;
        GlyphAdvanceX = 0.0f;
        GlyphOffset = ImVec2(0, 0);
        Font = NULL;
    }
    bool IsPacked() const { return X != 0xFFFF; }
};

// Flags for ImFontAtlas build
enum ImFontAtlasFlags_
{
    ImFontAtlasFlags_None = 0,
    ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0, // Don't round the height to next power of two
    ImFontAtlasFlags_NoMouseCursors = 1 << 1,     // Don't build software mouse cursors into the atlas (save a little texture memory)
    ImFontAtlasFlags_NoBakedLines = 1 << 2,       // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Dear ImGui.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= ImFontAtlasFlags_NoMouseCursors' in the font atlas).
// It is the user-code responsibility to setup/build the atlas, then upload the pixel data into a texture accessible by your graphics api.
//  - Optionally, call any of the AddFont*** functions. If you don't call any, the default font embedded in the code will be loaded for you.
//  - Call GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//  - Upload the pixels data into a texture within your graphics system (see imgui_impl_xxxx.cpp examples)
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural to your graphics API.
//    This value will be passed back to you during rendering to identify the texture. Read FAQ entry about ImTextureID for more details.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persist up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
// - This is an old API and it is currently awkward for those and various other reasons! We will address them in the future!
struct ImFontAtlas
{
    DISCRETE_API ImFontAtlas();
    DISCRETE_API ~ImFontAtlas();
    DISCRETE_API ImFont *AddFont(const ImFontConfig *font_cfg);
    DISCRETE_API ImFont *AddFontDefault(const ImFontConfig *font_cfg = NULL);
    DISCRETE_API ImFont *AddFontFromFileTTF(const char *filename, float size_pixels, const ImFontConfig *font_cfg = NULL, const ImWchar *glyph_ranges = NULL);
    DISCRETE_API ImFont *AddFontFromMemoryTTF(void *font_data, int font_size, float size_pixels, const ImFontConfig *font_cfg = NULL, const ImWchar *glyph_ranges = NULL);                                       // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
    DISCRETE_API ImFont *AddFontFromMemoryCompressedTTF(const void *compressed_font_data, int compressed_font_size, float size_pixels, const ImFontConfig *font_cfg = NULL, const ImWchar *glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
    DISCRETE_API ImFont *AddFontFromMemoryCompressedBase85TTF(const char *compressed_font_data_base85, float size_pixels, const ImFontConfig *font_cfg = NULL, const ImWchar *glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
    DISCRETE_API void ClearInputData();                                                                                                                                                                          // Clear input data (all ImFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
    DISCRETE_API void ClearTexData();                                                                                                                                                                            // Clear output texture data (CPU side). Saves RAM once the texture has been copied to graphics memory.
    DISCRETE_API void ClearFonts();                                                                                                                                                                              // Clear output font data (glyphs storage, UV coordinates).
    DISCRETE_API void Clear();                                                                                                                                                                                   // Clear all input and output.

    // Build atlas, retrieve pixel data.
    // User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
    // The pitch is always = Width * BytesPerPixels (1 or 4)
    // Building in RGBA32 format is provided for convenience and compatibility, but note that unless you manually manipulate or copy color data into
    // the texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be white (~75% of memory/bandwidth waste.
    DISCRETE_API bool Build();                                                                                                          // Build pixels data. This is called automatically for you by the GetTexData*** functions.
    DISCRETE_API void GetTexDataAsAlpha8(unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel = NULL); // 1 byte per-pixel
    DISCRETE_API void GetTexDataAsRGBA32(unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
    bool IsBuilt() const { return Fonts.Size > 0 && TexReady; }                                                                         // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent...
    void SetTexID(ImTextureID id) { TexID = id; }

    //-------------------------------------------
    // Glyph Ranges
    //-------------------------------------------

    // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
    // NB: Make sure that your string are UTF-8 and NOT in your local code page. In C++11, you can create UTF-8 string literal using the u8"Hello world" syntax. See FAQ for details.
    // NB: Consider using ImFontGlyphRangesBuilder to build glyph ranges from textual data.
    DISCRETE_API const ImWchar *GetGlyphRangesDefault();                 // Basic Latin, Extended Latin
    DISCRETE_API const ImWchar *GetGlyphRangesGreek();                   // Default + Greek and Coptic
    DISCRETE_API const ImWchar *GetGlyphRangesKorean();                  // Default + Korean characters
    DISCRETE_API const ImWchar *GetGlyphRangesJapanese();                // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
    DISCRETE_API const ImWchar *GetGlyphRangesChineseFull();             // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
    DISCRETE_API const ImWchar *GetGlyphRangesChineseSimplifiedCommon(); // Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
    DISCRETE_API const ImWchar *GetGlyphRangesCyrillic();                // Default + about 400 Cyrillic characters
    DISCRETE_API const ImWchar *GetGlyphRangesThai();                    // Default + Thai characters
    DISCRETE_API const ImWchar *GetGlyphRangesVietnamese();              // Default + Vietnamese characters

    //-------------------------------------------
    // [BETA] Custom Rectangles/Glyphs API
    //-------------------------------------------

    // You can request arbitrary rectangles to be packed into the atlas, for your own purposes.
    // - After calling Build(), you can query the rectangle position and render your pixels.
    // - If you render colored output, set 'atlas->TexPixelsUseColors = true' as this may help some backends decide of prefered texture format.
    // - You can also request your rectangles to be mapped as font glyph (given a font + Unicode point),
    //   so you can render e.g. custom colorful icons and use them as regular glyphs.
    // - Read docs/FONTS.md for more details about using colorful icons.
    // - Note: this API may be redesigned later in order to support multi-monitor varying DPI settings.
    DISCRETE_API int AddCustomRectRegular(int width, int height);
    DISCRETE_API int AddCustomRectFontGlyph(ImFont *font, ImWchar id, int width, int height, float advance_x, const ImVec2 &offset = ImVec2(0, 0));
    ImFontAtlasCustomRect *GetCustomRectByIndex(int index)
    {
        IM_ASSERT(index >= 0);
        return &CustomRects[index];
    }

    // [Internal]
    DISCRETE_API void CalcCustomRectUV(const ImFontAtlasCustomRect *rect, ImVec2 *out_uv_min, ImVec2 *out_uv_max) const;
    DISCRETE_API bool GetMouseCursorTexData(ImGuiMouseCursor cursor, ImVec2 *out_offset, ImVec2 *out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2]);

    //-------------------------------------------
    // Members
    //-------------------------------------------

    ImFontAtlasFlags Flags; // Build flags (see ImFontAtlasFlags_)
    ImTextureID TexID;      // User data to refer to the texture once it has been uploaded to user's graphic systems. It is passed back to you during rendering via the ImDrawCmd structure.
    int TexDesiredWidth;    // Texture width desired by user before Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
    int TexGlyphPadding;    // Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    bool Locked;            // Marked as Locked by ImGui::NewFrame() so attempt to modify the atlas will assert.
    void *UserData;         // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // [Internal]
    // NB: Access texture data via GetTexData*() calls! Which will setup a default font for you.
    bool TexReady;                                          // Set when texture was built matching current font input
    bool TexPixelsUseColors;                                // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format.
    unsigned char *TexPixelsAlpha8;                         // 1 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight
    unsigned int *TexPixelsRGBA32;                          // 4 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    int TexWidth;                                           // Texture width calculated during Build().
    int TexHeight;                                          // Texture height calculated during Build().
    ImVec2 TexUvScale;                                      // = (1.0f/TexWidth, 1.0f/TexHeight)
    ImVec2 TexUvWhitePixel;                                 // Texture coordinates to a white pixel
    ImVector<ImFont *> Fonts;                               // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
    ImVector<ImFontAtlasCustomRect> CustomRects;            // Rectangles for packing custom texture data into the atlas.
    ImVector<ImFontConfig> ConfigData;                      // Configuration data
    ImVec4 TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1]; // UVs for baked anti-aliased lines

    // [Internal] Font builder
    const ImFontBuilderIO *FontBuilderIO; // Opaque interface to a font builder (default to stb_truetype, can be changed to use FreeType by defining IMGUI_ENABLE_FREETYPE).
    unsigned int FontBuilderFlags;        // Shared flags (for all fonts) for custom font builder. THIS IS BUILD IMPLEMENTATION DEPENDENT. Per-font override is also available in ImFontConfig.

    // [Internal] Packing data
    int PackIdMouseCursors; // Custom texture rectangle ID for white pixel and mouse cursors
    int PackIdLines;        // Custom texture rectangle ID for baked anti-aliased lines

    // [Obsolete]
    // typedef ImFontAtlasCustomRect    CustomRect;         // OBSOLETED in 1.72+
    // typedef ImFontGlyphRangesBuilder GlyphRangesBuilder; // OBSOLETED in 1.67+
};

// Font runtime data and rendering
// ImFontAtlas automatically loads a default embedded font for you when you call GetTexDataAsAlpha8() or GetTexDataAsRGBA32().
struct ImFont
{
    // Members: Hot ~20/24 bytes (for CalcTextSize)
    ImVector<float> IndexAdvanceX; // 12-16 // out //            // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this this info, and are often bottleneck in large UI).
    float FallbackAdvanceX;        // 4     // out // = FallbackGlyph->AdvanceX
    float FontSize;                // 4     // in  //            // Height of characters/line, set during loading (don't change after loading)

    // Members: Hot ~28/40 bytes (for CalcTextSize + render loop)
    ImVector<ImWchar> IndexLookup;    // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
    ImVector<ImFontGlyph> Glyphs;     // 12-16 // out //            // All glyphs.
    const ImFontGlyph *FallbackGlyph; // 4-8   // out // = FindGlyph(FontFallbackChar)

    // Members: Cold ~32/40 bytes
    ImFontAtlas *ContainerAtlas;                                    // 4-8   // out //            // What we has been loaded into
    const ImFontConfig *ConfigData;                                 // 4-8   // in  //            // Pointer within ContainerAtlas->ConfigData
    short ConfigDataCount;                                          // 2     // in  // ~ 1        // Number of ImFontConfig involved in creating this font. Bigger than 1 when merging multiple font sources into one ImFont.
    ImWchar FallbackChar;                                           // 2     // out // = FFFD/'?' // Character used if a glyph isn't found.
    ImWchar EllipsisChar;                                           // 2     // out // = '...'    // Character used for ellipsis rendering.
    ImWchar DotChar;                                                // 2     // out // = '.'      // Character used for ellipsis rendering (if a single '...' character isn't found)
    bool DirtyLookupTables;                                         // 1     // out //
    float Scale;                                                    // 4     // in  // = 1.f      // Base font scale, multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
    float Ascent, Descent;                                          // 4+4   // out //            // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize]
    int MetricsTotalSurface;                                        // 4     // out //            // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    ImU8 Used4kPagesMap[(IM_UNICODE_CODEPOINT_MAX + 1) / 4096 / 8]; // 2 bytes if ImWchar=ImWchar16, 34 bytes if ImWchar==ImWchar32. Store 1-bit for each block of 4K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.

    // Methods
    DISCRETE_API ImFont();
    DISCRETE_API ~ImFont();
    DISCRETE_API const ImFontGlyph *FindGlyph(ImWchar c) const;
    DISCRETE_API const ImFontGlyph *FindGlyphNoFallback(ImWchar c) const;
    float GetCharAdvance(ImWchar c) const { return ((int)c < IndexAdvanceX.Size) ? IndexAdvanceX[(int)c] : FallbackAdvanceX; }
    bool IsLoaded() const { return ContainerAtlas != NULL; }
    const char *GetDebugName() const { return ConfigData ? ConfigData->Name : "<unknown>"; }

    // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
    // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
    DISCRETE_API ImVec2 CalcTextSizeA(float size, float max_width, float wrap_width, const char *text_begin, const char *text_end = NULL, const char **remaining = NULL) const; // utf8
    DISCRETE_API const char *CalcWordWrapPositionA(float scale, const char *text, const char *text_end, float wrap_width) const;
    DISCRETE_API void RenderChar(DiscreteDrawList *draw_list, float size, const ImVec2 &pos, ImU32 col, ImWchar c) const;
    DISCRETE_API void RenderText(DiscreteDrawList *draw_list, float size, const ImVec2 &pos, ImU32 col, const ImVec4 &clip_rect, const char *text_begin, const char *text_end, float wrap_width = 0.0f, bool cpu_fine_clip = false) const;

    // [Internal] Don't use!
    DISCRETE_API void BuildLookupTable();
    DISCRETE_API void ClearOutputData();
    DISCRETE_API void GrowIndex(int new_size);
    DISCRETE_API void AddGlyph(const ImFontConfig *src_cfg, ImWchar c, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);
    DISCRETE_API void AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst = true); // Makes 'dst' character/glyph points to 'src' character/glyph. Currently needs to be called AFTER fonts have been built.
    DISCRETE_API void SetGlyphVisible(ImWchar c, bool visible);
    DISCRETE_API bool IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

#endif // #ifndef IMGUI_DISABLE