#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include <array>
#include "GraphicsThrowMacros.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


Graphics::Graphics( HWND hWnd,int width,int height )
	:
	width( width ),
	height( height )
{
	//
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = width;		
	sd.BufferDesc.Height = height;							//BGRA
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;		//格式是其中通道的像素布局 
	sd.BufferDesc.RefreshRate.Numerator = 0;		//刷新率
	sd.BufferDesc.RefreshRate.Denominator = 0;		//刷新率 不用管
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//Scaling不设置具体的宽高比
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//不设置扫描线顺序
	sd.SampleDesc.Count = 1;		//不需要抗锯齿
	sd.SampleDesc.Quality = 0;		//不需要抗锯齿
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//rendertarget
	sd.BufferCount = 1;			//需要一个后缓存 这样就设置为一
	sd.OutputWindow = hWnd;		//窗口句柄
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	//一般都设置为这样 Swap链的模式
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// for checking results of d3d functions
	HRESULT hr;

	// create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
		nullptr,								//默认视频适配器
		D3D_DRIVER_TYPE_HARDWARE,				//使用硬件加速渲染
		nullptr,								//
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,						//SDK的宏
		&sd,									//指向描述符Descriptor结构的指针
		&pSwap,									//
		&pDevice,								//comptr的&是会把原来指向的接口release掉
		nullptr,
		&pContext
	) );

	// gain access to texture subresource in swap chain (back buffer)
	wrl::ComPtr<ID3D11Texture2D> pBackBuffer;
	//调用交换链对象 pSwap 的 GetBuffer 方法，获取索引为 0 的后台缓冲区（Back Buffer）
	// 并将其接口指针存储到 pBackBuffer 中
	//：通过宏 GFX_THROW_INFO 包裹操作，
	// 用于捕获并处理 GetBuffer 方法可能返回的 Direct3D 错误码，确保错误能被及时检测和处理。
	infoManager.Set(); if ((((HRESULT)(hr = (pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer)))) < 0)) throw Graphics::HrException(74, "D:\\C++文件\\hw3d-master1\\hw3d-master\\hw3d\\Graphics.cpp", (hr), infoManager.GetMessages());
	pTarget = std::shared_ptr<Bind::RenderTarget>{ new Bind::OutputOnlyRenderTarget( *this,pBackBuffer.Get() ) };
	

	// viewport always fullscreen (for now)
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports( 1u,&vp );
	
	// init imgui d3d impl
	ImGui_ImplDX11_Init( pDevice.Get(),pContext.Get() );
}

void Graphics::DrawTestTriangle() {

	pTarget->BindAsBuffer(*this);
	HRESULT hr;
	namespace wrl = Microsoft::WRL;

	struct Vertex {
		float X;
		float Y;
	};

	const Vertex vertices[] = {
		{0.0f, 0.5f},
		{0.5f, -0.5f},
		{-0.5f, -0.5f}
	};//顺时针

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(sizeof(vertices));
	bd.StructureByteStride = UINT(sizeof(Vertex));
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	hr = pDevice->CreateBuffer(&bd, &sd, pVertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("CreateBuffer failed");
	}
	//1.设置RT
	//wrl::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
	//wrl::ComPtr<ID3D11Texture2D> backBuffer;
	/*pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	
	pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &pRenderTargetView);
	pContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);*/
	//pContext->OMSetDepthStencilState(nullptr, 0);
	//2设置IA
	//绑定进入管线

	//3.设置Shaders		
#pragma region Shader绑定
	
	wrl::ComPtr<ID3DBlob> errorBlob;
	wrl::ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	hr = D3DCompileFromFile(
		L"VertexShader.hlsl",			//pFileName 是hlsl文件名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		&pVSBlob,
		&errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA(
				(char*)errorBlob->GetBufferPointer()
			);
		}
		throw std::runtime_error("error:Vertex Shader compile failed");
	}

	hr = D3DCompileFromFile(
		L"PixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		&pPSBlob,
		&errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA(
				(char*)errorBlob->GetBufferPointer()
			);
		}
		throw std::runtime_error("error:Pixel Shader compile failed");
	}
	pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());

	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

#pragma endregion

