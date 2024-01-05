#include "Example_DiscreteData.h"

using namespace wi::ecs;
using namespace wi::scene;
using namespace wi::graphics;

Shader imguiVS;
Shader imguiPS;
Texture fontTexture;
Sampler sampler;
InputLayout imguiInputLayout;
PipelineState imguiPSO;

Example_DiscreteData::~Example_DiscreteData()
{
}

void Example_DiscreteData::Initialize()
{

    Application::Initialize();

    renderer.init(canvas);
    renderer.Load();

    ActivatePath(&renderer);
}

void Example_DiscreteData::Compose(wi::graphics::CommandList cmd)
{
    Application::Compose(cmd);
}

/**
 * renderPath
 * **/

void Example_DiscreteDataRenderer::LoadShader()
{
    PipelineStateDesc desc;
    desc.vs = wi::renderer::GetShader(wi::enums::VSTYPE_VERTEXCOLOR);
    desc.ps = wi::renderer::GetShader(wi::enums::PSTYPE_VERTEXCOLOR);
    desc.il = wi::renderer::GetInputLayout(wi::enums::ILTYPE_VERTEXCOLOR);
    desc.dss = wi::renderer::GetDepthStencilState(wi::enums::DSSTYPE_DEPTHDISABLED);
    desc.rs = wi::renderer::GetRasterizerState(wi::enums::RSTYPE_DOUBLESIDED);
    desc.bs = wi::renderer::GetBlendState(wi::enums::BSTYPE_TRANSPARENT);
    desc.pt = PrimitiveTopology::LINELIST;
    wi::graphics::GetDevice()->CreatePipelineState(&desc, &pso);
}

void Example_DiscreteDataRenderer::ResizeLayout()
{
    RenderPath3D::ResizeLayout();

    float screenW = GetLogicalWidth();
    float screenH = GetLogicalHeight();
    label.SetPos(XMFLOAT2(screenW / 2.f - label.scale.x / 2.f, screenH * 0.95f));
}

void Example_DiscreteDataRenderer::Render() const
{

    GraphicsDevice *device = wi::graphics::GetDevice();
    CommandList cmd = device->BeginCommandList();

    for (auto d_node in nodes)
    {
        d_node.Render(*this, cmd);
    }

    RenderPath3D::Render();
}

void Example_DiscreteDataRenderer::Load()
{
    setSSREnabled(false);
    setReflectionsEnabled(true);
    setFXAAEnabled(false);

    label.Create("Label1");
    label.SetText("Wicked Engine ImGui integration");
    label.font.params.h_align = wi::font::WIFALIGN_CENTER;
    label.SetSize(XMFLOAT2(240, 20));
    GetGUI().AddWidget(&label);

    // Reset all state that tests might have modified:
    wi::eventhandler::SetVSync(true);
    wi::renderer::SetToDrawGridHelper(false);
    wi::renderer::SetTemporalAAEnabled(true);
    wi::renderer::ClearWorld(wi::scene::GetScene());
    wi::scene::GetScene().weather = WeatherComponent();
    this->ClearSprites();
    this->ClearFonts();
    if (wi::lua::GetLuaState() != nullptr)
    {
        wi::lua::KillProcesses();
    }

    // Reset camera position:
    TransformComponent transform;
    transform.Translate(XMFLOAT3(0, 2.f, -4.5f));
    transform.UpdateTransform();
    wi::scene::GetCamera().TransformCamera(transform);

    // Load model.
    // wi::scene::LoadModel("../Content/models/teapot.wiscene");
    LoadShader();
    RenderPath3D::Load();
}

void Example_DiscreteDataRenderer::Update(float dt)
{

    // Scene &scene = wi::scene::GetScene();
    // // teapot_material Base Base_mesh Top Top_mesh editorLight
    // wi::ecs::Entity e_teapot_base = scene.Entity_FindByName("Base");
    // wi::ecs::Entity e_teapot_top = scene.Entity_FindByName("Top");
    // assert(e_teapot_base != wi::ecs::INVALID_ENTITY);
    // assert(e_teapot_top != wi::ecs::INVALID_ENTITY);
    // TransformComponent *transform_base = scene.transforms.GetComponent(e_teapot_base);
    // TransformComponent *transform_top = scene.transforms.GetComponent(e_teapot_top);
    // assert(transform_base != nullptr);
    // assert(transform_top != nullptr);
    // float rotation = dt;
    // if (wi::input::Down(wi::input::KEYBOARD_BUTTON_LEFT))
    // {
    //     transform_base->Rotate(XMVectorSet(0, rotation, 0, 1));
    //     transform_top->Rotate(XMVectorSet(0, rotation, 0, 1));
    // }
    // else if (wi::input::Down(wi::input::KEYBOARD_BUTTON_RIGHT))
    // {
    //     transform_base->Rotate(XMVectorSet(0, -rotation, 0, 1));
    //     transform_top->Rotate(XMVectorSet(0, -rotation, 0, 1));
    // }

    // wi::renderer::RenderablePoint point;
    // point.color = XMFLOAT4(1, 0.5, 1, 1);
    // point.size = 0.01f;
    // wi::renderer::DrawPoint(point);

    wi::renderer::RenderableLine line_y;
    line_y.end = XMFLOAT3(0, 1, 0);
    wi::renderer::DrawLine(line_y);

    wi::renderer::RenderableLine line_x;
    line_x.end = XMFLOAT3(1, 0, 0);
    wi::renderer::DrawLine(line_x);

    wi::renderer::RenderableLine line_z;
    line_z.end = XMFLOAT3(0, 0, 1);
    line_z.color_start = XMFLOAT4(1, 0.5, 1, 1);
    line_z.color_end = XMFLOAT4(1, 0.5, 1, 1);
    wi::renderer::DrawLine(line_z);

    float rotation = dt;
    if (wi::input::Down(wi::input::KEYBOARD_BUTTON_LEFT))
    {
        for (auto d_node in nodes)
        {
            d_node.Rotate(XMVectorSet(0, rotation, 0, 1));
        }
    }
    else if (wi::input::Down(wi::input::KEYBOARD_BUTTON_RIGHT))
    {
        // d_node.Rotate(XMVectorSet(0, -rotation, 0, 1));
    }

    // d_node.UpdateTransform();

    // wi::renderer::RenderablePoint point1;
    // point1.position = XMFLOAT3(100, 100, 100);
    // point1.color = XMFLOAT4(1, 0.5, 1, 1);
    // wi::renderer::DrawPoint(point1);

    RenderPath3D::Update(dt);
}

void Example_DiscreteDataRenderer::Compose(wi::graphics::CommandList cmd) const
{

    RenderPath3D::Compose(cmd);
}