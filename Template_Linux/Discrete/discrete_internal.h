#pragma once
#include "../ImGui/imgui_internal.h"
#include "discrete.h"

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

    // Relation state
    ImVector<DiscreteRelation *> Relations;

    ImVector<ImGuiViewportP *> Viewports;
};

struct DiscreteNode : public wi::scene::TransformComponent
{

    ImGuiViewportP *Viewport;
    DiVec3 Pos;
    DiVec3 Size;

    ImDrawList *DrawList;

public:
    DiscreteNode(DiscreteContext *context);
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
    void Initialize();
    void Shutdown();

    void RenderNode();
} // namespace Discrete
