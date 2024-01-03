#pragma once
#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------
#ifndef IMGUI_VERSION
#include "discretedata.h"
#endif

#include <stdio.h>  // FILE*, sscanf
#include <stdlib.h> // NULL, malloc, free, qsort, atoi, atof
#include <math.h>   // sqrtf, fabsf, fmodf, powf, floorf, ceilf, cosf, sinf
#include <limits.h> // INT_MIN, INT_MAX

// Legacy defines
#ifdef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS // Renamed in 1.74
#error Use IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef IMGUI_DISABLE_MATH_FUNCTIONS // Renamed in 1.74
#error Use IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

// Enable stb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both IMGUI_ENABLE_FREETYPE and IMGUI_ENABLE_STB_TRUETYPE together.
#ifndef IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_STB_TRUETYPE
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------
struct ImGuiContext;        // Main Dear ImGui context
struct ImGuiContextHook;    // Hook for extensions like ImGuiTestEngine
struct ImGuiWindow;         // Storage for one window
struct ImGuiWindowTempData; // Temporary storage for one window (that's the data which in theory we could ditch at the end of the frame, in practice we currently keep it for each window)
struct ImGuiWindowSettings; // Storage for a window .ini settings (we keep one of those even if the actual window wasn't instanced during this session)

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------
// Static Asserts
#define IM_STATIC_ASSERT(_COND) static_assert(_COND, "")

// "Paranoid" Debug Asserts are meant to only be enabled during specific debugging/work, otherwise would slow down the code too much.
// We currently don't have many of those so the effect is currently negligible, but onward intent to add more aggressive ones in the code.
// #define IMGUI_DEBUG_PARANOID
#ifdef IMGUI_DEBUG_PARANOID
#define IM_ASSERT_PARANOID(_EXPR) IM_ASSERT(_EXPR)
#else
#define IM_ASSERT_PARANOID(_EXPR)
#endif

// Misc Macros
#define IM_PI 3.14159265358979323846f
#ifdef _WIN32
#define IM_NEWLINE "\r\n" // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#else
#define IM_NEWLINE "\n"
#endif
#ifndef IM_TABSIZE // Until we move this to runtime and/or add proper tab support, at least allow users to compile-time override
#define IM_TABSIZE (4)
#endif
#define IM_MEMALIGN(_OFF, _ALIGN) (((_OFF) + ((_ALIGN)-1)) & ~((_ALIGN)-1))                  // Memory align e.g. IM_ALIGN(0,4)=0, IM_ALIGN(1,4)=4, IM_ALIGN(4,4)=4, IM_ALIGN(5,4)=8
#define IM_F32_TO_INT8_UNBOUND(_VAL) ((int)((_VAL) * 255.0f + ((_VAL) >= 0 ? 0.5f : -0.5f))) // Unsaturated, for display purpose
#define IM_F32_TO_INT8_SAT(_VAL) ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))                   // Saturated, always output 0..255
#define IM_FLOOR(_VAL) ((float)(int)(_VAL))                                                  // ImFloor() is not inlined in MSVC debug builds
#define IM_ROUND(_VAL) ((float)(int)((_VAL) + 0.5f))                                         //

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the ImXXX helpers functions are lower-level than ImGui functions.
// ImGui functions or the ImGui context are never called/used from other ImXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String
// - Helpers: Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: ImVec2/ImVec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: ImVec1
// - Helper: ImVec2ih
// - Helper: ImRect
// - Helper: ImBitArray
// - Helper: ImBitVector
// - Helper: ImSpan<>, ImSpanAllocator<>
// - Helper: ImPool<>
// - Helper: ImChunkStream<>
// - Helper: ImGuiTextIndex
//-----------------------------------------------------------------------------

// Helpers: Hashing
DISCRETE_API ImGuiID ImHashData(const void *data, size_t data_size, ImU32 seed = 0);
DISCRETE_API ImGuiID ImHashStr(const char *data, size_t data_size = 0, ImU32 seed = 0);

// Helpers: Sorting
#ifndef ImQsort
static inline void ImQsort(void *base, size_t count, size_t size_of_element, int(IMGUI_CDECL *compare_func)(void const *, void const *))
{
    if (count > 1)
        qsort(base, count, size_of_element, compare_func);
}
#endif

// Helpers: Maths
IM_MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that imgui_demo.cpp does _not_ use them to keep the code easy to copy)
#ifndef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define ImFabs(X) fabsf(X)
#define ImSqrt(X) sqrtf(X)
#define ImFmod(X, Y) fmodf((X), (Y))
#define ImCos(X) cosf(X)
#define ImSin(X) sinf(X)
#define ImAcos(X) acosf(X)
#define ImAtan2(Y, X) atan2f((Y), (X))
#define ImAtof(STR) atof(STR)
// #define ImFloorStd(X)     floorf(X)           // We use our own, see ImFloor() and ImFloorSigned()
#define ImCeil(X) ceilf(X)
static inline float ImPow(float x, float y) { return powf(x, y); } // DragBehaviorT/SliderBehaviorT uses ImPow with either float/double and need the precision
static inline double ImPow(double x, double y) { return pow(x, y); }
static inline float ImLog(float x) { return logf(x); } // DragBehaviorT/SliderBehaviorT uses ImLog with either float/double and need the precision
static inline double ImLog(double x) { return log(x); }
static inline int ImAbs(int x) { return x < 0 ? -x : x; }
static inline float ImAbs(float x) { return fabsf(x); }
static inline double ImAbs(double x) { return fabs(x); }
static inline float ImSign(float x) { return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f
                                                                             : 0.0f; } // Sign operator - returns -1, 0 or 1 based on sign of argument
static inline double ImSign(double x) { return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0
                                                                            : 0.0; }
