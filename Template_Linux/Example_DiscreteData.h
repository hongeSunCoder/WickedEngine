#pragma once
#include "WickedEngine.h"

class Example_DiscreteDataRenderer : public wi::RenderPath3D
{
    wi::gui::Label label;

public:
    void Load() override;
    void Update(float dt) override;
    void ResizeLayout() override;
    void Render() const override;
    // void Compose(wi::graphics::CommandList cmd) const override;
};

class Example_DiscreteData : public wi::Application
{
    Example_DiscreteDataRenderer renderer;

private:
public:
    ~Example_DiscreteData() override;
    void Initialize() override;
    void Compose(wi::graphics::CommandList cmd) override;
};