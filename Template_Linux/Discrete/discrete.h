#pragma once
#include "WickedEngine.h"

//--------
// [SECTION] Forward declarations and basic types
//------
//
struct DiscreteContext;

struct DiVec3
{
    float x, y, z;
    constexpr DiVec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr DiVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

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

    //

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

//-------
// [SECTION] Helpers  (DiDrawVert)
//-------
