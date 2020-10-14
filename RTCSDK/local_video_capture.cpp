/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "local_video_capture.h"
#include <stdint.h>
#include <memory>
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"

namespace vi {

	LocalVideoCapture::LocalVideoCapture() : _vcm(nullptr) {}

	bool LocalVideoCapture::Init(size_t width,
		size_t height,
		size_t targetFps,
		size_t captureDeviceIndex) {
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> deviceInfo(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());

		char deviceName[256];
		char uniqueName[256];
		if (deviceInfo->GetDeviceName(static_cast<uint32_t>(captureDeviceIndex),
			deviceName, sizeof(deviceName), uniqueName,
			sizeof(uniqueName)) != 0) {
			Destroy();
			return false;
		}

		_vcm = webrtc::VideoCaptureFactory::Create(uniqueName);
		if (!_vcm) {
			return false;
		}
		_vcm->RegisterCaptureDataCallback(this);

		deviceInfo->GetCapability(_vcm->CurrentDeviceName(), 0, _capability);

		_capability.width = static_cast<int32_t>(width);
		_capability.height = static_cast<int32_t>(height);
		_capability.maxFPS = static_cast<int32_t>(targetFps);
		_capability.videoType = webrtc::VideoType::kI420;

		if (_vcm->StartCapture(_capability) != 0) {
			Destroy();
			return false;
		}

		RTC_CHECK(_vcm->CaptureStarted());

		return true;
	}

	LocalVideoCapture* LocalVideoCapture::Create(size_t width,
		size_t height,
		size_t target_fps,
		size_t capture_device_index) {
		std::unique_ptr<LocalVideoCapture> vcm_capturer(new LocalVideoCapture());
		if (!vcm_capturer->Init(width, height, target_fps, capture_device_index)) {
			RTC_LOG(LS_WARNING) << "Failed to create VcmCapturer(w = " << width
				<< ", h = " << height << ", fps = " << target_fps
				<< ")";
			return nullptr;
		}
		return vcm_capturer.release();
	}

	void LocalVideoCapture::Destroy() {
		if (!_vcm)
			return;

		_vcm->StopCapture();
		_vcm->DeRegisterCaptureDataCallback();
		// Release reference to VCM.
		_vcm = nullptr;
	}

	LocalVideoCapture::~LocalVideoCapture() {
		Destroy();
	}

	void LocalVideoCapture::OnFrame(const webrtc::VideoFrame& frame) {
		VideoCapturerBase::OnFrame(frame);
	}

	VideoCapturerBase::VideoCapturerBase() = default;

	VideoCapturerBase::~VideoCapturerBase() = default;

	void VideoCapturerBase::OnFrame(const webrtc::VideoFrame& frame) {
		int cropped_width = 0;
		int cropped_height = 0;
		int out_width = 0;
		int out_height = 0;

		if (!_videoAdapter.AdaptFrameResolution(
			frame.width(), frame.height(), frame.timestamp_us() * 1000,
			&cropped_width, &cropped_height, &out_width, &out_height)) {
			// Drop frame in order to respect frame rate constraint.
			return;
		}

		if (out_height != frame.height() || out_width != frame.width()) {
			// Video adapter has requested a down-scale. Allocate a new buffer and
			// return scaled version.
			rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer =
				webrtc::I420Buffer::Create(out_width, out_height);
			scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
			_broadcaster.OnFrame(webrtc::VideoFrame::Builder()
				.set_video_frame_buffer(scaled_buffer)
				.set_rotation(webrtc::kVideoRotation_0)
				.set_timestamp_us(frame.timestamp_us())
				.set_id(frame.id())
				.build());
		}
		else {
			// No adaptations needed, just return the frame as is.
			_broadcaster.OnFrame(frame);
		}
	}

	rtc::VideoSinkWants VideoCapturerBase::GetSinkWants() {
		return _broadcaster.wants();
	}

	void VideoCapturerBase::AddOrUpdateSink(
		rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
		const rtc::VideoSinkWants& wants) {
		_broadcaster.AddOrUpdateSink(sink, wants);
		UpdateVideoAdapter();
	}

	void VideoCapturerBase::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) {
		_broadcaster.RemoveSink(sink);
		UpdateVideoAdapter();
	}

	void VideoCapturerBase::UpdateVideoAdapter() {
		rtc::VideoSinkWants wants = _broadcaster.wants();
		_videoAdapter.OnSinkWants(wants);
		//_videoAdapter.OnResolutionFramerateRequest(wants.target_pixel_count, wants.max_pixel_count, wants.max_framerate_fps);
	}

}
