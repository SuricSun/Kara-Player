#include<Windows.h>

#include"BaseException.h"
#include"AudioDecoder.h"
#include"AudioRenderer.h"
#include"AudioEnumerator.h"
#include"SuancaiRenderer.h"
#include"FFT.h"
#include"PathUtil.h"
#include"KaraPlayer.h"

#include<process.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>

#pragma comment(lib, "d3d11")

using namespace Suancai::Kara::Renderer;
using namespace Suancai::Util;

Suancai::Media::Audio::AudioDecoder* pDec = nullptr;
Suancai::Media::Audio::AudioRenderer* pAR = nullptr;

volatile float coeff = 0;
volatile float amplifier = 32;

//float CoordMap(float val, float strength) {
//
//    // * for now we just don't do any mapping
//    //return val;
//    return ((strength + 1.0f) / strength) * (-1.0f / (strength * val + 1.0f) + 1.0f);;
//}

//unsigned __stdcall DecodeThread(void* pArg) {
//
//    Suancai::Media::Audio::AudioRenderer** pR = (Suancai::Media::Audio::AudioRenderer**)pArg;
//
//    while (pAR == nullptr);
//
//    try {
//
//        WNDCLASS wc = { };
//
//        wc.lpfnWndProc = DefWindowProcW;
//        wc.hInstance = GetModuleHandleW(NULL);
//        wc.lpszClassName = L"Kara Vifft";
//        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
//
//        RegisterClassW(&wc);
//
//        int screenX = GetSystemMetrics(SM_CXSCREEN);
//        int screenY = GetSystemMetrics(SM_CYSCREEN);
//        int edgeLen = 1080;
//
//        // Create the window.
//        HWND hwnd = CreateWindowExW(
//            // * layered and transparent make the window click-through-able
//            WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOPMOST,								// Optional window styles.
//            L"Kara Vifft",
//            L"Kara Vifft",
//            WS_POPUP | WS_DISABLED,
//            0, 0, 1920, 1080,
//            NULL,
//            NULL,
//            GetModuleHandleW(NULL),
//            NULL
//        );
//
//        CHECK_AND_THROW(hwnd == NULL, "无法创建窗口", 0);
//
//        ShowWindow(hwnd, SW_SHOW);
//
//        i32 fftSize = 4096;
//
//        float* pSamples = pDec->allSamples.data();
//        float* pWindowedSamples = new float[fftSize];
//
//        SuancaiRenderer renderer;
//        renderer.init();
//        renderer.connectToWindow(hwnd);
//        GraphicsObject* pGO;
//        renderer.createGraphicsObject(addr(pGO));
//        pGO->init(
//            (8192) * 2,
//            nullptr,
//            sizeof(GraphicsObject::ConstantBuffer),
//            #ifdef _DEBUG
//            u8"BaseVS.cso",
//            u8"BasePS.cso",
//            #else
//            u8"SimpleVS.cso",
//            u8"SimplePS.cso",
//            #endif
//            addr(renderer)
//        );
//        GraphicsObject::ConstantBuffer* pGOConstantBuffer = nullptr;
//        pGOConstantBuffer = (GraphicsObject::ConstantBuffer*)pGO->getConstantBuffer();
//        pGOConstantBuffer->screen.x = screenX;
//        pGOConstantBuffer->screen.y = screenY;
//        GraphicsObject::VertexProp* pVertexBuffer = (GraphicsObject::VertexProp*)pGO->getVertexBuffer();
//
//
//        Suancai::Util::FFT fft;
//        fft.init(fftSize);
//
//        u32 curIdx = 0;
//
//        MSG msg;
//        bool shouldExit = false;
//        while (true) {
//            while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE) != 0) {
//                if (msg.message == WM_QUIT) {
//                    shouldExit = true;
//                    break;
//                }
//                TranslateMessage(&msg);
//                DispatchMessageW(&msg);
//            }
//            if (shouldExit) {
//                break;
//            }
//            // * forward buffer
//            auto pos = pAR->getDevicePosition();
//            double t = double(pos.first) / pos.second;
//            curIdx = floor(t * 44100.0);
//            //curIdx -= (fftSize / 2);
//            if (curIdx < 0) {
//                curIdx = 0;
//            }
//            // * windowing
//            for (int i = 0; i < fftSize; i++) {
//                pWindowedSamples[i] =
//                    pow((0.5f - (1.0f - 0.5f) * cos((2.0f * PI_F * i) / float((fftSize - 1)))), 1) * (pSamples[curIdx + i] - pSamples[curIdx + i - 1] * coeff);
//            }
//            // * fft
//            fft.doFFT(pWindowedSamples, fftSize, 0);
//            // * Draw
//            FFT::Complex* pCurFFTBin = fft.p_result1_final;
//            i32 binSize = fftSize / 2;
//            i64 fftVertexPairCnt = binSize;
//            float freq = 0;
//            float maxMapped = floor(2595.0f * log10f(1.0f + binSize / 700.0f));
//            float h = 0, s = 1, v = 1, r = 0, g = 0, b = 0;
//            for (int i = 0; i < binSize; i++) {
//                freq = float(i) * (44100.0f / binSize);
//                float mappednormalizedPos = CoordMap(float(i) / (binSize - 1), 32);
//                pVertexBuffer[2 * i].pos = {mappednormalizedPos,0,0};
//                pVertexBuffer[2 * i + 1].pos = {mappednormalizedPos, pCurFFTBin[i].real / amplifier + 4.0f/1080, 0};
//                h = float(i) / binSize;
//                ImGui::ColorConvertHSVtoRGB(mappednormalizedPos * (0.85 - 0.7) + 0.7, 1, 1, r, g, b);
//                pVertexBuffer[2 * i].color = {r,g,b,1};
//                pVertexBuffer[2 * i + 1].color = {r,g,b,1};
//            }
//    //        float binSizeF = binSize;
//    //        float xMappingStrenth = 32;
//    //        
//    //        i64 fftIdx = 1;
//    //        i64 lastProcessIdx = 0;
//    //        i32 windCnt = 1;
//    //        i32 curTime = 0;
//    //        float circleRadius = 0.2;
//    //        {
//				////pCurFFTBin[0].real = 0;
//				//bool shouldQuit = false;
//				//for (fftIdx = 1; fftIdx < binSize; fftIdx++) {
//				//	float sX = CoordMap(((fftIdx == 0) ? 0 : (fftIdx - 1.0f)) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (((fftIdx == 0) ? 0 : (fftIdx - 1.0f)) / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//	float mX = CoordMap((fftIdx) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * ((fftIdx) / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//	float eX = CoordMap(((fftIdx == binSize) ? binSize : (fftIdx + 1.0f)) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (((fftIdx == binSize) ? binSize : (fftIdx + 1.0f)) / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//	//映射到顶点索引坐标系
//				//	sX *= (fftVertexPairCnt - 1);
//				//	mX *= (fftVertexPairCnt - 1);
//				//	eX *= (fftVertexPairCnt - 1);
//				//	//获取中点
//				//	sX = (fftIdx == 1) ? sX : (sX + mX) / 2.0f;
//				//	eX = (mX + eX) / 2.0f;
//				//	//求出贝塞尔中点
//				//	float rMX = ((sX + mX) / 2.0f + (mX + eX) / 2.0f) / 2.0f;// real mX
//				//	//判断sX和eX的距离，大于1且至少包含一个贝塞尔点，否则停止贝塞尔平滑并跳出
//				//	i32 start = std::ceil(sX);
//				//	i32 end = std::floor(eX);
//				//	if (!((eX - sX) > 1.0f && (start <= end))) {
//				//		end = start;
//				//		shouldQuit = true;
//				//		//break;
//				//	}
//				//	lastProcessIdx = end;
//				//	//获取高度
//				//	float sY = pCurFFTBin[fftIdx - 1].real / amplifier;
//				//	float mY = pCurFFTBin[fftIdx].real / amplifier;
//				//	float eY = pCurFFTBin[fftIdx + 1].real / amplifier;
//				//	//获取中点
//				//	sY = (fftIdx == 1) ? sY : (sY + mY) / 2.0f;
//				//	eY = (mY + eY) / 2.0f;
//				//	//开始贝塞尔插值
//				//	float x = 0.0f;
//				//	//(s-2m+l)x^2 + (2m-2s)x + s = start
//				//	//(s-2m+l)x^2 + (2m-2s)x + s - start = 0
//				//	for (u32 i = start; i <= end; i++) {
//				//		if (i < rMX) {
//				//			x = (i - sX) / (rMX - sX) / 2.0f;
//				//		} else {
//				//			x = 0.5f + (i - rMX) / (eX - rMX) / 2.0f;
//				//		}
//				//		//x = (i - float(start)) / (end - float(start));
//				//		x = ((sY - 2.0f * mY + eY) * pow(x, 2.0f) + 2.0f * (mY - sY) * x + sY);
//				//		//x = (toDB(x) + 75) / 200;
//				//		if (x < 0)x = 0;
//				//		//x = NeverReach(x, specMaxHeight);
//				//		//x *= (1.0f - ((xMappingStrenth + 1.0f) / xMappingStrenth) * (-1.0f / (xMappingStrenth * ((fftIdx) / binSizeF) + 1.0f) + 1.0f));
//				//		float circleX = cos(((i / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
//				//		float circleY = -sin(((i / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
//				//		//pVertexBuffer[i * 2 + 0].pos.x = float(i) / fftVertexPairCnt;
//				//		if (x <= 0)x = 0;
//				//		float a = 0.1;
//				//		float b = 10;
//				//		float t = float(i) / fftVertexPairCnt;
//				//		float xamp = pow(a - a * t, 3) * b + 1;
//				//		//x *= (1 + pow(t, 3));
//    //                    pVertexBuffer[2 * i].pos = {t,0,0};
//    //                    pVertexBuffer[2 * i + 1].pos = {t,x + 4.0f/1080, 0};
//    //                    ImGui::ColorConvertHSVtoRGB(t* (0.85 - 0.7) + 0.7, 1, 1, r, g, b);
//    //                    pVertexBuffer[2 * i].color = {r,g,b,1};
//    //                    pVertexBuffer[2 * i + 1].color = {r,g,b,1};
//				//	}
//				//	if (shouldQuit) {
//				//		break;
//				//	}
//				//}
//				////继续处理不需要贝塞尔的剩余fft数据
//				//float sum = pCurFFTBin[fftIdx].real;
//				//i32 cnt = 1;
//				//float begginingSX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//begginingSX *= (fftVertexPairCnt - 1);
//				//while (true) {
//				//	fftIdx++;
//				//	if (fftIdx >= binSize) {
//				//		break;
//				//	}
//				//	float sX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//	sX *= (fftVertexPairCnt - 1);
//				//	//判断当前sX到beginningSX之间有至少一个整数
//				//	i32 start = std::ceil(begginingSX);
//				//	i32 end = std::floor(sX);
//				//	if (start > end) {
//				//		sum += pCurFFTBin[fftIdx].real;
//				//		cnt++;
//				//		continue;
//				//	}
//				//	//开始合并
//				//	sum /= cnt;
//				//	for (; start <= end; start++) {
//				//		float circleX = cos(((start / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
//				//		float circleY = -sin(((start / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
//				//		float x = sum / amplifier;
//				//		//x = (toDB(x) + 75) / 200;
//				//		if (x < 0)x = 0;
//				//		//x = NeverReach(x, specMaxHeight);
//				//		//x *= (1.0f - ((xMappingStrenth + 1.0f) / xMappingStrenth) * (-1.0f / (xMappingStrenth * ((fftIdx) / binSizeF) + 1.0f) + 1.0f));
//				//		if (x <= 0)x = 0;
//				//		float a = 0.1;
//				//		float b = 10;
//				//		float t = float(start) / fftVertexPairCnt;
//				//		float xamp = pow(a - a * t, 3) * b + 1;
//				//		//x *= (1 + pow(t, 3));
//    //                    pVertexBuffer[2 * start].pos = {t,0,0};
//    //                    pVertexBuffer[2 * start + 1].pos = {t,x + 4.0f / 1080, 0};
//    //                    ImGui::ColorConvertHSVtoRGB(t * (0.85-0.7) + 0.7, 1, 1, r, g, b);
//    //                    pVertexBuffer[2 * start].color = {r,g,b,1};
//    //                    pVertexBuffer[2 * start + 1].color = {r,g,b,1};
//				//	}
//				//	//清理
//				//	sum = pCurFFTBin[fftIdx].real;
//				//	cnt = 1;
//				//	begginingSX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
//				//	begginingSX *= (fftVertexPairCnt - 1);
//				//}
//    //        }
//
//            pGO->UpdateVertexBufferToGPU();
//            renderer.setAlphaBlend(false);
//            renderer.clearRT(0, 0, 0, 0);
//
//            pGOConstantBuffer->userData.x = 0;
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(1, 1, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(1, -1, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(-1, 1, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(-1, -1, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//            /// <summary>
//            /// /////////////////////
//            /// </summary>
//            /// <param name="pArg"></param>
//            /// <returns></returns>
//            pGOConstantBuffer->userData.x = 1;
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(1, 0.5, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(1, -0.5, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(-1, 0.5, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//            DirectX::XMStoreFloat4x4(
//                addr(pGOConstantBuffer->mvp),
//                DirectX::XMMatrixScaling(-1, -0.5, 1)
//                *
//                DirectX::XMMatrixRotationZ(0)
//                *
//                DirectX::XMMatrixTranslation(0, 0, 0)
//            );
//            pGO->UpdateConstantBufferToGPU();
//            renderer.render(pGO, fftVertexPairCnt * 2);
//
//
//            renderer.copyToRT();
//            renderer.PresentRT(1, 0);
//        }
//    } catch (Suancai::Exception::BaseException* p) {
//        p->showMsg();
//    }
//
//    return 0;
//}
//
//unsigned __stdcall AudioThread(void* pArg) {
//
//    pDec = new Suancai::Media::Audio::AudioDecoder();
//    pDec->decode((char8_t*)((u8string*)pArg)->c_str());
//
//	Suancai::Media::Audio::AudioEnumerator e;
//	e.init();
//	Suancai::Media::Audio::AudioRenderer r;
//	r.init(e.getDefaultRenderDevice(), 10000000, false);
//	auto p = r.getRenderClient();
//	r.enableRender();
//    pAR = &r;
//	int size = (48000.0f / 44100.0f) * pDec->allSamples.size();
//	float* final = new float[size];
//	for (int i = 0; i < size; i++) {
//		float t = float(i) / size;
//		float idxIn = t * pDec->allSamples.size();
//		int l = floor(idxIn);
//		int r = l + 1;
//		if (r >= pDec->allSamples.size()) {
//			r = l;
//		}
//		float tt = (idxIn - l) / (r - l);
//		float val = (1 - tt) * pDec->allSamples[l] + tt * pDec->allSamples[r];
//		final[i] = val;
//	}
//	r.play(final, size);
//
//    return 0;
//}

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool hasAttribute(DWORD attributes, DWORD targetAttribute) {
    return (attributes & targetAttribute) != 0;
}

void EnumFile(WCHAR* folderName, vector<u8string>& vec) {

    WIN32_FIND_DATAW data;

    HANDLE h = FindFirstFileW(folderName, &data);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    u16string fileName;
    u8string out;

    vec.clear();

    do {
        // Check if it's a file (not a directory or hidden file)
        if (!hasAttribute(data.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) &&
            !hasAttribute(data.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN)) {
            fileName.clear();
            out.clear();
            fileName = (char16_t*)data.cFileName;
            UTFStringAffair::UTF16To8(fileName, out);
            vec.push_back(out);
        }
    } while (FindNextFileW(h, &data));

    FindClose(h);
}

// Main code
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    Suancai::Media::Audio::AudioRenderer* pR = nullptr;

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF((char*)u8"C:\\Users\\suric\\Downloads\\SmileySans-Oblique.otf", 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<u8string> fileList;
    std::u8string path = u8"C:/Users/Suric/AppData/Local/GeometryDash/*";
    std::u16string path16 = u"C:/Users/Suric/AppData/Local/GeometryDash/";
    EnumFile((WCHAR*)L"C:/Users/Suric/AppData/Local/GeometryDash/*", fileList);

    Suancai::Player::KaraPlayer kp;

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (IsWindowVisible(hwnd) == FALSE) {
            Sleep(0);
            continue;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
            // Based on your use case you may want one of the other.
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        if (ImGui::Begin("Main", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::BeginTabBar("Bar")) {
                if (ImGui::BeginTabItem("Vifft")) {
                    if (ImGui::Button("Up Folder")) {
                        std::u8string parentPath;
                        PathUtil::GetParentFolder(path, parentPath);
                        std::u16string p16;
                        UTFStringAffair::UTF8To16(parentPath, p16);
                        //EnumFile((WCHAR*)p16.c_str(), fileList);
                    }
                    for (auto it : fileList) {
                        if (ImGui::Button((char*)it.c_str())) {
                            u8string* p = new u8string();
                            p->append(u8"C:/Users/Suric/AppData/Local/GeometryDash/").append(it);
                            kp.play((char8_t*)p->c_str());
                            //_beginthreadex(0, 0, AudioThread, p, 0, 0);
                            //_beginthreadex(0, 0, DecodeThread, nullptr, 0, 0);
                        }
                    }
                    ImGui::Text("%.2f/%d/%.2f", kp.time, kp.sample, kp.sample / 48000.0);
                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("coeff", (float*)&coeff, 0.8f, 1.0f);
                    ImGui::SliderFloat("amp", (float*)&amplifier, 0.0f, 2048);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    ImGui::SameLine();
                    if (pR != nullptr) {
                        auto pos = pR->getDevicePosition();
                        ImGui::Text("%d/%d=%.2f", pos.first, pos.second, (double)pos.first / pos.second);
                    }

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

         
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release(); g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release(); g_pd3dDevice = nullptr;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr;
    }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
