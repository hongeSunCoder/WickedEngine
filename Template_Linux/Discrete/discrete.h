#pragma once
#include "WickedEngine.h"
#include "../ImGui/imgui.h"
#include "discrete_internal.h"

/**
 *
 *
 */
namespace Discrete
{
    // Context creation and access
    DiscreteContext *CreateContext();
    void DestroyContext(DiscreteContext *ctx = NULL); // NULL = destroy current context
    DiscreteContext *GetCurrentContext();
    void SetCurrentContext(DiscreteContext *ctx);

    // Main
    void NewFrame();
    void EndFrame();
    void Render();
    ImDrawData *GetDrawData();

    // class DiscreteDataManager
    // {
    // public:
    //     inline void AddNode(DiscreteNode node)
    //     {
    //         nodes.push_back(node);
    //     }
    //     inline void AddRelation(DiscreteRelation relation)
    //     {
    //         relations.push_back(relation);
    //     }

    //     inline const wi::vector<DiscreteNode> &GetNodes()
    //     {
    //         return nodes;
    //     }

    // private:
    //     wi::vector<DiscreteNode> nodes;
    //     wi::vector<DiscreteRelation> relations;
    // };

    // inline DiscreteDataManager &GetDataManager()
    // {

    //     static DiscreteDataManager dataManager;
    //     return dataManager;
    // }

} // end namespace Discrete
