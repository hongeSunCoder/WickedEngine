//----------
// [SECTION] INCLUDES
//---------
#include "discrete_internal.h"

using namespace wi::graphics;
using namespace wi::scene;
using namespace wi::renderer;

//--------
// [SECTION] FORWARD DECLARATIONS
//--------
static void AddDrawListToDrawData(ImVector<ImDrawList *> *out_list, ImDrawList *draw_list);
namespace Discrete
{
    // Viewports
    static void UpdateViewportsNewFrame();

}

#ifndef GDiscrete
DiscreteContext *GDiscrete = NULL;
#endif

// --------
// [SECTION] INITIALIZATION, SHUTDOWN
// --------
DiscreteContext *Discrete::GetCurrentContext()
{
    return GDiscrete;
}
void Discrete::SetCurrentContext(DiscreteContext *ctx)
{
    GDiscrete = ctx;
}

DiscreteContext *Discrete::CreateContext()
{
    DiscreteContext *ctx = IM_NEW(DiscreteContext);
    SetCurrentContext(ctx);
    Initialize();
    return ctx;
}
void Discrete::DestroyContext(DiscreteContext *ctx)
{
    DiscreteContext *prev_ctx = GetCurrentContext();
    if (ctx == NULL)
        ctx = prev_ctx;
    SetCurrentContext(ctx);
    Shutdown();
    SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
    IM_DELETE(ctx);
}

void Discrete::Initialize()
{
    DiscreteContext &g = *GDiscrete;

    ImGuiViewportP *viewport = (ImGuiViewportP *)ImGui::GetMainViewport();
    g.Viewports.push_back(viewport);
    g.Initialized = true;
}
// To free heap allocations
void Discrete::Shutdown()
{
    DiscreteContext &g = *GDiscrete;
    if (!g.Initialized)
        return;

    // Clear everything else
    g.Nodes.clear_delete();
    g.Relations.clear_delete();
    g.Viewports.clear_delete();

    g.Initialized = false;
}

// -----------------------
// [SECTION] MAIN CODE
// ----------------------
DiscreteNode::DiscreteNode(DiscreteContext *context) : DrawListInst(NULL)
{
    memset(this, 0, sizeof(*this));
    DrawList = &DrawListInst;
    DrawList->_Data = &GImGui->DrawListSharedData;
}

void Discrete::NewFrame()
{
    DiscreteContext &g = *GDiscrete;

    g.FrameCount += 1;
    // UpdateViewportsNewFrame();
}
// current no use
static void Discrete::UpdateViewportsNewFrame()
{
}

static void AddDrawListToDrawData(ImVector<ImDrawList *> *out_list, ImDrawList *draw_list)
{
    if (draw_list->CmdBuffer.Size == 0)
        return;
    if (draw_list->CmdBuffer.Size == 1 && draw_list->CmdBuffer[0].ElemCount == 0 && draw_list->CmdBuffer[0].UserCallback == NULL)
        return;

    // Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc.
    // May trigger for you if you are using PrimXXX functions incorrectly.
    IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
    IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
    if (!(draw_list->Flags & ImDrawListFlags_AllowVtxOffset))
        IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

    if (sizeof(ImDrawIdx) == 2)
        IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

    out_list->push_back(draw_list);
}
// add to regular layer
static void AddNodeToDrawData(DiscreteNode *node)
{

    DiscreteContext &g = *GDiscrete;
    ImGuiViewportP *viewport = g.Viewports[0];
    AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], node->DrawList);
}

static void AddRelationToDrawData(DiscreteRelation *relation)
{
    DiscreteContext &g = *GDiscrete;
    ImGuiViewportP *viewport = g.Viewports[0];
    AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], relation->DrawList);
}
// current no use
void Discrete::EndFrame()
{
}
void Discrete::Render()
{
    DiscreteContext &g = *GDiscrete;

    for (int n = 0; n != g.Nodes.Size; n++)
    {
        DiscreteNode *node = g.Nodes[n];
        AddNodeToDrawData(node);
    }
    for (int n = 0; n != g.Relations.Size; n++)
    {
        DiscreteRelation *relation = g.Relations[n];
        AddRelationToDrawData(relation);
    }
}

static DiscreteNode *CreateNewNode()
{
    DiscreteContext &g = *GDiscrete;

    DiscreteNode *node = IM_NEW(DiscreteNode)(&g);
    g.Nodes.push_back(node);
    return node;
}
void Discrete::Node(DiVec3 &pos, DiVec3 &size)
{
    DiscreteNode *node = CreateNewNode();
    node->Pos = pos;
    node->Size = size;

    node->DrawList->_ResetForNewFrame();

    // DRAWING
    ImU32 col = IM_COL32(255, 255, 255, 255);
    node->DrawList->AddCircleFilled(ImVec2(pos.x, pos.y), size.x, col);
}

// void DiscreteNode::Render(const wi::Canvas &canvas, CommandList cmd) const
// {

//     auto device = GetDevice();
//     auto camera = GetCamera();

//     auto rotation = GetRotation();

//     MiscCB sb;
//     XMStoreFloat4x4(&sb.g_xTransform, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&rotation)) * camera.GetProjection()); // only projection, we will expand in view space on CPU below to be camera facing!
//     sb.g_xColor = XMFLOAT4(1, 1, 1, 1);
//     device->BindDynamicConstantBuffer(sb, CB_GETBINDSLOT(MiscCB), cmd);

//     struct LineSegment
//     {
//         XMFLOAT4 a, colorA, b, colorB;
//     };

//     GraphicsDevice::GPUAllocation mem = device->AllocateGPU(sizeof(LineSegment) * 3, cmd);

//     XMMATRIX V = camera.GetView();

//     RenderablePoint point;

//     // draw
//     int i = 0;

//     LineSegment segment;
//     segment.colorA = segment.colorB = XMFLOAT4(1, 0.5, 1, 1);

//     XMVECTOR _c = XMLoadFloat3(&point.position);
//     _c = XMVector3Transform(_c, V);

//     XMVECTOR _a = _c + XMVectorSet(-1, -1, 0, 0) * point.size;
//     XMVECTOR _b = _c + XMVectorSet(1, 1, 0, 0) * point.size;
//     XMStoreFloat4(&segment.a, _a);
//     XMStoreFloat4(&segment.b, _b);
//     memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
//     i++;

//     _a = _c + XMVectorSet(-1, 1, 0, 0) * point.size;
//     _b = _c + XMVectorSet(1, -1, 0, 0) * point.size;
//     XMStoreFloat4(&segment.a, _a);
//     XMStoreFloat4(&segment.b, _b);
//     memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
//     i++;

//     _a = _c + XMVectorSet(0, 0, 1, 0) * point.size;
//     _b = _c + XMVectorSet(0, 0, -1, 0) * point.size;
//     XMStoreFloat4(&segment.a, _a);
//     XMStoreFloat4(&segment.b, _b);
//     memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
//     i++;

//     const GPUBuffer *vbs[] = {
//         &mem.buffer,
//     };
//     const uint32_t strides[] = {
//         sizeof(XMFLOAT4) + sizeof(XMFLOAT4),
//     };
//     const uint64_t offsets[] = {
//         mem.offset,
//     };
//     device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, offsets, cmd);

//     device->Draw(2 * i, 0, cmd);
// }