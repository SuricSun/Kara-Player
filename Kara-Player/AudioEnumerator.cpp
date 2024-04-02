#include "AudioEnumerator.h"

Suancai::Media::Audio::AudioEnumerator::AudioEnumerator() {
}

void Suancai::Media::Audio::AudioEnumerator::init() {

	if (FAILED(CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED))) {
		SUANCAI_BASE_THROW("CoInitializeEx: 无法初始化COM", -1);
	}
	//创建enum class以得到device collection
	HRESULT hr =
		CoCreateInstance(
			__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator),
			(void**)addr(this->p_enumerator)
		);
	CHECK_AND_THROW(FAILED(hr), "can't create IMMDeviceEnumerator", -1);
}

void Suancai::Media::Audio::AudioEnumerator::enumAudioDevice() {

	this->p_renderDeviceVec.clear();
	this->p_captureDeviceVec.clear();

	IMMDeviceCollection* p_collection = nullptr;

	//*枚举render设备
	HRESULT hr = this->p_enumerator->EnumAudioEndpoints(EDataFlow::eRender, DEVICE_STATE_ACTIVE, addr(p_collection));
	CHECK_AND_THROW(FAILED(hr), "can't enum audio end points", -1);

	u32 size = 0;
	p_collection->GetCount(addr(size));

	for (u32 i = 0; i < size; i++) {
		this->p_renderDeviceVec.push_back(nullptr);
		p_collection->Item(i, addr(this->p_renderDeviceVec.back()));
	}

	p_collection->Release();

	//*枚举capture设备
	hr = this->p_enumerator->EnumAudioEndpoints(EDataFlow::eCapture, DEVICE_STATE_ACTIVE, addr(p_collection));
	CHECK_AND_THROW(FAILED(hr), "can't enum audio end points", -1);

	size = 0;
	p_collection->GetCount(addr(size));

	for (u32 i = 0; i < size; i++) {
		this->p_captureDeviceVec.push_back(nullptr);
		p_collection->Item(i, addr(this->p_captureDeviceVec.back()));
	}

	p_collection->Release();
}

IMMDevice* Suancai::Media::Audio::AudioEnumerator::getDefaultRenderDevice() {

	IMMDevice* p_ret = nullptr;
	//TODO: Whats ERole::eMultiedia?
	HRESULT hr = this->p_enumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, addr(p_ret));
	CHECK_AND_THROW(FAILED(hr), "Can't get default audio render device", -1);
	return p_ret;
}

IMMDevice* Suancai::Media::Audio::AudioEnumerator::getDefaultCaptureDevice() {

	IMMDevice* p_ret = nullptr;
	HRESULT hr = this->p_enumerator->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eMultimedia, addr(p_ret));
	CHECK_AND_THROW(FAILED(hr), "Can't get default audio capture device", -1);
	return p_ret;
}

Suancai::Media::Audio::AudioEnumerator::~AudioEnumerator() {

	SAFE_RELEASE(this->p_enumerator);
}
