#include "AudioSpecRenderer.h"

#include "imgui.h"

void Suancai::Kara::Renderer::AudioSpecRenderer::init() {

	WNDCLASS wc = { };

	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = L"Kara Vifft";
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);

	RegisterClassW(&wc);

	int screenX = GetSystemMetrics(SM_CXSCREEN);
	int screenY = GetSystemMetrics(SM_CYSCREEN);
	int edgeLen = 1080;

	// Create the window.
	HWND hwnd = CreateWindowExW(
		// * layered and transparent make the window click-through-able
		WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOPMOST,								// Optional window styles.
		L"Kara Vifft",
		L"Kara Vifft",
		WS_POPUP | WS_DISABLED,
		0, 0, 1920, 1080,
		NULL,
		NULL,
		GetModuleHandleW(NULL),
		NULL
	);

	CHECK_AND_THROW(hwnd == INVALID_HANDLE_VALUE, "无法创建窗口", 0);

	this->hwnd = hwnd;

	ShowWindow(hwnd, SW_SHOW);

	this->rdr.init();
	this->rdr.connectToWindow(this->hwnd);

	this->go.init(
		(1920) * 2,
		nullptr,
		sizeof(GraphicsObject::ConstantBuffer),
		#ifdef _DEBUG
		u8"BaseVS.cso",
		u8"BasePS.cso",
		#else
		u8"SimpleVS.cso",
		u8"SimplePS.cso",
		#endif
		addr(this->rdr)
	);
}