#pragma region 输入布局
	//
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	pDevice->CreateInputLayout(layout, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	
	const UINT stride = sizeof(Vertex);
	const UINT of = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &of);//设置顶点缓冲区 前面IA是指管线的一个阶段SetVertexBuffers就是设置
	pContext->IASetInputLayout(pInputLayout.Get());
	//图元拓扑 图元渲染是什么
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
#pragma endregion

	//NDC转换到屏幕坐标
	//D3D11_VIEWPORT vp = {};
	//vp.Width = width;
	//vp.Height = height;
	//vp.MinDepth = 0.0f;
	//vp.MaxDepth = 1.0f;
	//vp.TopLeftX = 0;//什么意思：窗口左上角的坐标
	//vp.TopLeftY = 0;//：窗口左上角的坐标
	//pContext->RSSetViewports(1, &vp);

	pContext->Draw(std::size(vertices), 0u);
}

void Graphics::Test() {
	//********************************************************************************
	//千万不要在绑定Comptr时使用&，这会先释放原来Comptr里指向的资源 然后把空资源绑定进去
	// *******************************************************************************
	// 绑定渲染目标（如果 pTarget 已经做了绑定，这行可以保留；否则你需要手动绑定 SwapChain 的 back buffer）
	pTarget->BindAsBuffer(*this);

	namespace wrl = Microsoft::WRL;
#pragma region 结构体
	//这样写结构体使得我们可以任意选择性修改里面的color或者position
	struct Vertex {
		struct
		{
			float X;
			float Y;
		} Position;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} Color;
	};

	Vertex vertices[] =
	{
		{ 0.0f,0.5f,255,0,0,0 },
		{0.5f,-0.5f,0,255,0,0 },
		{ -0.5f,-0.5f,0,0,255,0 },

		{ -0.3f,0.3f,0,255,0,0 },
		{ 0.3f,0.3f,0,255,0,0 },
		{0.0f,-0.8f,255,0,0,0 },
	}; // 顺时针（默认 Direct3D 前向为顺时针/可见，若看不到可改为逆时针或禁用剔除）
	vertices[0].Color.g = 255;

	const unsigned short index[]{
		0,4,3,
		3,4,1,
		3,1,2,
		2,1,5,
	};
#pragma endregion

#pragma region 顶点缓冲与索引缓存
	// 创建顶点缓冲
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(sizeof(vertices));
	bd.StructureByteStride = UINT(sizeof(Vertex));
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	HRESULT hr = pDevice->CreateBuffer(&bd, &sd, pVertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("CreateBuffer failed");
	}
	// 绑定顶点缓冲（注意传入 ID3D11Buffer* 数组）
	ID3D11Buffer* vbs[] = { pVertexBuffer.Get() };
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, vbs, &stride, &offset);

	//创建index buffer
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC id = {};
	id.BindFlags = D3D11_BIND_INDEX_BUFFER;
	id.Usage = D3D11_USAGE_DEFAULT;
	id.CPUAccessFlags = 0u;
	id.MiscFlags = 0u;
	id.ByteWidth = UINT(sizeof(index));
	id.StructureByteStride = UINT(sizeof(unsigned short));
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = index;
	hr = pDevice->CreateBuffer(&id, &isd, pIndexBuffer.GetAddressOf());
	if (FAILED(hr)) throw std::runtime_error("CreateIndexBuffer failed");
	//绑定进入管线
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
#pragma endregion

#pragma region 着色器
	// 编译并创建着色器
	wrl::ComPtr<ID3DBlob> errorBlob;
	wrl::ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;

	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Vertex Shader compile failed");
	}
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		throw std::runtime_error("Pixel Shader compile failed");
	}

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	if (FAILED(hr)) throw std::runtime_error("CreateVertexShader failed");
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());
	if (FAILED(hr)) throw std::runtime_error("CreatePixelShader failed");

	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);
#pragma endregion

