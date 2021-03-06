#include "renderer.h"

#include "mesh.h"

D3D11_PRIMITIVE_TOPOLOGY translate(DrawMode mode)
{
    return static_cast<D3D11_PRIMITIVE_TOPOLOGY>(mode);
}

Renderer::Renderer(const gui::Window& window)
    : device_()
    , swapchain_()
    , context_()
    , target_()
    , shader_(nullptr)
    , vertexCount_(0u)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = window.handle();
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT swapCreateFlags = 0u;
#ifdef DEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // create device and front/back buffers, and swap chain and rendering context
    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        swapCreateFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &swapchain_,
        &device_,
        nullptr,
        &context_
    );
    // gain access to texture subresource in swap chain (back buffer)
    Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
    swapchain_->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
    device_->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &target_);

    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(window.width());
    vp.Height = static_cast<FLOAT>(window.height());
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context_->RSSetViewports(1u, &vp);
}

void Renderer::bindVertexShader(VertexShader& shader)
{
    shader_ = &shader;
    shader.bind(device_.Get(), context_.Get());
}

void Renderer::bindPixelShader(PixelShader& shader)
{
    shader.bind(device_.Get(), context_.Get());
}

void Renderer::bindCamera(Camera& camera)
{
    camera.bind(device_.Get(), context_.Get());
}

void Renderer::beginFrame(const Color& color)
{
    context_->ClearRenderTargetView(target_.Get(), color.data);
}

void Renderer::draw(DrawMode mode)
{
    context_->OMSetRenderTargets(1u, target_.GetAddressOf(), nullptr);

    D3D11_PRIMITIVE_TOPOLOGY topology = translate(mode);

    context_->IASetPrimitiveTopology(topology);
    context_->Draw((UINT)vertexCount_, 0u);

    vertexCount_ = 0;
}

void Renderer::draw(Mesh& mesh)
{
    mesh.draw(*this);
}

void Renderer::endFrame()
{
    swapchain_->Present(1u, 0u);

    ConstantBuffer<int>::clearBuffers();
}

