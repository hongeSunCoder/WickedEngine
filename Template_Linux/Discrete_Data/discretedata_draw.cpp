#include "discretedata.h"
#include "discretedata_internal.h"

//-----------------------------------------------------------------------------
// [SECTION] ImDrawList
//-----------------------------------------------------------------------------

DiscreteDrawListSharedData::DiscreteDrawListSharedData()
{
    memset(this, 0, sizeof(*this));
    for (int i = 0; i < IM_ARRAYSIZE(ArcFastVtx); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(ArcFastVtx);
        ArcFastVtx[i] = ImVec2(ImCos(a), ImSin(a));
    }
    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

// Initialize before use in a new frame. We always have a command ready in the buffer.
void DiscreteDrawList::_ResetForNewFrame()
{
    // Verify that the ImDrawCmd fields we want to memcmp() are contiguous in memory.
    IM_STATIC_ASSERT(IM_OFFSETOF(DiscreteDrawCmd, ClipRect) == 0);
    IM_STATIC_ASSERT(IM_OFFSETOF(DiscreteDrawCmd, TextureId) == sizeof(ImVec4));
    IM_STATIC_ASSERT(IM_OFFSETOF(DiscreteDrawCmd, VtxOffset) == sizeof(ImVec4) + sizeof(ImTextureID));

    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    // Flags = _Data->InitialFlags;
    // memset(&_CmdHeader, 0, sizeof(_CmdHeader));
    // _VtxCurrentIdx = 0;
    // _VtxWritePtr = NULL;
    // _IdxWritePtr = NULL;
    // _ClipRectStack.resize(0);
    // _TextureIdStack.resize(0);
    // _Path.resize(0);
    // _Splitter.Clear();
    // CmdBuffer.push_back(ImDrawCmd());
    // _FringeScale = 1.0f;
}

void DiscreteDrawList::_ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    // Flags = ImDrawListFlags_None;
    // _VtxCurrentIdx = 0;
    // _VtxWritePtr = NULL;
    // _IdxWritePtr = NULL;
    // _ClipRectStack.clear();
    // _TextureIdStack.clear();
    // _Path.clear();
    // _Splitter.ClearFreeMemory();
}

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void DiscreteDrawList::PushClipRect(const ImVec2 &cr_min, const ImVec2 &cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect)
    {
        ImVec4 current = _CmdHeader.ClipRect;
        if (cr.x < current.x)
            cr.x = current.x;
        if (cr.y < current.y)
            cr.y = current.y;
        if (cr.z > current.z)
            cr.z = current.z;
        if (cr.w > current.w)
            cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    _CmdHeader.ClipRect = cr;
    _OnChangedClipRect();
}

void DiscreteDrawList::AddDrawCmd()
{
    DiscreteDrawCmd draw_cmd;
    draw_cmd.ClipRect = _CmdHeader.ClipRect; // Same as calling ImDrawCmd_HeaderCopy()
    draw_cmd.TextureId = _CmdHeader.TextureId;
    draw_cmd.VtxOffset = _CmdHeader.VtxOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

// Pop trailing draw command (used before merging or presenting to user)
// Note that this leaves the ImDrawList in a state unfit for further commands, as most code assume that CmdBuffer.Size > 0 && CmdBuffer.back().UserCallback == NULL
void DiscreteDrawList::_PopUnusedDrawCmd()
{
    while (CmdBuffer.Size > 0)
    {
        DiscreteDrawCmd *curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        if (curr_cmd->ElemCount != 0 || curr_cmd->UserCallback != NULL)
            return; // break;
        CmdBuffer.pop_back();
    }
}

// Compare ClipRect, TextureId and VtxOffset with a single memcmp()
#define ImDrawCmd_HeaderSize (IM_OFFSETOF(DiscreteDrawCmd, VtxOffset) + sizeof(unsigned int))
#define ImDrawCmd_HeaderCompare(CMD_LHS, CMD_RHS) (memcmp(CMD_LHS, CMD_RHS, ImDrawCmd_HeaderSize)) // Compare ClipRect, TextureId, VtxOffset
#define ImDrawCmd_HeaderCopy(CMD_DST, CMD_SRC) (memcpy(CMD_DST, CMD_SRC, ImDrawCmd_HeaderSize))    // Copy ClipRect, TextureId, VtxOffset
#define ImDrawCmd_AreSequentialIdxOffset(CMD_0, CMD_1) (CMD_0->IdxOffset + CMD_0->ElemCount == CMD_1->IdxOffset)

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void DiscreteDrawList::_OnChangedClipRect()
{
    // If current command is used with different settings we need to add a new command
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    DiscreteDrawCmd *curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &_CmdHeader.ClipRect, sizeof(ImVec4)) != 0)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);

    // Try to merge with previous command if it matches, else use current command
    DiscreteDrawCmd *prev_cmd = curr_cmd - 1;
    if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && prev_cmd->UserCallback == NULL)
    {
        CmdBuffer.pop_back();
        return;
    }

    curr_cmd->ClipRect = _CmdHeader.ClipRect;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------
void ImFontAtlas::GetTexDataAsAlpha8(unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
        Build();

    *out_pixels = TexPixelsAlpha8;
    if (out_width)
        *out_width = TexWidth;
    if (out_height)
        *out_height = TexHeight;
    if (out_bytes_per_pixel)
        *out_bytes_per_pixel = 1;
}
void ImFontAtlas::GetTexDataAsRGBA32(unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char *pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int *)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char *src = pixels;
            unsigned int *dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char *)TexPixelsRGBA32;
    if (out_width)
        *out_width = TexWidth;
    if (out_height)
        *out_height = TexHeight;
    if (out_bytes_per_pixel)
        *out_bytes_per_pixel = 4;
}

bool ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");

    //     // Default font is none are specified
    //     if (ConfigData.Size == 0)
    //         AddFontDefault();

    //     // Select builder
    //     // - Note that we do not reassign to atlas->FontBuilderIO, since it is likely to point to static data which
    //     //   may mess with some hot-reloading schemes. If you need to assign to this (for dynamic selection) AND are
    //     //   using a hot-reloading scheme that messes up static data, store your own instance of ImFontBuilderIO somewhere
    //     //   and point to it instead of pointing directly to return value of the GetBuilderXXX functions.
    //     const ImFontBuilderIO *builder_io = FontBuilderIO;
    //     if (builder_io == NULL)
    //     {
    // #ifdef IMGUI_ENABLE_FREETYPE
    //         builder_io = ImGuiFreeType::GetBuilderForFreeType();
    // #elif defined(IMGUI_ENABLE_STB_TRUETYPE)
    //         builder_io = ImFontAtlasGetBuilderForStbTruetype();
    // #else
    //         IM_ASSERT(0); // Invalid Build function
    // #endif
    //     }

    //     // Build
    //     return builder_io->FontBuilder_Build(this);
}