#pragma region IA
	// 输入布局（POSITION float2）
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		//8u 是因为R32G32_FLOAT是八位
		//1.语义 2.语义索引如COLOR1就填1 3.数据格式 4.输入槽位 
		// 5.AligendByteOffset对齐字节偏移 表示该数据在缓冲里的偏移量 从第几个字节开始读
		//6.输入槽类型 per vertex还是 per instance 7.实例数据步长
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//format不能用uint因为他会把我们的颜色压成0
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	//1.Layout描述 2.语义数量
	hr = pDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	if (FAILED(hr)) throw std::runtime_error("CreateInputLayout failed");

	pContext->IASetInputLayout(pInputLayout.Get());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 设置视口（确保与目标大小一致）
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)width;   // width 为你的渲染目标宽度成员
	vp.Height = (float)height; // height 为你的渲染目标高度成员
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1, &vp);
#pragma endregion

	// 可选：清除渲染目标（如果 pTarget->BindAsBuffer 没做）
	// float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
	// pContext->ClearRenderTargetView(pYourRTV, clearColor);

	// 绘制
    // DrawIndexed 方法用于绘制使用索引缓冲区的图形。
    // 参数说明：
    // - IndexCount: 要绘制的索引数量，通常是索引数组的大小。
    // - StartIndexLocation: 索引缓冲区中第一个索引的偏移量，通常是0，表示从第一个索引开始绘制。
    // - BaseVertexLocation: 顶点缓冲区中第一个顶点的偏移量，通常是0，表示从第一个顶点开始绘制。
    pContext->DrawIndexed(UINT(std::size(index)), 0u, 0u);
	// Present 通常在外层交换链循环做，这里不处理
}


Graphics::~Graphics()
{
	ImGui_ImplDX11_Shutdown();
}

void Graphics::EndFrame()
{
	// imgui frame end
	if( imguiEnabled )
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	}

	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	if( FAILED( hr = pSwap->Present( 1u,0u ) ) )//设置帧率为设备帧率/1
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw GFX_DEVICE_REMOVED_EXCEPT( pDevice->GetDeviceRemovedReason() );
		}
		else
		{
			throw GFX_EXCEPT( hr );
		}
	}
}


//void Graphics::BeginFrame(float red, float green, float blue) noexcept
//{
//	// imgui begin frame
//	if (imguiEnabled)
//	{
//		ImGui_ImplDX11_NewFrame();
//		ImGui_ImplWin32_NewFrame();
//		ImGui::NewFrame();
//	}
//
//	// ⭐ 清屏颜色（关键）
//	pTarget->Clear(*this);
//
//	// 清空 shader 输入（防止读写冲突）
//	ID3D11ShaderResourceView* const pNullTex = nullptr;
//	pContext->PSSetShaderResources(0, 1, &pNullTex);
//	pContext->PSSetShaderResources(3, 1, &pNullTex);
//}


void Graphics::BeginFrame( float red,float green,float blue ) noexcept
{
	// imgui begin frame
	if( imguiEnabled )
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	// ⭐ 清屏颜色（关键）
	const std::array<float,4> clearColor = { red,green,blue,1.0f };
	//pTarget->Clear(*this,clearColor);
	pTarget->Clear(*this);
	// clearing shader inputs to prevent simultaneous in/out bind carried over from prev frame
	ID3D11ShaderResourceView* const pNullTex = nullptr;
	pContext->PSSetShaderResources( 0,1,&pNullTex ); // fullscreen input texture
	pContext->PSSetShaderResources( 3,1,&pNullTex ); // shadow map texture
}



void Graphics::DrawIndexed( UINT count ) noxnd
{
	GFX_THROW_INFO_ONLY( pContext->DrawIndexed( count,0u,0u ) );
}

void Graphics::SetProjection( DirectX::FXMMATRIX proj ) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

void Graphics::SetCamera( DirectX::FXMMATRIX cam ) noexcept
{
	camera = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept
{
	return camera;
}

void Graphics::EnableImgui() noexcept
{
	imguiEnabled = true;
}

void Graphics::DisableImgui() noexcept
{
	imguiEnabled = false;
}

bool Graphics::IsImguiEnabled() const noexcept
{
	return imguiEnabled;
}

UINT Graphics::GetWidth() const noexcept
{
	return width;
}

UINT Graphics::GetHeight() const noexcept
{
	return height;
}

std::shared_ptr<Bind::RenderTarget> Graphics::GetTarget()
{
	return pTarget;
}


// Graphics exception stuff
Graphics::HrException::HrException( int line,const char* file,HRESULT hr,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file ),
	hr( hr )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if( !info.empty() )
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString( hr );
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription( hr,buf,sizeof( buf ) );
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
Graphics::InfoException::InfoException( int line,const char * file,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}


const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
