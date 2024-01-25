#pragma once
#include "../ImGui/imgui_internal.h"
#include "discrete.h"
#include "WickedEngine.h"

//-------
// [SECTION] forward declarations
//-------
class DiscreteNode;
class DiscreteRelation;

struct DiscreteContext
{
    bool Initialized;
    int FrameCount;

    // Nodes state
    ImVector<DiscreteNode *> Nodes;
    ImGuiStorage NodesById; // map node's ImGuiID to ImGuiWindow*

    // Relation state
    ImVector<DiscreteRelation *> Relations;

    ImVector<ImGuiViewportP *> Viewports;
};

struct DiscreteNode : public wi::scene::TransformComponent
{
    char *Name;
    ImGuiID ID;

    ImGuiViewportP *Viewport;
    DiVec3 Pos;
    DiVec3 Size;

    ImDrawList *DrawList;
    ImDrawList DrawListInst;

public:
    DiscreteNode(DiscreteContext *context, const char *name);
    //  void Render(const wi::Canvas &canvas, wi::graphics::CommandList cmd) const;
};

struct DiscreteRelation : public wi::scene::TransformComponent
{

    ImGuiViewportP *Viewport;

    ImDrawList *DrawList;

public:
    DiscreteRelation(DiscreteContext *context);
    //     void Render(const wi::Canvas &canvas, wi::graphics::CommandList cmd) const;
};

namespace Discrete
{

    DiscreteNode *FindNodeByID(ImGuiID id);
    DiscreteNode *FindNodeByName(const char *name);

    void Initialize();
    void Shutdown();

    void RenderNode();
} // namespace Discrete
