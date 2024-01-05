#include "WickedEngine.h"

/**
 * what
 * why
 *
 **/
class DiscreteNode : public wi::scene::TransformComponent
{

public:
    void Render(const wi::Canvas &canvas, wi::graphics::CommandList cmd) const;
};

class DiscreteRelation : public wi::scene::TransformComponent
{

public:
    void Render(const wi::Canvas &canvas, wi::graphics::CommandList cmd) const;
}