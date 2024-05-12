#include "SuancaiRenderer.h"

Suancai::Kara::Renderer::SuancaiRenderer::SuancaiRenderer() {

}

void Suancai::Kara::Renderer::SuancaiRenderer::init() {

	this->destroyDevice();
	this->createDevice();

	this->pBlurGO = new GraphicsObject();
	this->pBlurGO->init(
		3,
		nullptr,
		sizeof(GraphicsObject::ConstantBuffer),
		#ifdef _DEBUG
		u8"BlurVS.cso",
		u8"BlurPS.cso",
		#else
		u8"BlurVS.cso",
		u8"BlurPS.cso",
		#endif
		this
	);
	GraphicsObject::VertexProp* pVB = (GraphicsObject::VertexProp*)this->pBlurGO->getVertexBuffer();
	pVB[0].pos = {-1.0f, 1.0f, 0.0f};
	pVB[1].pos = {3.0f, 1.0f, 0.0f};
	pVB[2].pos = {-1.0f, -3.0f, 0.0f};
	pVB[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	pVB[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
	pVB[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
	this->pBlurGO->UpdateVertexBufferToGPU();

	this->pCopyGO = new GraphicsObject();
	this->pCopyGO->init(
		3,
		nullptr,
		sizeof(GraphicsObject::ConstantBuffer),
		#ifdef _DEBUG
		u8"CopyVS.cso",
		u8"CopyPS.cso",
		#else
		u8"CopyVS.cso",
		u8"CopyPS.cso",
		#endif
		this
	);
	pVB = (GraphicsObject::VertexProp*)this->pCopyGO->getVertexBuffer();
	pVB[0].pos = {-1.0f, 1.0f, 0.0f};
	pVB[1].pos = {3.0f, 1.0f, 0.0f};
	pVB[2].pos = {-1.0f, -3.0f, 0.0f};
	pVB[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	pVB[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
	pVB[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
	this->pCopyGO->UpdateVertexBufferToGPU();
}

void Suancai::Kara::Renderer::SuancaiRenderer::createGraphicsObject(GraphicsObject** pGO) {

	auto go = new GraphicsObject();
	go->pR = this;
	deref(pGO) = go;
}

void Suancai::Kara::Renderer::SuancaiRenderer::connectToWindow(HWND hwnd) {

	this->destroySwapChain();
	this->createSwapChain(hwnd);
}

void Suancai::Kara::Renderer::SuancaiRenderer::beginFullScreen() {
}

void Suancai::Kara::Renderer::SuancaiRenderer::beginWindowed() {
}

void Suancai::Kara::Renderer::SuancaiRenderer::resizeSwapChain() {

	// * first release back buffer resources
	SAFE_RELEASE(this->pRT);
	SAFE_RELEASE(this->pRTV);
	this->pCtx->Flush();

	HRESULT hr = S_OK;

	// * resize
	RECT rect{};
	GetClientRect(this->wnd, addr(rect));
	hr = this->pSwapChain1->ResizeBuffers(
		0,                   // Number of buffers. Set this to 0 to preserve the existing setting.
		rect.right, 
		rect.bottom,                // Width and height of the swap chain. Set to 0 to match the screen resolution.
		DXGI_FORMAT_UNKNOWN, // This tells DXGI to retain the current back buffer format.
		0
	);
	
	// * configure rtv etc.
	hr = this->pSwapChain1->GetBuffer(0, IID_PPV_ARGS(addr(this->pRT)));
	CHECK_AND_THROW(FAILED(hr), "this->pSwapChain1->GetBuffer(0, IID_PPV_ARGS(addr(pBuffer)))", hr);

	hr = this->pDevice->CreateRenderTargetView(this->pRT, nullptr, addr(this->pRTV));
	CHECK_AND_THROW(FAILED(hr), "this->pDevice->CreateRenderTargetView(pBuffer, nullptr, addr(this->pRTV))", 0);

	SAFE_RELEASE(this->pReadFromRT);
	SAFE_RELEASE(this->pReadFromRTV);
	SAFE_RELEASE(this->pReadFromSRV);
	SAFE_RELEASE(this->pReadFromUAV);

	SAFE_RELEASE(this->pWriteToRT);
	SAFE_RELEASE(this->pWriteToRTV);
	SAFE_RELEASE(this->pWriteToSRV);
	SAFE_RELEASE(this->pWriteToUAV);
	
	SAFE_RELEASE(this->pReadFromVelRT);
	SAFE_RELEASE(this->pReadFromVelSRV);
	SAFE_RELEASE(this->pReadFromVelUAV);
	
	SAFE_RELEASE(this->pWriteToVelRT);
	SAFE_RELEASE(this->pWriteToVelSRV);
	SAFE_RELEASE(this->pWriteToVelUAV);

	// * configure rtv srv
	UINT support = 0;
	this->pDevice->CheckFormatSupport(this->rtDesc.Format, addr(support));
	if ((support & D3D11_FORMAT_SUPPORT::D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) == false) {
		CHECK_AND_THROW(FAILED(hr), "back buffer format dont support mip auto gen", 0);
	}
	this->pRT->GetDesc(addr(this->rtDesc));
	this->rtDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
	this->rtDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	this->rtDesc.MipLevels = 0;
	this->rtDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = rtDesc.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = -1;
	desc.Texture2D.MostDetailedMip = 0;
	// * UAV DESC
	CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2D, this->rtDesc.Format);
	// * create read from RTV SRV
	hr = this->pDevice->CreateTexture2D(
		addr(this->rtDesc),
		nullptr,
		addr(this->pReadFromRT)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rt", 0);
	hr = this->pDevice->CreateRenderTargetView(
		this->pReadFromRT,
		nullptr,
		addr(this->pReadFromRTV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rtv", 0);
	hr = this->pDevice->CreateShaderResourceView(
		this->pReadFromRT,
		addr(desc),
		addr(this->pReadFromSRV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-srv", 0);
	hr = this->pDevice->CreateUnorderedAccessView(
		this->pReadFromRT,
		addr(uavDesc),
		addr(this->pReadFromUAV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-uav", 0);
	// * create write to RTV SRV
	hr = this->pDevice->CreateTexture2D(
		addr(this->rtDesc),
		nullptr,
		addr(this->pWriteToRT)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rt", 0);
	hr = this->pDevice->CreateRenderTargetView(
		this->pWriteToRT,
		nullptr,
		addr(this->pWriteToRTV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rtv", 0);
	hr = this->pDevice->CreateShaderResourceView(
		this->pWriteToRT,
		addr(desc),
		addr(this->pWriteToSRV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-srv", 0);
	hr = this->pDevice->CreateUnorderedAccessView(
		this->pWriteToRT,
		addr(uavDesc),
		addr(this->pWriteToUAV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-uav", 0);
	// * vel
	hr = this->pDevice->CreateTexture2D(
		addr(this->rtDesc),
		nullptr,
		addr(this->pReadFromVelRT)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rt", 0);
	hr = this->pDevice->CreateShaderResourceView(
		this->pReadFromVelRT,
		addr(desc),
		addr(this->pReadFromVelSRV)
	);
	hr = this->pDevice->CreateUnorderedAccessView(
		this->pReadFromVelRT,
		addr(uavDesc),
		addr(this->pReadFromVelUAV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-uav", 0);
	// * 
	CHECK_AND_THROW(FAILED(hr), "create pre-srv", 0);
	hr = this->pDevice->CreateTexture2D(
		addr(this->rtDesc),
		nullptr,
		addr(this->pWriteToVelRT)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-rt", 0);
	hr = this->pDevice->CreateShaderResourceView(
		this->pWriteToVelRT,
		addr(desc),
		addr(this->pWriteToVelSRV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-srv", 0);
	hr = this->pDevice->CreateUnorderedAccessView(
		this->pWriteToVelRT,
		addr(uavDesc),
		addr(this->pWriteToVelUAV)
	);
	CHECK_AND_THROW(FAILED(hr), "create pre-uav", 0);
	// * init shader
	hr = S_OK;

	char szFilePath[MAX_PATH + 1] = {0};
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串//
	std::string path;
	path.clear();
	path.append(szFilePath).append("\\").append((char*)"AdvectForDenCS.cso");

	i32 bufferBytes = 1024 * 1024;
	u8* pShaderBuffer = new u8[bufferBytes]; // 1MB
	FILE* f = nullptr;
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	i32 bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pDevice->CreateComputeShader(pShaderBuffer, bytesRead, nullptr, addr(this->pAdvectForDenCS));
	CHECK_AND_THROW(FAILED(hr), "create AdvectForDenCS", 0);
	///////////////////////////////////
	path.clear();
	path.append(szFilePath).append("\\").append((char*)"AdvectForVelCS.cso");

	bufferBytes = 1024 * 1024;
	pShaderBuffer = new u8[bufferBytes]; // 1MB
	f = nullptr;
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pDevice->CreateComputeShader(pShaderBuffer, bytesRead, nullptr, addr(this->pAdvectForVelCS));
	CHECK_AND_THROW(FAILED(hr), "create AdvectForVelCS", 0);
	///////////////////////////////////
	path.clear();
	path.append(szFilePath).append("\\").append((char*)"DiffuseCS.cso");

	bufferBytes = 1024 * 1024;
	pShaderBuffer = new u8[bufferBytes]; // 1MB
	f = nullptr;
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pDevice->CreateComputeShader(pShaderBuffer, bytesRead, nullptr, addr(this->pDiffuseCS));
	CHECK_AND_THROW(FAILED(hr), "create DiffuseCS", 0);
	//////////////////////////////////
	path.clear();
	path.append(szFilePath).append("\\").append((char*)"ProjectCS.cso");

	bufferBytes = 1024 * 1024;
	pShaderBuffer = new u8[bufferBytes]; // 1MB
	f = nullptr;
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pDevice->CreateComputeShader(pShaderBuffer, bytesRead, nullptr, addr(this->pProjectCS));
	CHECK_AND_THROW(FAILED(hr), "create ProjectCS", 0);
	//WCHAR szFilePath[MAX_PATH + 1] = { 0 };
	//GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
	///*
	//strrchr:函数功能：查找一个字符c在另一个字符串str中末次出现的位置（也就是从str的右侧开始查找字符c首次出现的位置），
	//并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。
	//使用这个地址返回从最后一个字符c到str末尾的字符串。
	//*/
	//std::wstring path = szFilePath;
	//int idx = path.find_last_of(L'\\');
	//path = path.substr(0, idx);
	//path.append(L"\\").append(L"荆棘公主.jpg");
	//ID3D11Resource* pTemp = nullptr;
	//hr = DirectX::CreateWICTextureFromFile(
	//	this->pDevice,
	//	path.c_str(),
	//	addr(pTemp),
	//	addr(this->pTexSRV)
	//);
	//CHECK_AND_THROW(FAILED(hr), "create tecture", 0);
	//hr = pTemp->QueryInterface(IID_PPV_ARGS(addr(this->pTex)));
	//CHECK_AND_THROW(FAILED(hr), "get tex2d interface", 0);

	// * configure view port
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = this->rtDesc.Width;
	vp.Height = this->rtDesc.Height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	this->pCtx->RSSetViewports(1, addr(vp));
}

Suancai::Kara::Renderer::SuancaiRenderer::~SuancaiRenderer() {
}

void Suancai::Kara::Renderer::SuancaiRenderer::copyToRT() {

	GraphicsObject::ConstantBuffer* pCopyCB = (GraphicsObject::ConstantBuffer*)this->pCopyGO->getConstantBuffer();
	pCopyCB->screen.x = this->rtDesc.Width;
	pCopyCB->screen.y = this->rtDesc.Height;
	LARGE_INTEGER freq{};
	QueryPerformanceFrequency(addr(freq));
	LARGE_INTEGER li{};
	QueryPerformanceCounter(addr(li));
	pCopyCB->time.x = float(li.QuadPart) / float(freq.QuadPart);
	this->pCopyGO->UpdateConstantBufferToGPU();

	// * now render to backbuffer
	ID3D11DeviceContext* pCtx = this->pCtx;
	pCtx->OMSetRenderTargets(1, addr(this->pRTV), nullptr);
	UINT stride = sizeof(GraphicsObject::VertexProp), offset = 0;
	pCtx->IASetVertexBuffers(
		0,
		1,
		addr(this->pCopyGO->pVertexBufferGPU),
		addr(stride),
		addr(offset)
	);
	pCtx->IASetInputLayout(this->pCopyGO->pVertexBufferIL);
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCtx->VSSetShader(this->pCopyGO->pVS, nullptr, 0);
	pCtx->VSSetConstantBuffers(0, 1, addr(this->pCopyGO->pConstantBufferGPU));
	pCtx->PSSetShader(this->pCopyGO->pPS, nullptr, 0);
	pCtx->PSSetConstantBuffers(0, 1, addr(this->pCopyGO->pConstantBufferGPU));
	pCtx->PSSetSamplers(0, 1, addr(this->pCopyGO->pSS));
	pCtx->PSSetShaderResources(0, 1, addr(this->pWriteToSRV));
	pCtx->Draw(this->pCopyGO->vertexCnt, 0);
	// * unbind
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pCtx->OMSetRenderTargets(1, addr(pNullRTV), nullptr);
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	pCtx->PSSetShaderResources(0, 1, addr(pNullSRV));
}

void Suancai::Kara::Renderer::SuancaiRenderer::clearRT(float r, float g, float b, float a) {

	float col[4] = {r, g, b, a};
	this->pCtx->OMSetRenderTargets(1, addr(this->pWriteToRTV), nullptr);
	this->pCtx->ClearRenderTargetView(this->pWriteToRTV, col);
	ID3D11RenderTargetView* pNullRTV = nullptr;
	this->pCtx->OMSetRenderTargets(1, addr(pNullRTV), nullptr);
}

void Suancai::Kara::Renderer::SuancaiRenderer::createDevice() {

	// * enumarate adapter
	HRESULT hr = S_OK;
	IDXGIFactory* pDXGIFac = nullptr;
	UINT dxgiFlag = 0;
	#ifdef _DEBUG
	dxgiFlag |= DXGI_CREATE_FACTORY_DEBUG;
	#endif
	hr = CreateDXGIFactory2(dxgiFlag, IID_PPV_ARGS(addr(pDXGIFac)));
	CHECK_AND_THROW(FAILED(hr), "create dxgi fac failed", hr);

	IDXGIAdapter* pAdapter = nullptr;
	hr = S_OK;
	u32 idx = 0;
	while (true) {
		hr = pDXGIFac->EnumAdapters(idx, addr(pAdapter));
		idx++;
		if (hr != S_OK) {
			break;
		}
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(addr(desc));
		this->adaperVec.push_back(std::pair(pAdapter, desc));
	}

	CHECK_AND_THROW(this->adaperVec.size() == 0, "your pc don't have any available gpu", hr);

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL levelTaken = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

	DWORD flags = 0;
	#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	hr = D3D11CreateDevice(
		this->adaperVec.at(1).first,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		flags,
		levels,
		ARRAYSIZE(levels), 
		D3D11_SDK_VERSION,
		addr(this->pDevice),
		addr(levelTaken), 
		addr(this->pCtx)
	);

	if (FAILED(hr)) {
		SUANCAI_BASE_THROW("D3D11CreateDevice failed, switch to another GPU and check your GPU's maximum Direct3D feature level supported and the Kara's minimum feature level required", 0);
	}

	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	desc.FrontCounterClockwise = FALSE;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.SlopeScaledDepthBias = 0;
	desc.DepthClipEnable = FALSE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	ID3D11RasterizerState* pRS = nullptr;
	hr = this->pDevice->CreateRasterizerState(addr(desc), addr(pRS));
	CHECK_AND_THROW(FAILED(hr), "CreateRasterizerState", hr);

	this->pCtx->RSSetState(pRS);

	pRS->Release();

	// * create blend state
	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = this->pDevice->CreateBlendState(addr(bd), addr(this->pBaseBlendState));
	CHECK_AND_THROW(FAILED(hr), "Create BLend State", hr);

	this->pCtx->OMSetBlendState(this->pBaseBlendState, NULL, UINT_MAX);
}

void Suancai::Kara::Renderer::SuancaiRenderer::destroyDevice() {

	SAFE_RELEASE(this->pCtx)
	SAFE_RELEASE(this->pDevice);
}

void Suancai::Kara::Renderer::SuancaiRenderer::createSwapChain(HWND hwnd) {

	this->wnd = hwnd;

	RECT rect{};
	GetClientRect(hwnd, addr(rect));

	DXGI_SWAP_CHAIN_DESC1 desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 4;
	desc.Width = rect.right;
	desc.Height = rect.bottom;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	desc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_PREMULTIPLIED;
	desc.Flags = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	IDXGIDevice* pDxgiDevice = nullptr;
	IDXGIAdapter* pAdapter = nullptr;
	IDXGIFactory2* pDxgiFac2 = nullptr;

	HRESULT hr = this->pDevice->QueryInterface(addr(pDxgiDevice));
	CHECK_AND_THROW(FAILED(hr), "this->pDevice->QueryInterface(addr(pDxgiDevice))", 0);

	hr = pDxgiDevice->GetAdapter(addr(pAdapter));
	CHECK_AND_THROW(FAILED(hr), "pDxgiDevice->GetAdapter(addr(pAdapter))", 0);

	hr = pAdapter->GetParent(IID_PPV_ARGS(addr(pDxgiFac2)));
	CHECK_AND_THROW(FAILED(hr), "pAdapter->GetParent(IID_PPV_ARGS(addr(pDxgiFac2)));", 0);

	/*hr = pDxgiFac->QueryInterface(IID_PPV_ARGS(addr(pDxgiFac2)));
	CHECK_AND_THROW(FAILED(hr), "pDxgiFac1->QueryInterface(IID_PPV_ARGS(addr(pDxgiFac2)))", 0);*/

	hr = DCompositionCreateDevice(pDxgiDevice, IID_PPV_ARGS(addr(this->pCompDevice)));
	CHECK_AND_THROW(FAILED(hr), "create comp device", hr);

	hr = this->pCompDevice->CreateTargetForHwnd(hwnd, FALSE, addr(this->pCompTarget));
	CHECK_AND_THROW(FAILED(hr), "create comp target", hr);

	hr = this->pCompDevice->CreateVisual(addr(this->pCompVisual));
	CHECK_AND_THROW(FAILED(hr), "create comp visual", hr);

	hr = pDxgiFac2->CreateSwapChainForComposition(this->pDevice, addr(desc),  NULL, addr(this->pSwapChain1));
	CHECK_AND_THROW(FAILED(hr), "pDxgiFac2->CreateSwapChainForComposition", hr);

	hr = this->pCompVisual->SetContent(this->pSwapChain1);
	CHECK_AND_THROW(FAILED(hr), "SetContent", hr);

	hr = this->pCompTarget->SetRoot(this->pCompVisual);
	CHECK_AND_THROW(FAILED(hr), "SetRoot", hr);

	pDxgiDevice->Release();
	pAdapter->Release();
	pDxgiFac2->Release();

	// * configure swap chain related resources
	this->resizeSwapChain();
}

void Suancai::Kara::Renderer::SuancaiRenderer::render(GraphicsObject* pGO, u32 drawVertCnt, PrimitiveType pt) {
	
	D3D11_PRIMITIVE_TOPOLOGY topo;
	switch (pt) {
		case PrimitiveType::TriangleStrip:
			topo = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case PrimitiveType::PointStrip:
			topo = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
			break;
		default:
			return;
	}

	ID3D11DeviceContext* pCtx = this->pCtx;
	
	pCtx->OMSetRenderTargets(1, addr(this->pWriteToRTV), nullptr);
	UINT stride = sizeof(GraphicsObject::VertexProp), offset = 0;
	pCtx->IASetVertexBuffers(
		0,
		1,
		addr(pGO->pVertexBufferGPU),
		addr(stride),
		addr(offset)
	);
	pCtx->IASetInputLayout(pGO->pVertexBufferIL);
	pCtx->IASetPrimitiveTopology(topo);
	pCtx->VSSetShader(pGO->pVS, nullptr, 0);
	pCtx->VSSetConstantBuffers(0, 1, addr(pGO->pConstantBufferGPU));
	pCtx->PSSetShader(pGO->pPS, nullptr, 0);
	pCtx->PSSetConstantBuffers(0, 1, addr(pGO->pConstantBufferGPU));
	//pCtx->PSSetSamplers(0, 1, addr(this->pBlurGO->pSS));
	//pCtx->PSSetShaderResources(0, 1, addr(this->pTexSRV));
	pCtx->Draw(drawVertCnt, 0);
	// * unbind views
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pCtx->OMSetRenderTargets(1, addr(pNullRTV), nullptr);
}

void Suancai::Kara::Renderer::SuancaiRenderer::destroySwapChain() {

	SAFE_RELEASE(this->pRTV);
	SAFE_RELEASE(this->pSwapChain1);
}

void Suancai::Kara::Renderer::SuancaiRenderer::PresentRT(UINT syncInterval, UINT flags) {

	// * present
	DXGI_PRESENT_PARAMETERS p;
	p.DirtyRectsCount = 0;
	p.pDirtyRects = NULL;
	p.pScrollOffset = NULL;
	p.pScrollRect = NULL;
	HRESULT hr = this->pSwapChain1->Present1(syncInterval, flags, addr(p));
	this->pCompDevice->Commit();
}

void Suancai::Kara::Renderer::SuancaiRenderer::switchRT() {

	// * switch views

	auto tempRT = this->pReadFromRT;
	this->pReadFromRT = this->pWriteToRT;
	this->pWriteToRT = tempRT;
	tempRT = this->pReadFromVelRT;
	this->pReadFromVelRT = this->pWriteToVelRT;
	this->pWriteToVelRT = tempRT;

	auto tempRTV = this->pReadFromRTV;
	this->pReadFromRTV = this->pWriteToRTV;
	this->pWriteToRTV = tempRTV;

	auto tempSRV = this->pReadFromSRV;
	this->pReadFromSRV = this->pWriteToSRV;
	this->pWriteToSRV = tempSRV;
	tempSRV = this->pReadFromVelSRV;
	this->pReadFromVelSRV = this->pWriteToVelSRV;
	this->pWriteToVelSRV = tempSRV;

	auto tempUAV = this->pReadFromUAV;
	this->pReadFromUAV = this->pWriteToUAV;
	this->pWriteToUAV = tempUAV;
	tempUAV = this->pReadFromVelUAV;
	this->pReadFromVelUAV = this->pWriteToVelUAV;
	this->pWriteToVelUAV = tempUAV;
}

void Suancai::Kara::Renderer::SuancaiRenderer::swap(void* a, void* b) {

	auto tempRT = this->pReadFromRT;
	this->pReadFromRT = this->pWriteToRT;
	this->pWriteToRT = tempRT;
	tempRT = this->pReadFromVelRT;
	this->pReadFromVelRT = this->pWriteToVelRT;
	this->pWriteToVelRT = tempRT;

	auto tempRTV = this->pReadFromRTV;
	this->pReadFromRTV = this->pWriteToRTV;
	this->pWriteToRTV = tempRTV;

	auto tempSRV = this->pReadFromSRV;
	this->pReadFromSRV = this->pWriteToSRV;
	this->pWriteToSRV = tempSRV;
	tempSRV = this->pReadFromVelSRV;
	this->pReadFromVelSRV = this->pWriteToVelSRV;
	this->pWriteToVelSRV = tempSRV;

	auto tempUAV = this->pReadFromUAV;
	this->pReadFromUAV = this->pWriteToUAV;
	this->pWriteToUAV = tempUAV;
	tempUAV = this->pReadFromVelUAV;
	this->pReadFromVelUAV = this->pWriteToVelUAV;
	this->pWriteToVelUAV = tempUAV;
}

void Suancai::Kara::Renderer::SuancaiRenderer::setAlphaBlend(bool isBlend) {

	if (isBlend) {
		this->pCtx->OMSetBlendState(this->pBaseBlendState, NULL, UINT_MAX);
	} else {
		this->pCtx->OMSetBlendState(NULL, NULL, UINT_MAX);
	}
}

void Suancai::Kara::Renderer::SuancaiRenderer::blur(float userDataX, float userDataY, bool blurMainRT) {

	GraphicsObject::ConstantBuffer* pBlurCB = (GraphicsObject::ConstantBuffer*)this->pBlurGO->getConstantBuffer();

	pBlurCB->screen.x = this->rtDesc.Width;
	pBlurCB->screen.y = this->rtDesc.Height;

	LARGE_INTEGER freq{};
	QueryPerformanceFrequency(addr(freq));
	LARGE_INTEGER li{};
	QueryPerformanceCounter(addr(li));
	pBlurCB->time.x = float(li.QuadPart) / float(freq.QuadPart);

	pBlurCB->userData.x = userDataX;
	pBlurCB->userData.y = userDataY;

	this->pBlurGO->UpdateConstantBufferToGPU();

	// * now render
	this->switchRT();
	
	ID3D11DeviceContext* pCtx = this->pCtx;

	pCtx->GenerateMips(this->pReadFromSRV);

	pCtx->OMSetRenderTargets(1, addr(this->pWriteToRTV), nullptr);
	UINT stride = sizeof(GraphicsObject::VertexProp), offset = 0;
	pCtx->IASetVertexBuffers(
		0,
		1,
		addr(this->pBlurGO->pVertexBufferGPU),
		addr(stride),
		addr(offset)
	);
	pCtx->IASetInputLayout(this->pBlurGO->pVertexBufferIL);
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCtx->VSSetShader(this->pBlurGO->pVS, nullptr, 0);
	pCtx->VSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	pCtx->PSSetShader(this->pBlurGO->pPS, nullptr, 0);
	pCtx->PSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	pCtx->PSSetSamplers(0, 1, addr(this->pBlurGO->pSS));
	pCtx->PSSetShaderResources(0, 1, addr(this->pReadFromSRV));
	pCtx->Draw(this->pBlurGO->vertexCnt, 0);
	// * unbind
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pCtx->OMSetRenderTargets(1, addr(pNullRTV), nullptr);
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	pCtx->PSSetShaderResources(0, 1, addr(pNullSRV));
}

void Suancai::Kara::Renderer::SuancaiRenderer::stableFluid(float dt, float diff, float visc) {
	
	GraphicsObject::ConstantBuffer* pBlurCB = (GraphicsObject::ConstantBuffer*)this->pBlurGO->getConstantBuffer();

	pBlurCB->screen.x = this->rtDesc.Width;
	pBlurCB->screen.y = this->rtDesc.Height;

	LARGE_INTEGER freq{};
	QueryPerformanceFrequency(addr(freq));
	LARGE_INTEGER li{};
	QueryPerformanceCounter(addr(li));
	pBlurCB->time.x = float(li.QuadPart) / float(freq.QuadPart);

	ID3D11UnorderedAccessView* p = nullptr;

	// * Vel Step

	//pBlurCB->userData.x = dt;
	//pBlurCB->userData.y = visc;
	//pBlurCB->datai.x = 64;
	//this->pBlurGO->UpdateConstantBufferToGPU();

	//// Diffuse
	//this->switchRT();
	//this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pReadFromVelUAV), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pWriteToVelUAV), nullptr);
	//this->pCtx->CSSetShader(this->pDiffuseCS, nullptr, 0);
	//this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);

	//// Project
	//this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pWriteToVelUAV), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pReadFromVelUAV), nullptr);
	//this->pCtx->CSSetShader(this->pProjectCS, nullptr, 0);
	//this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);

	//// Advect
	//this->switchRT();
	//this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pReadFromVelUAV), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pWriteToVelUAV), nullptr);
	//this->pCtx->CSSetShader(this->pAdvectForVelCS, nullptr, 0);
	//this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);

	//// Project
	//this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pWriteToVelUAV), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pReadFromVelUAV), nullptr);
	//this->pCtx->CSSetShader(this->pProjectCS, nullptr, 0);
	//this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
	//this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
	//this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);

	// * Den Step

	pBlurCB->userData.x = dt;
	pBlurCB->userData.y = diff;
	pBlurCB->datai.x = 64;
	this->pBlurGO->UpdateConstantBufferToGPU();

	//// Advect
	this->switchRT();
	this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
	this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pReadFromUAV), nullptr);
	this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pWriteToUAV), nullptr);
	this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(this->pWriteToVelUAV), nullptr);
	this->pCtx->CSSetShader(this->pAdvectForDenCS, nullptr, 0);
	this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
	this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
	this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
	this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);

	// Diffuse
	for (int i = 0; i < 64; i++) {
		this->switchRT();
		this->pCtx->CSSetConstantBuffers(0, 1, addr(this->pBlurGO->pConstantBufferGPU));
		this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(this->pReadFromUAV), nullptr);
		this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(this->pWriteToUAV), nullptr);
		this->pCtx->CSSetShader(this->pDiffuseCS, nullptr, 0);
		this->pCtx->Dispatch((this->rtDesc.Width + 32 - 1) / 32.0f, (this->rtDesc.Height + 32 - 1) / 32.0f, 1);
		this->pCtx->CSSetUnorderedAccessViews(0, 1, addr(p), nullptr);
		this->pCtx->CSSetUnorderedAccessViews(1, 1, addr(p), nullptr);
		this->pCtx->CSSetUnorderedAccessViews(2, 1, addr(p), nullptr);
	}
}

Suancai::Kara::Renderer::GraphicsObject::GraphicsObject() {

}

void Suancai::Kara::Renderer::GraphicsObject::init(i32 vertexCnt, void* pCBInitialData, u32 cbBytes,const char8_t* pVSPath, const char8_t* pPSPath, SuancaiRenderer* pRenderer) {

	this->pR = pRenderer;
	this->setVertexCnt(vertexCnt);
	this->setConstantBufferBytes(pCBInitialData, cbBytes);
	this->setShader(pVSPath, pPSPath);

	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MipLODBias = 0;
	desc.MaxAnisotropy = 0;
	desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	desc.BorderColor[0] = 0; desc.BorderColor[1] = 0; desc.BorderColor[2] = 0; desc.BorderColor[3] = 0;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = this->pR->pDevice->CreateSamplerState(addr(desc), addr(this->pSS));
	CHECK_AND_THROW(FAILED(hr), "create sampler state", hr);
}

void Suancai::Kara::Renderer::GraphicsObject::setConstantBufferBytes(void* pInitialData, u32 bytes) {

	sdela(this->pConstantBuffer);
	this->pConstantBuffer = new u8[bytes];
	this->constantBufferBytes = bytes;
	if (pInitialData != nullptr) {
		CopyMemory(this->pConstantBuffer, pInitialData, bytes);
	}

	//创建常量缓冲区
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	HRESULT hr = this->pR->pDevice->CreateBuffer(
		addr(cbDesc),
		nullptr,
		addr(this->pConstantBufferGPU)
	);
	CHECK_AND_THROW(FAILED(hr), "CreateConstantBuffer", hr);
}

Suancai::u32 Suancai::Kara::Renderer::GraphicsObject::getConstantBufferBytes() {

	return this->constantBufferBytes;
}

void* Suancai::Kara::Renderer::GraphicsObject::getConstantBuffer() {

	return this->pConstantBuffer;
}

void Suancai::Kara::Renderer::GraphicsObject::setVertexCnt(i32 vertexCnt) {

	this->vertexCnt = vertexCnt;
	this->pVertexBuffer = new VertexProp[vertexCnt];
	ZeroMemory(this->pVertexBuffer, sizeof(VertexProp) * vertexCnt);
	//创建顶点缓冲
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = sizeof(VertexProp) * vertexCnt;
	vbDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	vbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = this->pVertexBuffer;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	HRESULT hr = this->pR->pDevice->CreateBuffer(
		addr(vbDesc),
		addr(initData),
		addr(this->pVertexBufferGPU)
	);
	CHECK_AND_THROW(FAILED(hr), "CreateVertexBuffer", hr);
}

Suancai::i32 Suancai::Kara::Renderer::GraphicsObject::getVertexCnt() {

	return this->vertexCnt;
}

void* Suancai::Kara::Renderer::GraphicsObject::getVertexBuffer() {

	return this->pVertexBuffer;
}

void Suancai::Kara::Renderer::GraphicsObject::UpdateConstantBufferToGPU() {

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = this->pR->pCtx->Map(
		this->pConstantBufferGPU,
		0,
		D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
		0,
		addr(mapped)
	);
	CHECK_AND_THROW(FAILED(hr), "MapConstantBuffer", hr);
	//更新数据 
	CopyMemory(mapped.pData, this->pConstantBuffer, this->constantBufferBytes);
	this->pR->pCtx->Unmap(this->pConstantBufferGPU, 0);
}

void Suancai::Kara::Renderer::GraphicsObject::UpdateVertexBufferToGPU() {

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = this->pR->pCtx->Map(
		this->pVertexBufferGPU,
		0,
		D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
		0,
		addr(mapped)
	);
	CHECK_AND_THROW(FAILED(hr), "MapVertexBuffer", hr)
	//更新数据
	CopyMemory(mapped.pData, this->pVertexBuffer, sizeof(VertexProp) * this->vertexCnt);
	this->pR->pCtx->Unmap(this->pVertexBufferGPU, 0);
}

Suancai::Kara::Renderer::GraphicsObject::~GraphicsObject() {
}

void Suancai::Kara::Renderer::GraphicsObject::setShader(const char8_t* pVSPath, const char8_t* pPSPath) {

	HRESULT hr = S_OK;

	char szFilePath[MAX_PATH + 1] = {0};
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	/*
	strrchr:函数功能：查找一个字符c在另一个字符串str中末次出现的位置（也就是从str的右侧开始查找字符c首次出现的位置），
	并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。
	使用这个地址返回从最后一个字符c到str末尾的字符串。
	*/
	(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串//
	std::string path = szFilePath;
	path.append("\\").append((char*)pVSPath);

	i32 bufferBytes = 1024 * 1024;
	u8* pShaderBuffer = new u8[bufferBytes]; // 1MB
	FILE* f = nullptr;
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	i32 bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pR->pDevice->CreateVertexShader(
		pShaderBuffer,
		bytesRead,
		nullptr,
		addr(this->pVS)
	);
	CHECK_AND_THROW(FAILED(hr), "CreateVertexShader", hr);

	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{
			"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};

	hr = this->pR->pDevice->CreateInputLayout(
		desc,
		ARRAYSIZE(desc),
		pShaderBuffer,
		bytesRead,
		addr(this->pVertexBufferIL)
	);
	CHECK_AND_THROW(FAILED(hr), "CreateInputLayout", hr);

	path = szFilePath;
	path.append("\\").append((char*)pPSPath);
	fopen_s(addr(f), (char*)path.c_str(), "rb");
	bytesRead = fread_s(pShaderBuffer, bufferBytes, 1, bufferBytes, f);
	fclose(f);
	hr = this->pR->pDevice->CreatePixelShader(
		pShaderBuffer,
		bytesRead,
		nullptr,
		addr(this->pPS)
	);
	CHECK_AND_THROW(FAILED(hr), "CreatePixelShader", hr);

	sdela(pShaderBuffer);
}

void Suancai::Kara::Renderer::GraphicsObject::destroyShader() {
}
