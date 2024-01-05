#pragma once
#include "WickedEngine.h"
#include "Discrete_Data/discrete.h"

class Example_DiscreteDataRenderer : public wi::RenderPath3D
{

private:
    wi::vector<DiscreteNode *> nodes;
    void LoadShader();

public:
    wi::gui::Label label;
    wi::graphics::PipelineState pso;
    void Load() override;
    void Update(float dt) override;
    void ResizeLayout() override;
    void Render() const override;
    void Compose(wi::graphics::CommandList cmd) const override;
};

class Example_DiscreteData : public wi::Application
{
    Example_DiscreteDataRenderer renderer;

public:
    ~Example_DiscreteData() override;
    void Initialize() override;
    void Compose(wi::graphics::CommandList cmd) override;
};