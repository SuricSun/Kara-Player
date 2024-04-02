#include "AudioRenderer.h"

Suancai::Media::Audio::AudioRenderer::AudioRenderer() {
}

void Suancai::Media::Audio::AudioRenderer::init(IMMDevice* p_audDevice, u32 bufferDurationIn100NanoUnit, bool whetherLoopbackCapturing) {

	SAFE_RELEASE(this->p_client);
	SAFE_RELEASE(this->p_device);
	SAFE_RELEASE(this->pRenderClient);
	if (this->p_wave_fmt != nullptr) {
		CoTaskMemFree(this->p_wave_fmt);
	}
	//get aud client from device
	this->p_device = p_audDevice;
	//不是自己创建的object要addRef
	this->p_device->AddRef();

	HRESULT hr = this->p_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)addr(this->p_client));
	CHECK_AND_THROW(FAILED(hr), "can't activate p_device", hr);

	// * set bypass system apo effect (sound enhancement) before initialization
	IAudioClient2* p;
	hr = this->p_client->QueryInterface(IID_PPV_ARGS(addr(p)));
	if (SUCCEEDED(hr)) {
		AudioClientProperties acp{};
		acp.cbSize = sizeof(acp);
		acp.bIsOffload = FALSE;
		acp.eCategory = AUDIO_STREAM_CATEGORY::AudioCategory_Other;
		acp.Options = AUDCLNT_STREAMOPTIONS_RAW;
		hr = p->SetClientProperties(addr(acp));
		if (FAILED(hr)) {
			SUANCAI_MSG("byd", "byd");
		}
	} else {
		SUANCAI_MSG("byd out", "byd");
	}

	hr = this->p_client->GetMixFormat((WAVEFORMATEX**)addr(this->p_wave_fmt));
	CHECK_AND_THROW(FAILED(hr), "can't get mix format on aud client", hr);

	if (this->p_wave_fmt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
		this->audioFmt = AudioSampleFormat::IEEE_Float;
	} else if (this->p_wave_fmt->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
		this->audioFmt = AudioSampleFormat::PCM;
	} else {
		SUANCAI_BASE_THROW("Unsupported device output fmt", -1);
	}

	this->samples_per_sec = this->p_wave_fmt->Format.nSamplesPerSec;
	this->channels = this->p_wave_fmt->Format.nChannels;
	this->sampleBitDepth = this->p_wave_fmt->Format.wBitsPerSample;

	this->frameSizeInByte = this->channels * (this->sampleBitDepth / 8);

	REFERENCE_TIME bufferDuration = bufferDurationIn100NanoUnit;

	// * initialize
	DWORD initFlags = 0;

	hr =
		this->p_client->Initialize(
			AUDCLNT_SHAREMODE::AUDCLNT_SHAREMODE_SHARED,
			initFlags,
			bufferDuration,
			0,
			(WAVEFORMATEX*)this->p_wave_fmt,
			nullptr
		);
	CHECK_AND_THROW(FAILED(hr), "can't initialize p_client", hr);

	//this->hAudioBufferReadyEvent = CreateEvent(nullptr, false, false, nullptr);
	//THROW_ON_NULL_AUD(this->hAudioBufferReadyEvent, "CreateEvent(nullptr, false, false, nullptr)");

	//hr = this->p_client->SetEventHandle(this->hAudioBufferReadyEvent);
	//THROW_ON_HR_FAILED_AUD(hr, "this->p_client->SetEventHandle(this->hAudioBufferReadyEvent)");

	hr = this->p_client->GetBufferSize(addr(this->buffer_frame_cnt));
	CHECK_AND_THROW(FAILED(hr), "this->p_client->GetBufferSize(addr(audioFrameCnt))", -1);

	hr = this->p_client->GetService(__uuidof(IAudioRenderClient), (void**)addr(this->pRenderClient));
	CHECK_AND_THROW(FAILED(hr), "this->p_client->GetService(__uuidof(IAudioRenderClient), (void**)addr(this->p_captureClient))", -1);

	hr = this->p_client->GetService(__uuidof(IAudioClock), (void**)addr(this->pClock));
	CHECK_AND_THROW(FAILED(hr), "this->p_client->GetService(__uuidof(IAudioClock), (void**)addr(this->pClock))", -1);

	IPropertyStore* p_store = NULL;
	this->p_device->OpenPropertyStore(STGM_READ, addr(p_store));
	PROPVARIANT prop = {};
	p_store->GetValue(PKEY_Device_FriendlyName, addr(prop));
	this->device_name.assign((char16_t*)prop.pwszVal);
}

void Suancai::Media::Audio::AudioRenderer::enableRender() {
	
	CHECK_AND_THROW(FAILED(this->p_client->Start()), "无法开始捕获", -1);
}

void Suancai::Media::Audio::AudioRenderer::play(std::vector<std::vector<float>>& data, i32 samples) {

	UINT32 bufferUsedFrameCnt = 0;
	HRESULT hr;
	UINT32 baseIdx = 0;
	float* pData = nullptr;
	while (true) {
		hr = this->p_client->GetCurrentPadding(addr(bufferUsedFrameCnt));
		UINT32 available = this->buffer_frame_cnt - bufferUsedFrameCnt;
		available = min(available, samples - baseIdx);
		
		hr = this->pRenderClient->GetBuffer(available, (BYTE**)addr(pData));
		i32 channels = this->channels;
		for (int frameIdx = 0; frameIdx < available; frameIdx++) {
			// interleave channels
			for (int chnIdx = 0; chnIdx < channels; chnIdx++) {
				pData[frameIdx * channels + chnIdx] = data[chnIdx][baseIdx + frameIdx];
			}
		}
		hr = this->pRenderClient->ReleaseBuffer(available, 0);
		baseIdx += available;
		if (baseIdx >= samples) {
			return;
		}
		Sleep(16);
	}
}

std::pair<u64, u64>& Suancai::Media::Audio::AudioRenderer::getDevicePosition() {

	this->pClock->GetFrequency(addr(this->devicePosition.second));
	this->pClock->GetPosition(addr(this->devicePosition.first), NULL);
	return this->devicePosition;
}

void Suancai::Media::Audio::AudioRenderer::getFreeSpaceCnt(u32& freeCnt, u32& totalCnt) {

	this->p_client->GetCurrentPadding(&freeCnt);
	freeCnt = buffer_frame_cnt - freeCnt;
	totalCnt = this->buffer_frame_cnt;
}

void* Suancai::Media::Audio::AudioRenderer::getFreeSpacePointer(i32 frameRequested) {
	
	void* p = nullptr;
	//TODO: CHEKC HERE
	HRESULT hr = this->pRenderClient->GetBuffer(frameRequested, (BYTE**)addr(p));
	return p;
}

void Suancai::Media::Audio::AudioRenderer::releaseFreeSpacePointer(i32 frameWrote) {

	//TODO: CHECK HERE
	HRESULT hr = this->pRenderClient->ReleaseBuffer(frameWrote, 0);
}

void Suancai::Media::Audio::AudioRenderer::disableRender() {

	CHECK_AND_THROW(FAILED(this->p_client->Stop()), "无法停止捕获", -1);
}

IAudioRenderClient* Suancai::Media::Audio::AudioRenderer::getRenderClient() {
    
	return this->pRenderClient;
}

Suancai::Media::Audio::AudioRenderer::~AudioRenderer() {

	SAFE_RELEASE(this->p_client);
	SAFE_RELEASE(this->p_device);
	SAFE_RELEASE(this->pRenderClient);
	if (this->p_wave_fmt != nullptr) {
		CoTaskMemFree(this->p_wave_fmt);
	}
}