#ifdef IMGUI_ENABLE_SSE
static inline float ImRsqrt(float x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
static inline float ImRsqrt(float x) { return 1.0f / sqrtf(x); }
#endif
static inline double ImRsqrt(double x) { return 1.0 / sqrt(x); }
#endif
// - ImMin/ImMax/ImClamp/ImLerp/ImSwap are used by widgets which support variety of types: signed/unsigned int/long long float/double
// (Exceptionally using templates here but we could also redefine them for those types)
template <typename T>
static inline T ImMin(T lhs, T rhs) { return lhs < rhs ? lhs : rhs; }
template <typename T>
static inline T ImMax(T lhs, T rhs) { return lhs >= rhs ? lhs : rhs; }
template <typename T>
static inline T ImClamp(T v, T mn, T mx) { return (v < mn) ? mn : (v > mx) ? mx
                                                                           : v; }
template <typename T>
static inline T ImLerp(T a, T b, float t) { return (T)(a + (b - a) * t); }
template <typename T>
static inline void ImSwap(T &a, T &b)
{
    T tmp = a;
    a = b;
    b = tmp;
}
template <typename T>
static inline T ImAddClampOverflow(T a, T b, T mn, T mx)
{
    if (b < 0 && (a < mn - b))
        return mn;
    if (b > 0 && (a > mx - b))
        return mx;
    return a + b;
}
template <typename T>
static inline T ImSubClampOverflow(T a, T b, T mn, T mx)
{
    if (b > 0 && (a < mn + b))
        return mn;
    if (b < 0 && (a > mx + b))
        return mx;
    return a - b;
}
// - Misc maths helpers
static inline ImVec2 ImMin(const ImVec2 &lhs, const ImVec2 &rhs) { return ImVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
static inline ImVec2 ImMax(const ImVec2 &lhs, const ImVec2 &rhs) { return ImVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
static inline ImVec2 ImClamp(const ImVec2 &v, const ImVec2 &mn, ImVec2 mx) { return ImVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x
                                                                                                                              : v.x,
                                                                                           (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y
                                                                                                                              : v.y); }
static inline ImVec2 ImLerp(const ImVec2 &a, const ImVec2 &b, float t) { return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline ImVec2 ImLerp(const ImVec2 &a, const ImVec2 &b, const ImVec2 &t) { return ImVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline ImVec4 ImLerp(const ImVec4 &a, const ImVec4 &b, float t) { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
static inline float ImSaturate(float f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f
                                                                                : f; }
static inline float ImLengthSqr(const ImVec2 &lhs) { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
static inline float ImLengthSqr(const ImVec4 &lhs) { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
static inline float ImInvLength(const ImVec2 &lhs, float fail_value)
{
    float d = (lhs.x * lhs.x) + (lhs.y * lhs.y);
    if (d > 0.0f)
        return ImRsqrt(d);
    return fail_value;
}
static inline float ImFloor(float f) { return (float)(int)(f); }
static inline float ImFloorSigned(float f) { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
static inline ImVec2 ImFloor(const ImVec2 &v) { return ImVec2((float)(int)(v.x), (float)(int)(v.y)); }
static inline ImVec2 ImFloorSigned(const ImVec2 &v) { return ImVec2(ImFloorSigned(v.x), ImFloorSigned(v.y)); }
static inline int ImModPositive(int a, int b) { return (a + b) % b; }
static inline float ImDot(const ImVec2 &a, const ImVec2 &b) { return a.x * b.x + a.y * b.y; }
static inline ImVec2 ImRotate(const ImVec2 &v, float cos_a, float sin_a) { return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
static inline float ImLinearSweep(float current, float target, float speed)
{
    if (current < target)
        return ImMin(current + speed, target);
    if (current > target)
        return ImMax(current - speed, target);
    return current;
}
static inline ImVec2 ImMul(const ImVec2 &lhs, const ImVec2 &rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline bool ImIsFloatAboveGuaranteedIntegerPrecision(float f) { return f <= -16777216 || f >= 16777216; }
static inline float ImExponentialMovingAverage(float avg, float sample, int n)
{
    avg -= avg / n;
    avg += sample / n;
    return avg;
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

// Helper: ImVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)
IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec1
{
    float x;
    constexpr ImVec1() : x(0.0f) {}
    constexpr ImVec1(float _x) : x(_x) {}
};

// Helper: ImVec2ih (2D vector, half-size integer, for long-term packed storage)
struct ImVec2ih
{
    short x, y;
    constexpr ImVec2ih() : x(0), y(0) {}
    constexpr ImVec2ih(short _x, short _y) : x(_x), y(_y) {}
    constexpr explicit ImVec2ih(const ImVec2 &rhs) : x((short)rhs.x), y((short)rhs.y) {}
};

// Helper: ImRect (2D axis aligned bounding-box)
// NB: we can't rely on ImVec2 math operators being available here!
struct DISCRETE_API ImRect
{
    ImVec2 Min; // Upper-left
    ImVec2 Max; // Lower-right

    constexpr ImRect() : Min(0.0f, 0.0f), Max(0.0f, 0.0f) {}
    constexpr ImRect(const ImVec2 &min, const ImVec2 &max) : Min(min), Max(max) {}
    constexpr ImRect(const ImVec4 &v) : Min(v.x, v.y), Max(v.z, v.w) {}
    constexpr ImRect(float x1, float y1, float x2, float y2) : Min(x1, y1), Max(x2, y2) {}

    ImVec2 GetCenter() const { return ImVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    ImVec2 GetSize() const { return ImVec2(Max.x - Min.x, Max.y - Min.y); }
    float GetWidth() const { return Max.x - Min.x; }
    float GetHeight() const { return Max.y - Min.y; }
    float GetArea() const { return (Max.x - Min.x) * (Max.y - Min.y); }
    ImVec2 GetTL() const { return Min; }                  // Top-left
    ImVec2 GetTR() const { return ImVec2(Max.x, Min.y); } // Top-right
    ImVec2 GetBL() const { return ImVec2(Min.x, Max.y); } // Bottom-left
    ImVec2 GetBR() const { return Max; }                  // Bottom-right
    bool Contains(const ImVec2 &p) const { return p.x >= Min.x && p.y >= Min.y && p.x < Max.x && p.y < Max.y; }
    bool Contains(const ImRect &r) const { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool Overlaps(const ImRect &r) const { return r.Min.y < Max.y && r.Max.y > Min.y && r.Min.x < Max.x && r.Max.x > Min.x; }
    void Add(const ImVec2 &p)
    {
        if (Min.x > p.x)
            Min.x = p.x;
        if (Min.y > p.y)
            Min.y = p.y;
        if (Max.x < p.x)
            Max.x = p.x;
        if (Max.y < p.y)
            Max.y = p.y;
    }
    void Add(const ImRect &r)
    {
        if (Min.x > r.Min.x)
            Min.x = r.Min.x;
        if (Min.y > r.Min.y)
            Min.y = r.Min.y;
        if (Max.x < r.Max.x)
            Max.x = r.Max.x;
        if (Max.y < r.Max.y)
            Max.y = r.Max.y;
    }
    void Expand(const float amount)
    {
        Min.x -= amount;
        Min.y -= amount;
        Max.x += amount;
        Max.y += amount;
    }
    void Expand(const ImVec2 &amount)
    {
        Min.x -= amount.x;
        Min.y -= amount.y;
        Max.x += amount.x;
        Max.y += amount.y;
    }
    void Translate(const ImVec2 &d)
    {
        Min.x += d.x;
        Min.y += d.y;
        Max.x += d.x;
        Max.y += d.y;
    }
    void TranslateX(float dx)
    {
        Min.x += dx;
        Max.x += dx;
    }
    void TranslateY(float dy)
    {
        Min.y += dy;
        Max.y += dy;
    }
    void ClipWith(const ImRect &r)
    {
        Min = ImMax(Min, r.Min);
        Max = ImMin(Max, r.Max);
    } // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
    void ClipWithFull(const ImRect &r)
    {
        Min = ImClamp(Min, r.Min, r.Max);
        Max = ImClamp(Max, r.Min, r.Max);
    } // Full version, ensure both points are fully clipped.
    void Floor()
    {
        Min.x = IM_FLOOR(Min.x);
        Min.y = IM_FLOOR(Min.y);
        Max.x = IM_FLOOR(Max.x);
        Max.y = IM_FLOOR(Max.y);
    }
    bool IsInverted() const { return Min.x > Max.x || Min.y > Max.y; }
    ImVec4 ToVec4() const { return ImVec4(Min.x, Min.y, Max.x, Max.y); }
};

//-----------------------------------------------------------------------------
// [SECTION] ImDrawList support
//-----------------------------------------------------------------------------
// ImDrawList: Helper function to calculate a circle's segment count given its radius and a "maximum error" value.
// Estimation of number of circle segment based on error is derived using method described in https://stackoverflow.com/a/2244088/15194693
// Number of segments (N) is calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for choosing segment that is
// perpendicular to X axis. Follow steps in the article from this starting condition and you will
// will get this result.
//
// Rendering circles with an odd number of segments, while mathematically correct will produce
// asymmetrical results on the raster grid. Therefore we're rounding N to next even number (7->8, 8->8, 9->10 etc.)
#define IM_ROUNDUP_TO_EVEN(_V) ((((_V) + 1) / 2) * 2)
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 4
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD, _MAXERROR) ImClamp(IM_ROUNDUP_TO_EVEN((int)ImCeil(IM_PI / ImAcos(1 - ImMin((_MAXERROR), (_RAD)) / (_RAD)))), IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and 'error'.
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N, _MAXERROR) ((_MAXERROR) / (1 - ImCos(IM_PI / ImMax((float)(_N), IM_PI))))
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N, _RAD) ((1 - ImCos(IM_PI / ImMax((float)(_N), IM_PI))) / (_RAD))

// ImDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef IM_DRAWLIST_ARCFAST_TABLE_SIZE
#define IM_DRAWLIST_ARCFAST_TABLE_SIZE 48 // Number of samples in lookup table.
#endif
#define IM_DRAWLIST_ARCFAST_SAMPLE_MAX IM_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle.

// Data shared between all ImDrawList instances
// You may want to create your own instance of this if you want to use ImDrawList completely without ImGui. In that case, watch out for future changes to this structure.
struct DiscreteDrawListSharedData
{
    ImVec2 TexUvWhitePixel;       // UV of white pixel in the atlas
    ImFont *Font;                 // Current/default font (optional, for simplified AddText overload)
    float FontSize;               // Current/default font size (optional, for simplified AddText overload)
    float CurveTessellationTol;   // Tessellation tolerance when using PathBezierCurveTo()
    float CircleSegmentMaxError;  // Number of circle segments to use per pixel of radius for AddCircle() etc
    ImVec4 ClipRectFullscreen;    // Value for PushClipRectFullscreen()
    ImDrawListFlags InitialFlags; // Initial flags at the beginning of the frame (it is possible to alter flags on a per-drawlist basis afterwards)

    // [Internal] Temp write buffer
    ImVector<ImVec2> TempBuffer;

    // [Internal] Lookup tables
    ImVec2 ArcFastVtx[IM_DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the quarter of the circle.
    float ArcFastRadiusCutoff;                         // Cutoff radius after which arc drawing will fallback to slower PathArcTo()
    ImU8 CircleSegmentCounts[64];                      // Precomputed segment count for given radius before we calculate it dynamically (to avoid calculation overhead)
    const ImVec4 *TexUvLines;                          // UV of anti-aliased lines in the atlas

    DiscreteDrawListSharedData();
    void SetCircleTessellationMaxError(float max_error);
};

struct ImDrawDataBuilder
{
    ImVector<DiscreteDrawList *> Layers[2]; // Global layers for: regular, tooltip

    void Clear()
    {
        for (int n = 0; n < IM_ARRAYSIZE(Layers); n++)
            Layers[n].resize(0);
    }
    void ClearFreeMemory()
    {
        for (int n = 0; n < IM_ARRAYSIZE(Layers); n++)
            Layers[n].clear();
    }
    int GetDrawListCount() const
    {
        int count = 0;
        for (int n = 0; n < IM_ARRAYSIZE(Layers); n++)
            count += Layers[n].Size;
        return count;
    }
    DISCRETE_API void FlattenIntoSingleLayer();
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Flags used by upcoming items
// - input: PushItemFlag() manipulates g.CurrentItemFlags, ItemAdd() calls may add extra flags.
// - output: stored in g.LastItemData.InFlags
// Current window shared by all windows.
// This is going to be exposed in imgui.h when stabilized enough.

//-----------------------------------------------------------------------------
// [SECTION] Inputs support
//-----------------------------------------------------------------------------
enum ImGuiInputEventType
{
    ImGuiInputEventType_None = 0,
    ImGuiInputEventType_MousePos,
    ImGuiInputEventType_MouseWheel,
    ImGuiInputEventType_MouseButton,
    ImGuiInputEventType_Key,
    ImGuiInputEventType_Text,
    ImGuiInputEventType_Focus,
    ImGuiInputEventType_COUNT
};

enum ImGuiInputSource
{
    ImGuiInputSource_None = 0,
    ImGuiInputSource_Mouse,
    ImGuiInputSource_Keyboard,
    ImGuiInputSource_Gamepad,
    ImGuiInputSource_Clipboard, // Currently only used by InputText()
    ImGuiInputSource_Nav,       // Stored in g.ActiveIdSource only
    ImGuiInputSource_COUNT
};

// FIXME: Structures in the union below need to be declared as anonymous unions appears to be an extension?
// Using ImVec2() would fail on Clang 'union member 'MousePos' has a non-trivial default constructor'
struct ImGuiInputEventMousePos
{
    float PosX, PosY;
};
struct ImGuiInputEventMouseWheel
{
    float WheelX, WheelY;
};
struct ImGuiInputEventMouseButton
{
    int Button;
    bool Down;
};
struct ImGuiInputEventKey
{
    ImGuiKey Key;
    bool Down;
    float AnalogValue;
};
struct ImGuiInputEventText
{
    unsigned int Char;
};
struct ImGuiInputEventAppFocused
{
    bool Focused;
};

struct ImGuiInputEvent
{
    ImGuiInputEventType Type;
    ImGuiInputSource Source;
    union
    {
        ImGuiInputEventMousePos MousePos;       // if Type == ImGuiInputEventType_MousePos
        ImGuiInputEventMouseWheel MouseWheel;   // if Type == ImGuiInputEventType_MouseWheel
        ImGuiInputEventMouseButton MouseButton; // if Type == ImGuiInputEventType_MouseButton
        ImGuiInputEventKey Key;                 // if Type == ImGuiInputEventType_Key
        ImGuiInputEventText Text;               // if Type == ImGuiInputEventType_Text
        ImGuiInputEventAppFocused AppFocused;   // if Type == ImGuiInputEventType_Focus
    };
    bool AddedByTestEngine;

    ImGuiInputEvent() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// ImGuiViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of ImGuiViewport is in fact a ImGuiViewportP.
struct ImGuiViewportP : public ImGuiViewport
{
    int DrawListsLastFrame[2];      // Last frame number the background (0) and foreground (1) draw lists were used
    DiscreteDrawList *DrawLists[2]; // Convenience background (0) and foreground (1) draw lists. We use them to draw software mouser cursor when io.MouseDrawCursor is set and to draw most debug overlays.
    DiscreteDrawData DrawDataP;
    ImDrawDataBuilder DrawDataBuilder;

    ImVec2 WorkOffsetMin;      // Work Area: Offset from Pos to top-left corner of Work Area. Generally (0,0) or (0,+main_menu_bar_height). Work Area is Full Area but without menu-bars/status-bars (so WorkArea always fit inside Pos/Size!)
    ImVec2 WorkOffsetMax;      // Work Area: Offset from Pos+Size to bottom-right corner of Work Area. Generally (0,0) or (0,-status_bar_height).
    ImVec2 BuildWorkOffsetMin; // Work Area: Offset being built during current frame. Generally >= 0.0f.
    ImVec2 BuildWorkOffsetMax; // Work Area: Offset being built during current frame. Generally <= 0.0f.

    ImGuiViewportP()
    {
        DrawListsLastFrame[0] = DrawListsLastFrame[1] = -1;
        DrawLists[0] = DrawLists[1] = NULL;
    }
    ~ImGuiViewportP()
    {
        if (DrawLists[0])
            IM_DELETE(DrawLists[0]);
        if (DrawLists[1])
            IM_DELETE(DrawLists[1]);
    }

    // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked from last frame data, and 1 pair for currently building rect)
    ImVec2 CalcWorkRectPos(const ImVec2 &off_min) const { return ImVec2(Pos.x + off_min.x, Pos.y + off_min.y); }
    ImVec2 CalcWorkRectSize(const ImVec2 &off_min, const ImVec2 &off_max) const { return ImVec2(ImMax(0.0f, Size.x - off_min.x + off_max.x), ImMax(0.0f, Size.y - off_min.y + off_max.y)); }
    void UpdateWorkRect()
    {
        WorkPos = CalcWorkRectPos(WorkOffsetMin);
        WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax);
    } // Update public fields

    // Helpers to retrieve ImRect (we don't need to store BuildWorkRect as every access tend to change it, hence the code asymmetry)
    ImRect GetMainRect() const { return ImRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    ImRect GetWorkRect() const { return ImRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    ImRect GetBuildWorkRect() const
    {
        ImVec2 pos = CalcWorkRectPos(BuildWorkOffsetMin);
        ImVec2 size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax);
        return ImRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    }
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*ImGuiContextHookCallback)(DiscreteDataContext *ctx, ImGuiContextHook *hook);
enum ImGuiContextHookType
{
    ImGuiContextHookType_NewFramePre,
    ImGuiContextHookType_NewFramePost,
    ImGuiContextHookType_EndFramePre,
    ImGuiContextHookType_EndFramePost,
    ImGuiContextHookType_RenderPre,
    ImGuiContextHookType_RenderPost,
    ImGuiContextHookType_Shutdown,
    ImGuiContextHookType_PendingRemoval_
};

struct ImGuiContextHook
{
    ImGuiID HookId; // A unique ID assigned by AddContextHook()
    ImGuiContextHookType Type;
    ImGuiID Owner;
    ImGuiContextHookCallback Callback;
    void *UserData;

    ImGuiContextHook() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] ImGuiContext (main Dear ImGui context)
//-----------------------------------------------------------------------------

struct DiscreteDataContext
{
    bool Initialized;
    bool FontAtlasOwnedByContext; // IO.Fonts-> is owned by the ImGuiContext and will be destructed along with it.
    ImGuiIO IO;
    ImFont *Font;       // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
    float FontSize;     // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale == window->FontSize(). Text height for current window.
    float FontBaseSize; // (Shortcut) == IO.FontGlobalScale * Font->Scale * Font->FontSize. Base text height.

    DiscreteDrawListSharedData DrawListSharedData;

    // Windows state
    ImVector<ImGuiWindow *> Windows;               // Windows, sorted in display order, back to front
    ImVector<ImGuiWindow *> WindowsFocusOrder;     // Root windows, sorted in focus order, back to front.
    ImVector<ImGuiWindow *> WindowsTempSortBuffer; // Temporary buffer used in EndFrame() to reorder windows so parents are kept before their child
    // ImVector<ImGuiWindowStackData> CurrentWindowStack;
    ImGuiStorage WindowsById;   // Map window's ImGuiID to ImGuiWindow*
    int WindowsActiveCount;     // Number of unique windows submitted by frame
    ImVec2 WindowsHoverPadding; // Padding around resizable windows for which hovering on counts as hovering the window == ImMax(style.TouchExtraPadding, WINDOWS_HOVER_PADDING)
    ImGuiWindow *CurrentWindow; // Window being drawn into

    double Time;
    int FrameCount;
    int FrameCountEnded;
    int FrameCountRendered;

    DiscreteDataContext(ImFontAtlas *shared_font_atlas)
    {
        Initialized = false;
        FontAtlasOwnedByContext = shared_font_atlas ? false : true;
        Font = NULL;
        FontSize = FontBaseSize = 0.0f;
        IO.Fonts = shared_font_atlas ? shared_font_atlas : IM_NEW(ImFontAtlas)();
        Time = 0.0f;
        FrameCount = 0;
        FrameCountEnded = FrameCountRendered = -1;
        // WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild = false;
        // GcCompactAll = false;
        // TestEngineHookItems = false;
        // TestEngine = NULL;
    }

    // Viewports
    ImVector<ImGuiViewportP *> Viewports; // Active viewports (Size==1 in 'master' branch). Each viewports hold their copy of ImDrawData.

    // Settings
    bool SettingsLoaded;
    float SettingsDirtyTimer; // Save .ini Settings to memory when time reaches zero
    // ImGuiTextBuffer         SettingsIniData;                    // In memory .ini settings
    // ImVector<ImGuiSettingsHandler>      SettingsHandlers;       // List of .ini settings handlers
    // ImChunkStream<ImGuiWindowSettings>  SettingsWindows;        // ImGuiWindow .ini settings entries
    // ImChunkStream<ImGuiTableSettings>   SettingsTables;         // ImGuiTable .ini settings entries
    ImVector<ImGuiContextHook> Hooks; // Hooks for extensions (e.g. test engine)
    ImGuiID HookIdNext;               // Next available HookId
};

//-----------------------------------------------------------------------------
// [SECTION] ImGuiWindowTempData, ImGuiWindow
//-----------------------------------------------------------------------------

// Transient per-window data, reset at the beginning of the frame. This used to be called ImGuiDrawContext, hence the DC variable name in ImGuiWindow.
// (That's theory, in practice the delimitation between ImGuiWindow and ImGuiWindowTempData is quite tenuous and could be reconsidered..)
// (This doesn't need a constructor because we zero-clear it as part of ImGuiWindow and all frame-temporary data are setup on Begin)
struct DISCRETE_API ImGuiWindowTempData
{
    // Layout
    ImVec2 CursorPos; // Current emitting position, in absolute coordinates.
    ImVec2 CursorPosPrevLine;
    ImVec2 CursorStartPos; // Initial position after Begin(), generally ~ window position + WindowPadding.
    ImVec2 CursorMaxPos;   // Used to implicitly calculate ContentSize at the beginning of next frame, for scrolling range and auto-resize. Always growing during the frame.
    ImVec2 IdealMaxPos;    // Used to implicitly calculate ContentSizeIdeal at the beginning of next frame, for auto-resize only. Always growing during the frame.
    ImVec2 CurrLineSize;
    ImVec2 PrevLineSize;
    float CurrLineTextBaseOffset; // Baseline offset (0.0f by default on a new line, generally == style.FramePadding.y when a framed item has been added).
    float PrevLineTextBaseOffset;
    bool IsSameLine;
    bool IsSetPos;
    ImVec1 Indent;        // Indentation / start position from left of window (increased by TreePush/TreePop, etc.)
    ImVec1 ColumnsOffset; // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and the above should be a stack to allow use cases like Tree->Column->Tree. Need revamp columns API.
    ImVec1 GroupOffset;
    ImVec2 CursorStartPosLossyness; // Record the loss of precision of CursorStartPos due to really large scrolling amount. This is used by clipper to compensentate and fix the most common use case of large scroll area.

    // Keyboard/Gamepad navigation
    // ImGuiNavLayer           NavLayerCurrent;        // Current layer, 0..31 (we currently only use 0..1)
    short NavLayersActiveMask;     // Which layers have been written to (result from previous frame)
    short NavLayersActiveMaskNext; // Which layers have been written to (accumulator for current frame)
    bool NavHideHighlightOneFrame;
    bool NavHasScroll; // Set when scrolling can be used (ScrollMax > 0.0f)

    // Miscellaneous
    bool MenuBarAppending; // FIXME: Remove this
    ImVec2 MenuBarOffset;  // MenuBarOffset.x is sort of equivalent of a per-layer CursorPos.x, saved/restored as we switch to the menu bar. The only situation when MenuBarOffset.y is > 0 if when (SafeAreaPadding.y > FramePadding.y), often used on TVs.
    // ImGuiMenuColumns        MenuColumns;            // Simplified columns storage for menu items measurement
    int TreeDepth;                   // Current tree depth.
    ImU32 TreeJumpToParentOnPopMask; // Store a copy of !g.NavIdIsAlive for TreeDepth 0..31.. Could be turned into a ImU64 if necessary.
    ImVector<ImGuiWindow *> ChildWindows;
    ImGuiStorage *StateStorage; // Current persistent per-window storage (store e.g. tree node open/close state)
    // ImGuiOldColumns*        CurrentColumns;         // Current columns set
    int CurrentTableIdx; // Current table index (into g.Tables)
    // ImGuiLayoutType         LayoutType;
    // ImGuiLayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()

    // Local parameters stacks
    // We store the current settings outside of the vectors to increase memory locality (reduce cache misses). The vectors are rarely modified. Also it allows us to not heap allocate for short-lived windows which are not using those settings.
    float ItemWidth;                  // Current item width (>0.0: width in pixels, <0.0: align xx pixels to the right of window).
    float TextWrapPos;                // Current text wrap pos.
    ImVector<float> ItemWidthStack;   // Store item widths to restore (attention: .back() is not == ItemWidth)
    ImVector<float> TextWrapPosStack; // Store text wrap pos to restore (attention: .back() is not == TextWrapPos)
};

// Storage for one window
struct DISCRETE_API ImGuiWindow
{
    char *Name;               // Window name, owned by the window.
    ImGuiID ID;               // == ImHashStr(Name)
    ImGuiWindowFlags Flags;   // See enum ImGuiWindowFlags_
    ImGuiViewportP *Viewport; // Always set in Begin(). Inactive windows may have a NULL value here if their viewport was discarded.
    ImVec2 Pos;               // Position (always rounded-up to nearest pixel)
    ImVec2 Size;              // Current size (==SizeFull or collapsed title bar size)
    ImVec2 SizeFull;          // Size when non collapsed
    ImVec2 ContentSize;       // Size of contents/scrollable client area (calculated from the extents reach of the cursor) from previous frame. Does not include window decoration or window padding.
    ImVec2 ContentSizeIdeal;
    ImVec2 ContentSizeExplicit;             // Size of contents/scrollable client area explicitly request by the user via SetNextWindowContentSize().
    ImVec2 WindowPadding;                   // Window padding at the time of Begin().
    float WindowRounding;                   // Window rounding at the time of Begin(). May be clamped lower to avoid rendering artifacts with title bar, menu bar etc.
    float WindowBorderSize;                 // Window border size at the time of Begin().
    float DecoOuterSizeX1, DecoOuterSizeY1; // Left/Up offsets. Sum of non-scrolling outer decorations (X1 generally == 0.0f. Y1 generally = TitleBarHeight + MenuBarHeight). Locked during Begin().
    float DecoOuterSizeX2, DecoOuterSizeY2; // Right/Down offsets (X2 generally == ScrollbarSize.x, Y2 == ScrollbarSizes.y).
    float DecoInnerSizeX1, DecoInnerSizeY1; // Applied AFTER/OVER InnerRect. Specialized for Tables as they use specialized form of clipping and frozen rows/columns are inside InnerRect (and not part of regular decoration sizes).
    int NameBufLen;                         // Size of buffer storing Name. May be larger than strlen(Name)!
    ImGuiID MoveId;                         // == window->GetID("#MOVE")
    ImGuiID ChildId;                        // ID of corresponding item in parent window (for navigation to return from child window to parent window)
    ImVec2 Scroll;
    ImVec2 ScrollMax;
    ImVec2 ScrollTarget;             // target scroll position. stored as cursor position with scrolling canceled out, so the highest point is always 0.0f. (FLT_MAX for no change)
    ImVec2 ScrollTargetCenterRatio;  // 0.0f = scroll so that target position is at top, 0.5f = scroll so that target position is centered
    ImVec2 ScrollTargetEdgeSnapDist; // 0.0f = no snapping, >0.0f snapping threshold
    ImVec2 ScrollbarSizes;           // Size taken by each scrollbars on their smaller axis. Pay attention! ScrollbarSizes.x == width of the vertical scrollbar, ScrollbarSizes.y = height of the horizontal scrollbar.
    bool ScrollbarX, ScrollbarY;     // Are scrollbars visible?
    bool Active;                     // Set to true on Begin(), unless Collapsed
    bool WasActive;
    bool WriteAccessed; // Set to true when any widget access the current window
    bool Collapsed;     // Set when collapsing window to become only title-bar
    bool WantCollapseToggle;
    bool SkipItems;                // Set when items can safely be all clipped (e.g. window not visible or collapsed)
    bool Appearing;                // Set during the frame where the window is appearing (or re-appearing)
    bool Hidden;                   // Do not display (== HiddenFrames*** > 0)
    bool IsFallbackWindow;         // Set on the "Debug##Default" window.
    bool IsExplicitChild;          // Set when passed _ChildWindow, left to false by BeginDocked()
    bool HasCloseButton;           // Set when the window has a close button (p_open != NULL)
    signed char ResizeBorderHeld;  // Current border being held for resize (-1: none, otherwise 0-3)
    short BeginCount;              // Number of Begin() during the current frame (generally 0 or 1, 1+ if appending via multiple Begin/End pairs)
    short BeginCountPreviousFrame; // Number of Begin() during the previous frame
    short BeginOrderWithinParent;  // Begin() order within immediate parent window, if we are a child window. Otherwise 0.
    short BeginOrderWithinContext; // Begin() order within entire imgui context. This is mostly used for debugging submission order related issues.
    short FocusOrder;              // Order within WindowsFocusOrder[], altered when windows are focused.
    ImGuiID PopupId;               // ID in the popup stack when this window is used as a popup/menu (because we use generic Name/ID for recycling)
    ImS8 AutoFitFramesX, AutoFitFramesY;
    ImS8 AutoFitChildAxises;
    bool AutoFitOnlyGrows;
    ImGuiDir AutoPosLastDirection;
    ImS8 HiddenFramesCanSkipItems;              // Hide the window for N frames
    ImS8 HiddenFramesCannotSkipItems;           // Hide the window for N frames while allowing items to be submitted so we can measure their size
    ImS8 HiddenFramesForRenderOnly;             // Hide the window until frame N at Render() time only
    ImS8 DisableInputsFrames;                   // Disable window interactions for N frames
    ImGuiCond SetWindowPosAllowFlags : 8;       // store acceptable condition flags for SetNextWindowPos() use.
    ImGuiCond SetWindowSizeAllowFlags : 8;      // store acceptable condition flags for SetNextWindowSize() use.
    ImGuiCond SetWindowCollapsedAllowFlags : 8; // store acceptable condition flags for SetNextWindowCollapsed() use.
    ImVec2 SetWindowPosVal;                     // store window position when using a non-zero Pivot (position set needs to be processed when we know the window size)
    ImVec2 SetWindowPosPivot;                   // store window pivot for positioning. ImVec2(0, 0) when positioning from top-left corner; ImVec2(0.5f, 0.5f) for centering; ImVec2(1, 1) for bottom right.

    ImVector<ImGuiID> IDStack; // ID stack. ID are hashes seeded with the value at the top of the stack. (In theory this should be in the TempData structure)
    ImGuiWindowTempData DC;    // Temporary per-window data, reset at the beginning of the frame. This used to be called ImGuiDrawContext, hence the "DC" variable name.

    // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show Windows Rectangles' viewer.
    // The main 'OuterRect', omitted as a field, is window->Rect().
    ImRect OuterRectClipped;  // == Window->Rect() just after setup in Begin(). == window->Rect() for root window.
    ImRect InnerRect;         // Inner rectangle (omit title bar, menu bar, scroll bar)
    ImRect InnerClipRect;     // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped within viewport or parent clip rect.
    ImRect WorkRect;          // Initially covers the whole scrolling region. Reduced by containers e.g columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This is meant to replace ContentRegionRect over time (from 1.71+ onward).
    ImRect ParentWorkRect;    // Backup of WorkRect before entering a container such as columns/tables. Used by e.g. SpanAllColumns functions to easily access. Stacked containers are responsible for maintaining this. // FIXME-WORKRECT: Could be a stack?
    ImRect ClipRect;          // Current clipping/scissoring rectangle, evolve as we are using PushClipRect(), etc. == DrawList->clip_rect_stack.back().
    ImRect ContentRegionRect; // FIXME: This is currently confusing/misleading. It is essentially WorkRect but not handling of scrolling. We currently rely on it as right/bottom aligned sizing operation need some size to rely on.
    ImVec2ih HitTestHoleSize; // Define an optional rectangular hole where mouse will pass-through the window.
    ImVec2ih HitTestHoleOffset;

    int LastFrameActive;  // Last frame number the window was Active.
    float LastTimeActive; // Last timestamp the window was Active (using float as we don't need high precision there)
    float ItemWidthDefault;
    ImGuiStorage StateStorage;
    // ImVector<ImGuiOldColumns> ColumnsStorage;
    float FontWindowScale; // User scale multiplier per-window, via SetWindowFontScale()
    int SettingsOffset;    // Offset into SettingsWindows[] (offsets are always valid as we only grow the array from the back)

    DiscreteDrawList *DrawList; // == &DrawListInst (for backward compatibility reason with code using imgui_internal.h we keep this a pointer)
    DiscreteDrawList DrawListInst;
    ImGuiWindow *ParentWindow; // If we are a child _or_ popup _or_ docked window, this is pointing to our parent. Otherwise NULL.
    ImGuiWindow *ParentWindowInBeginStack;
    ImGuiWindow *RootWindow;                     // Point to ourself or first ancestor that is not a child window. Doesn't cross through popups/dock nodes.
    ImGuiWindow *RootWindowPopupTree;            // Point to ourself or first ancestor that is not a child window. Cross through popups parent<>child.
    ImGuiWindow *RootWindowForTitleBarHighlight; // Point to ourself or first ancestor which will display TitleBgActive color when this window is active.
    ImGuiWindow *RootWindowForNav;               // Point to ourself or first ancestor which doesn't have the NavFlattened flag.

    ImGuiWindow *NavLastChildNavWindow; // When going to the menu bar, we remember the child window we came from. (This could probably be made implicit if we kept g.Windows sorted by last focused including child window.)
    // ImGuiID NavLastIds[ImGuiNavLayer_COUNT]; // Last known NavId for this window, per layer (0/1)
    // ImRect NavRectRel[ImGuiNavLayer_COUNT]; // Reference rectangle, in window relative space
    ImGuiID NavRootFocusScopeId; // Focus Scope ID at the time of Begin()

    int MemoryDrawListIdxCapacity; // Backup of last idx/vtx count, so when waking up the window we can preallocate and avoid iterative alloc/copy
    int MemoryDrawListVtxCapacity;
    bool MemoryCompacted; // Set when window extraneous data have been garbage collected

public:
    ImGuiWindow(ImGuiContext *context, const char *name);
    ~ImGuiWindow();

    ImGuiID GetID(const char *str, const char *str_end = NULL);
    ImGuiID GetID(const void *ptr);
    ImGuiID GetID(int n);
    ImGuiID GetIDFromRectangle(const ImRect &r_abs);

    // We don't use g.FontSize because the window may be != g.CurrentWindow.
    ImRect Rect() const { return ImRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    float CalcFontSize() const
    {
        // ImGuiContext &g = *GImGui;
        // float scale = g.FontBaseSize * FontWindowScale;
        // if (ParentWindow)
        //     scale *= ParentWindow->FontWindowScale;
        // return scale;
    }
    float TitleBarHeight() const
    {
        // DiscreteDataContext &g = *GDiscreteData;
        // return (Flags & ImGuiWindowFlags_NoTitleBar) ? 0.0f : CalcFontSize() + g.Style.FramePadding.y * 2.0f;
    }
    ImRect TitleBarRect() const { return ImRect(Pos, ImVec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight())); }
    float MenuBarHeight() const
    {
        // DiscreteDataContext &g = *GDiscreteData;
        // return (Flags & ImGuiWindowFlags_MenuBar) ? DC.MenuBarOffset.y + CalcFontSize() + g.Style.FramePadding.y * 2.0f : 0.0f;
    }
    ImRect MenuBarRect() const
    {
        float y1 = Pos.y + TitleBarHeight();
        return ImRect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight());
    }
};

namespace DiscreteData
{

    // inline DiscreteDrawList *GetForegroundDrawList(ImGuiWindow *window)
    // {
    //     IM_UNUSED(window);
    // return GetForegroundDrawList();
    // } // This seemingly unnecessary wrapper simplifies compatibility between the 'master' and 'docking' branches.

    DISCRETE_API DiscreteDrawList *GetBackgroundDrawList(ImGuiViewport *viewport); // get background draw list for the given viewport. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
    DISCRETE_API DiscreteDrawList *GetForegroundDrawList(ImGuiViewport *viewport); // get foreground draw list for the given viewport. this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.

    // Init
    DISCRETE_API void Initialize();
    DISCRETE_API void Shutdown(); // Since 1.60 this is a _private_ function. You can call DestroyContext() to destroy the context created by CreateContext().

    // Generic context hooks
    DISCRETE_API ImGuiID AddContextHook(DiscreteDataContext *context, const ImGuiContextHook *hook);
    DISCRETE_API void RemoveContextHook(DiscreteDataContext *context, ImGuiID hook_to_remove);
    DISCRETE_API void CallContextHooks(DiscreteDataContext *context, ImGuiContextHookType type);

    // Viewports
    DISCRETE_API void SetWindowViewport(ImGuiWindow *window, ImGuiViewportP *viewport);

};

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas internal API
//-----------------------------------------------------------------------------

// This structure is likely to evolve as we add support for incremental atlas updates
struct ImFontBuilderIO
{
    bool (*FontBuilder_Build)(ImFontAtlas *atlas);
};

// Helper for font builder
#ifdef IMGUI_ENABLE_STB_TRUETYPE
DISCRETE_API const ImFontBuilderIO *ImFontAtlasGetBuilderForStbTruetype();
#endif
DISCRETE_API void ImFontAtlasBuildInit(ImFontAtlas *atlas);
DISCRETE_API void ImFontAtlasBuildSetupFont(ImFontAtlas *atlas, ImFont *font, ImFontConfig *font_config, float ascent, float descent);
DISCRETE_API void ImFontAtlasBuildPackCustomRects(ImFontAtlas *atlas, void *stbrp_context_opaque);
DISCRETE_API void ImFontAtlasBuildFinish(ImFontAtlas *atlas);
DISCRETE_API void ImFontAtlasBuildRender8bppRectFromString(ImFontAtlas *atlas, int x, int y, int w, int h, const char *in_str, char in_marker_char, unsigned char in_marker_pixel_value);
DISCRETE_API void ImFontAtlasBuildRender32bppRectFromString(ImFontAtlas *atlas, int x, int y, int w, int h, const char *in_str, char in_marker_char, unsigned int in_marker_pixel_value);
DISCRETE_API void ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_multiply_factor);
DISCRETE_API void ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char *pixels, int x, int y, int w, int h, int stride);

#endif // #ifndef IMGUI_DISABLE