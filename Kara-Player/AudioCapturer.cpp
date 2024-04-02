#include "AudioCapturer.h"

Suancai::Media::Audio::AudioCapturer::AudioCapturer() {
}

void Suancai::Media::Audio::AudioCapturer::init(IMMDevice* p_audDevice, u32 bufferDurationIn100NanoUnit, bool whetherLoopbackCapturing) {

	SAFE_RELEASE(this->p_client);
	SAFE_RELEASE(this->p_device);
	SAFE_RELEASE(this->pCaptureClient);
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
	//active loopback
	if (whetherLoopbackCapturing) {
		initFlags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
	}

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

	hr = this->p_client->GetService(__uuidof(IAudioCaptureClient), (void**)addr(this->pCaptureClient));
	CHECK_AND_THROW(FAILED(hr), "this->p_client->GetService(__uuidof(IAudioRenderClient), (void**)addr(this->p_captureClient))", -1);

	IPropertyStore* p_store = NULL;
	this->p_device->OpenPropertyStore(STGM_READ, addr(p_store));
	PROPVARIANT prop = {};
	p_store->GetValue(PKEY_Device_FriendlyName, addr(prop));
	this->device_name.assign((char16_t*)prop.pwszVal);
}

void Suancai::Media::Audio::AudioCapturer::enableCapture() {

	CHECK_AND_THROW(FAILED(this->p_client->Start()), "无法开始捕获", -1);
}

void Suancai::Media::Audio::AudioCapturer::disable_capture() {

	CHECK_AND_THROW(FAILED(this->p_client->Stop()), "无法停止捕获", -1);
}

IAudioCaptureClient* Suancai::Media::Audio::AudioCapturer::getRenderClient() {

	return this->pCaptureClient;
}

Suancai::Media::Audio::AudioCapturer::~AudioCapturer() {

	SAFE_RELEASE(this->p_client);
	SAFE_RELEASE(this->p_device);
	SAFE_RELEASE(this->pCaptureClient);
	if (this->p_wave_fmt != nullptr) {
		CoTaskMemFree(this->p_wave_fmt);
	}
}
