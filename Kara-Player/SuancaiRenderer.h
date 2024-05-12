#pragma once

#include"__RendererCommon.h"

#include<d3d11_4.h>
#include<DirectXMath.h>
#include<dcomp.h>

#include<vector>

//#include"../Third-Party/DirectXTK-oct2023/Src/Inc/WICTextureLoader.h"

#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dcomp")

//#ifdef _DEBUG
//#pragma comment(lib, "../Third-Party/DirectXTK-oct2023/Bin/Static/Debug/DirectXTK")
//#else
//#pragma comment(lib, "../Third-Party/DirectXTK-oct2023/Bin/Static/Release/DirectXTK")
//#endif

namespace Suancai {

	namespace Kara {

		namespace Renderer {

			class SuancaiRenderer;

			class GraphicsObject {
			public:
				class ConstantBuffer {
				public:
					DirectX::XMFLOAT4X4 mvp;
					DirectX::XMFLOAT4 screen;
					DirectX::XMFLOAT4 time;
					DirectX::XMFLOAT4 userData;
					DirectX::XMINT4 datai;
				};
				class VertexProp {
				public:
					DirectX::XMFLOAT3 pos;
					DirectX::XMFLOAT4 color;
				};
			protected:
				friend class SuancaiRenderer;
				SuancaiRenderer* pR = nullptr;
				void* pConstantBuffer = nullptr;
				u32 constantBufferBytes = 0;
				void* pVertexBuffer = nullptr;
				u32 vertexCnt = 0;
				ID3D11Buffer* pConstantBufferGPU = nullptr;
				ID3D11Buffer* pVertexBufferGPU = nullptr;
				ID3D11InputLayout* pVertexBufferIL = nullptr;
				ID3D11VertexShader* pVS = nullptr;
				ID3D11PixelShader* pPS = nullptr;
				ID3D11SamplerState* pSS = nullptr;
			public:
				GraphicsObject();
				void init(i32 vertexCnt, void* pCBInitialData, u32 cbBytes, const char8_t* pVSPath, const char8_t* pPSPath, SuancaiRenderer* pRenderer);
				void setConstantBufferBytes(void* pInitialData, u32 bytes);
				u32 getConstantBufferBytes();
				void* getConstantBuffer();
				void setVertexCnt(i32 vertexCnt);
				i32 getVertexCnt();
				void* getVertexBuffer();
				void UpdateConstantBufferToGPU();
				void UpdateVertexBufferToGPU();
				~GraphicsObject();
			protected:
				void setShader(const char8_t* pVSPath, const char8_t* pPSPath);
				void destroyShader();
			};

			class SuancaiRenderer {
			public:
				enum class PrimitiveType : u8 {
					TriangleStrip,
					PointStrip
				};
			protected:
				friend class GraphicsObject;
				std::vector<std::pair<IDXGIAdapter*, DXGI_ADAPTER_DESC>> adaperVec;
				ID3D11Device* pDevice = nullptr;
				ID3D11DeviceContext* pCtx = nullptr;

				ID3D11BlendState* pBaseBlendState = nullptr;

				ID3D11Texture2D* pReadFromRT = nullptr;
				ID3D11RenderTargetView* pReadFromRTV = nullptr;
				ID3D11ShaderResourceView* pReadFromSRV = nullptr;
				ID3D11UnorderedAccessView* pReadFromUAV = nullptr;

				ID3D11Texture2D* pWriteToRT = nullptr;
				ID3D11RenderTargetView* pWriteToRTV = nullptr;
				ID3D11ShaderResourceView* pWriteToSRV = nullptr;
				ID3D11UnorderedAccessView* pWriteToUAV = nullptr;

				ID3D11Texture2D* pReadFromVelRT = nullptr;
				ID3D11ShaderResourceView* pReadFromVelSRV = nullptr;
				ID3D11UnorderedAccessView* pReadFromVelUAV = nullptr;

				ID3D11Texture2D* pWriteToVelRT = nullptr;
				ID3D11ShaderResourceView* pWriteToVelSRV = nullptr;
				ID3D11UnorderedAccessView* pWriteToVelUAV = nullptr;

				// * Compute Shader
				ID3D11ComputeShader* pAdvectForDenCS = nullptr;
				ID3D11ComputeShader* pAdvectForVelCS = nullptr;
				ID3D11ComputeShader* pDiffuseCS = nullptr;
				ID3D11ComputeShader* pProjectCS = nullptr;

				ID3D11Texture2D* pTex = nullptr;
				ID3D11ShaderResourceView* pTexSRV = nullptr;

				HWND wnd = NULL;
				IDXGISwapChain1* pSwapChain1 = nullptr;
				ID3D11Texture2D* pRT = nullptr;
				ID3D11RenderTargetView* pRTV = nullptr;
				D3D11_TEXTURE2D_DESC rtDesc{};
				GraphicsObject* pBlurGO = nullptr;
				GraphicsObject* pCopyGO = nullptr;

				IDCompositionDevice* pCompDevice = nullptr;
				IDCompositionTarget* pCompTarget = nullptr;
				IDCompositionVisual* pCompVisual = nullptr;

				u32 continuousBuzyPresentCnt = 0;
			public:
				SuancaiRenderer();
				/// <summary>
				/// 创建D3D11设备以及相关资源
				/// </summary>
				void init();
				void createGraphicsObject(GraphicsObject** pGO);
				/// <summary>
				/// 渲染到窗口，此函数创建一个交换链（如果有的话清除并重建）
				/// </summary>
				/// <param name="hwnd"></param>
				void connectToWindow(HWND hwnd);
				void beginFullScreen();
				void beginWindowed();
				void resizeSwapChain();
				void clearRT(float r, float g, float b, float a);
				void render(GraphicsObject* pGO, u32 drawVertCnt, PrimitiveType pt);
				void blur(float userDataX = 0.0f, float userDataY = 0.0f, bool blurMainRT = false);
				void stableFluid(float dt, float diff, float visc);
				void copyToRT();
				void PresentRT(UINT syncInterval, UINT flags);
				void switchRT();
				void swap(void* a, void* b);
				void setAlphaBlend(bool isBlend);
				~SuancaiRenderer();
			protected:
				void createDevice();
				void destroyDevice();
				void createSwapChain(HWND hwnd);
				void destroySwapChain();
			};
		}
	}
}