#include "discrete.h"

using namespace wi::graphics;
using namespace wi::scene;
using namespace wi::renderer;

void DiscreteNode::Render(const wi::Canvas &canvas, CommandList cmd) const
{

    auto device = GetDevice();
    auto camera = GetCamera();

    auto rotation = GetRotation();

    device->BindPipelineState(&Example_DiscreteDataRenderer::pso, cmd);
    MiscCB sb;
    XMStoreFloat4x4(&sb.g_xTransform, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&rotation)) * camera.GetProjection()); // only projection, we will expand in view space on CPU below to be camera facing!
    sb.g_xColor = XMFLOAT4(1, 1, 1, 1);
    device->BindDynamicConstantBuffer(sb, CB_GETBINDSLOT(MiscCB), cmd);

    struct LineSegment
    {
        XMFLOAT4 a, colorA, b, colorB;
    };

    GraphicsDevice::GPUAllocation mem = device->AllocateGPU(sizeof(LineSegment) * 3, cmd);

    XMMATRIX V = camera.GetView();

    RenderablePoint point;

    // draw
    int i = 0;

    LineSegment segment;
    segment.colorA = segment.colorB = XMFLOAT4(1, 0.5, 1, 1);

    XMVECTOR _c = XMLoadFloat3(&point.position);
    _c = XMVector3Transform(_c, V);

    XMVECTOR _a = _c + XMVectorSet(-1, -1, 0, 0) * point.size;
    XMVECTOR _b = _c + XMVectorSet(1, 1, 0, 0) * point.size;
    XMStoreFloat4(&segment.a, _a);
    XMStoreFloat4(&segment.b, _b);
    memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
    i++;

    _a = _c + XMVectorSet(-1, 1, 0, 0) * point.size;
    _b = _c + XMVectorSet(1, -1, 0, 0) * point.size;
    XMStoreFloat4(&segment.a, _a);
    XMStoreFloat4(&segment.b, _b);
    memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
    i++;

    _a = _c + XMVectorSet(0, 0, 1, 0) * point.size;
    _b = _c + XMVectorSet(0, 0, -1, 0) * point.size;
    XMStoreFloat4(&segment.a, _a);
    XMStoreFloat4(&segment.b, _b);
    memcpy((void *)((size_t)mem.data + i * sizeof(LineSegment)), &segment, sizeof(LineSegment));
    i++;

    const GPUBuffer *vbs[] = {
        &mem.buffer,
    };
    const uint32_t strides[] = {
        sizeof(XMFLOAT4) + sizeof(XMFLOAT4),
    };
    const uint64_t offsets[] = {
        mem.offset,
    };
    device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, offsets, cmd);

    device->Draw(2 * i, 0, cmd);
}