void Suancai::Kara::Renderer::AudioSpecRenderer::render(Suancai::Util::FFT::Complex* pSpec, i32 cnt) {

	//this->specData.resize(cnt);
	//// * pre process
	//float minGap = 0.025;
	////
	//int startIdx = 0;
	//int addIdx = 0;
	////
	//float gapSum = 0;
	//while (startIdx < (cnt - 1)) {
	//	gapSum = 0;
	//	float startIdxMap = CoordMap(startIdx, xMappingStrenth);
	//	float nxtIdxMap = CoordMap(startIdx + 1, xMappingStrenth);
	//	if ((nxtIdxMap - startIdxMap) >= minGap) {
	//		// * ok then
	//		this->specData[addIdx] = pSpec[startIdx];
	//		addIdx++;
	//		this->specData[addIdx] = pSpec[startIdx + 1];
	//		addIdx++;
	//		startIdx += 2;
	//		continue;
	//	}
	//	// else we loop until reach requirement
	//	for (int i = startIdx + 2; i < cnt; i++) {
	//		float curIdxMap = CoordMap(i, xMappingStrenth);
	//		if ((curIdxMap - startIdxMap) >= minGap) {
	//			// * quit
	//			gapSum /= (i - startIdx - 1);
	//			this->specData[addIdx].real = (pSpec[startIdx].real + gapSum) / 2.0f;
	//			addIdx++;
	//			this->specData[addIdx].real = (pSpec[i].real + gapSum) / 2.0f;
	//			addIdx++;
	//			startIdx = i + 1;
	//			break;
	//		} else {
	//			gapSum += pSpec[i].real;
	//		}
	//	}
	//}

	//cnt = startIdx;
	
	int screenX = GetSystemMetrics(SM_CXSCREEN);
	int screenY = GetSystemMetrics(SM_CYSCREEN);
	int edgeLen = 1080;

	GraphicsObject::ConstantBuffer* pGOConstantBuffer = nullptr;
	pGOConstantBuffer = (GraphicsObject::ConstantBuffer*)this->go.getConstantBuffer();
	pGOConstantBuffer->screen.x = screenX;
	pGOConstantBuffer->screen.y = screenY;
	GraphicsObject::VertexProp* pVertexBuffer = (GraphicsObject::VertexProp*)this->go.getVertexBuffer();

	i32 fftBinSize = cnt;

	u32 idx20hz = u32(std::floor(20.0f / (48.0f * 1000.0f / 8192.0f)));

	pSpec += idx20hz;
	fftBinSize -= idx20hz;

	// TODO: This is dumb to calc every frame
	i32 vertexCnt = this->go.getVertexCnt();
	for (int i = 0; i < vertexCnt / 2; i++) {
		float t = float(i) / (vertexCnt / 2);
		pVertexBuffer[2 * i].pos = {t,0,0};
		pVertexBuffer[2 * i + 1].pos = {t,0, 0};
	}

	float h = 0, s = 1, v = 1, r = 0, g = 0, b = 0;
	i64 fftIdx = 1;
	i64 lastProcessIdx = 0;
	float xMappingStrenth = 16;
	i32 windCnt = 8;
	i32 curTime = 0;
	i32 binSize = fftBinSize;
	float binSizeF = binSize;
	i32 fftVertexPairCnt = vertexCnt / 2;
	Suancai::Util::FFT::Complex* pCurFFTBin = pSpec;
	float amplifier = 90;
	float circleRadius = 0.2;
	{
		//pCurFFTBin[0].real = 0;
		bool shouldQuit = false;
		for (fftIdx = 1; fftIdx < binSize; fftIdx++) {
			float sX = CoordMap(((fftIdx == 0) ? 0 : (fftIdx - 1.0f)) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (((fftIdx == 0) ? 0 : (fftIdx - 1.0f)) / (binSizeF - 1)) + 1.0f) + 1.0f);
			float mX = CoordMap((fftIdx) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * ((fftIdx) / (binSizeF - 1)) + 1.0f) + 1.0f);
			float eX = CoordMap(((fftIdx == binSize) ? binSize : (fftIdx + 1.0f)) / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (((fftIdx == binSize) ? binSize : (fftIdx + 1.0f)) / (binSizeF - 1)) + 1.0f) + 1.0f);
			//映射到顶点索引坐标系
			sX *= (fftVertexPairCnt - 1);
			mX *= (fftVertexPairCnt - 1);
			eX *= (fftVertexPairCnt - 1);
			//获取中点
			sX = (fftIdx == 1) ? sX : (sX + mX) / 2.0f;
			eX = (mX + eX) / 2.0f;
			//求出贝塞尔中点
			float rMX = ((sX + mX) / 2.0f + (mX + eX) / 2.0f) / 2.0f;// real mX
			//判断sX和eX的距离，大于1且至少包含一个贝塞尔点，否则停止贝塞尔平滑并跳出
			i32 start = std::ceil(sX);
			i32 end = std::floor(eX);
			if (!((eX - sX) > 1.0f && (start <= end))) {
				end = start;
				shouldQuit = true;
				//break;
			}
			lastProcessIdx = end;
			//获取高度
			float sY = pCurFFTBin[fftIdx - 1].real / amplifier;
			float mY = pCurFFTBin[fftIdx].real / amplifier;
			float eY = pCurFFTBin[fftIdx + 1].real / amplifier;
			//获取中点
			sY = (fftIdx == 1) ? sY : (sY + mY) / 2.0f;
			eY = (mY + eY) / 2.0f;
			//开始贝塞尔插值
			float x = 0.0f;
			//(s-2m+l)x^2 + (2m-2s)x + s = start
			//(s-2m+l)x^2 + (2m-2s)x + s - start = 0
			for (u32 i = start; i <= end; i++) {
				if (i < rMX) {
					x = (i - sX) / (rMX - sX) / 2.0f;
				} else {
					x = 0.5f + (i - rMX) / (eX - rMX) / 2.0f;
				}
				//x = (i - float(start)) / (end - float(start));
				x = ((sY - 2.0f * mY + eY) * pow(x, 2.0f) + 2.0f * (mY - sY) * x + sY);
				//x = (toDB(x) + 75) / 200;
				if (x < 0)x = 0;
				//x = NeverReach(x, specMaxHeight);
				//x *= (1.0f - ((xMappingStrenth + 1.0f) / xMappingStrenth) * (-1.0f / (xMappingStrenth * ((fftIdx) / binSizeF) + 1.0f) + 1.0f));
				float circleX = cos(((i / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
				float circleY = -sin(((i / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
				//pVertexBuffer[i * 2 + 0].pos.x = float(i) / fftVertexPairCnt;
				if (x <= 0)x = 0;
				float a = 0.1;
				float b = 10;
				float t = float(i) / fftVertexPairCnt;
				float xamp = pow(a - a * t, 3) * b + 1;
				//x *= (1 + pow(t, 3));
				pVertexBuffer[2 * i].pos = {t,0,0};
				pVertexBuffer[2 * i + 1].pos = {t,x + 4.0f / 1080, 0};
				//pVertexBuffer[2 * i].pos = {circleX * (x + circleRadius), circleY * (x + circleRadius), 0};
				//pVertexBuffer[2 * i + 1].pos = {circleX * (circleRadius), circleY * (circleRadius), 0};
				ImGui::ColorConvertHSVtoRGB(t * (0.8 - 0.7) + 0.7, 1, 1, r, g, b);
				pVertexBuffer[2 * i].color = {r,g,b,1};
				pVertexBuffer[2 * i + 1].color = {r,g,b,1};
			}
			if (shouldQuit) {
				break;
			}
		}
		//继续处理不需要贝塞尔的剩余fft数据
		float sum = pCurFFTBin[fftIdx].real;
		i32 cnt = 1;
		float begginingSX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
		begginingSX *= (fftVertexPairCnt - 1);
		while (true) {
			fftIdx++;
			if (fftIdx >= binSize) {
				break;
			}
			float sX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
			sX *= (fftVertexPairCnt - 1);
			//判断当前sX到beginningSX之间有至少一个整数
			i32 start = std::ceil(begginingSX);
			i32 end = std::floor(sX);
			if ((start) > end) {
				sum = max(sum, pCurFFTBin[fftIdx].real);
				cnt++;
				continue;
			}
			//开始合并
			//sum /= cnt;
			for (; start <= end; start++) {
				float circleX = cos(((start / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
				float circleY = -sin(((start / float(fftVertexPairCnt - 1)) - 0.5) * 2 * windCnt * PI_F + curTime);
				float x = sum / amplifier;
				//x = (toDB(x) + 75) / 200;
				if (x < 0)x = 0;
				//x = NeverReach(x, specMaxHeight);
				//x *= (1.0f - ((xMappingStrenth + 1.0f) / xMappingStrenth) * (-1.0f / (xMappingStrenth * ((fftIdx) / binSizeF) + 1.0f) + 1.0f));
				if (x <= 0)x = 0;
				float a = 0.1;
				float b = 10;
				float t = float(start) / fftVertexPairCnt;
				float xamp = pow(a - a * t, 3) * b + 1;
				////x *= (1 + pow(t, 3));
				pVertexBuffer[2 * start].pos = {t,0,0};
				pVertexBuffer[2 * start + 1].pos = {t,x + 4.0f / 1080, 0};
				//pVertexBuffer[2 * start].pos = {circleX * (x + circleRadius), circleY * (x + circleRadius), 0};
				//pVertexBuffer[2 * start + 1].pos = {circleX * (circleRadius), circleY * (circleRadius), 0};
				ImGui::ColorConvertHSVtoRGB(t * (0.8 - 0.7) + 0.7, 1, 1, r, g, b);
				pVertexBuffer[2 * start].color = {r,g,b,1};
				pVertexBuffer[2 * start + 1].color = {r,g,b,1};
			}
			//清理
			sum = pCurFFTBin[fftIdx].real;
			cnt = 1;
			begginingSX = CoordMap(fftIdx / (binSizeF - 1), xMappingStrenth);// ((xMappingStrenth + 1.0f) / xMappingStrenth)* (-1.0f / (xMappingStrenth * (fftIdx / (binSizeF - 1)) + 1.0f) + 1.0f);
			begginingSX *= (fftVertexPairCnt - 1);
		}
	}
	//for (int i = 0; i < fftBinSize; i++) {
	//	float mappednormalizedPos = CoordMap(float(i) / (fftBinSize - 1), 128);
	//	pVertexBuffer[2 * i].pos = {mappednormalizedPos,0,0};
	//	pVertexBuffer[2 * i + 1].pos = {mappednormalizedPos, pSpec[i].real / 80 + 4.0f / 1080, 0};
	//	h = float(i) / fftBinSize;
	//	ImGui::ColorConvertHSVtoRGB(mappednormalizedPos * (0.85 - 0.7) + 0.7, 1, 1, r, g, b);
	//	pVertexBuffer[2 * i].color = {r,g,b,1};
	//	pVertexBuffer[2 * i + 1].color = {r,g,b,1};
	//}

	this->go.UpdateVertexBufferToGPU();
	this->rdr.setAlphaBlend(false);
	this->rdr.clearRT(0, 0, 0, 0);

	pGOConstantBuffer->userData.x = 0;
	DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixTranslation(-0.5, 0, 0)
		*
		DirectX::XMMatrixScaling(2, 1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);

	DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixTranslation(-0.5, 0, 0)
		*
		DirectX::XMMatrixScaling(2, -1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);

	//DirectX::XMStoreFloat4x4(
	//	addr(pGOConstantBuffer->mvp),
	//	DirectX::XMMatrixScaling(-1, 1, 1)
	//	*
	//	DirectX::XMMatrixRotationZ(0)
	//	*
	//	DirectX::XMMatrixTranslation(0, 0, 0)
	//);
	//this->go.UpdateConstantBufferToGPU();
	//this->rdr.render(addr(this->go), vertexCnt);

	//DirectX::XMStoreFloat4x4(
	//	addr(pGOConstantBuffer->mvp),
	//	DirectX::XMMatrixScaling(-1, -1, 1)
	//	*
	//	DirectX::XMMatrixRotationZ(0)
	//	*
	//	DirectX::XMMatrixTranslation(0, 0, 0)
	//);
	//this->go.UpdateConstantBufferToGPU();
	//this->rdr.render(addr(this->go), vertexCnt);
	////////////////////////////////////
	pGOConstantBuffer->userData.x = 1;
	/*DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixScaling(1, 1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
		*
		DirectX::XMMatrixTranslation(0, 0, 0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);

	DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixScaling(1, -1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
		*
		DirectX::XMMatrixTranslation(0, 0, 0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);

	DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixScaling(-1, 1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
		*
		DirectX::XMMatrixTranslation(0, 0, 0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);

	DirectX::XMStoreFloat4x4(
		addr(pGOConstantBuffer->mvp),
		DirectX::XMMatrixScaling(-1, -1, 1)
		*
		DirectX::XMMatrixRotationZ(0)
		*
		DirectX::XMMatrixTranslation(0, 0, 0)
	);
	this->go.UpdateConstantBufferToGPU();
	this->rdr.render(addr(this->go), vertexCnt);*/

	this->rdr.blur();
	this->rdr.blur();

	this->rdr.copyToRT();
	this->rdr.PresentRT(1, 0);
}
