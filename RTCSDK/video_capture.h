/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <vector>
#include "absl/memory/memory.h"
#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"

namespace vi {
	class VideoCapturerBase : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
	public:
		VideoCapturerBase();
		~VideoCapturerBase() override;

		void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
			const rtc::VideoSinkWants& wants) override;
		void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

	protected:
		void OnFrame(const webrtc::VideoFrame& frame);
		rtc::VideoSinkWants GetSinkWants();

	private:
		void UpdateVideoAdapter();

		rtc::VideoBroadcaster _broadcaster;
		cricket::VideoAdapter _videoAdapter;
	};

	class VideoCapture : public VideoCapturerBase, public rtc::VideoSinkInterface<webrtc::VideoFrame> {
	public:
		static VideoCapture* Create(size_t width, size_t height, size_t targetFps, size_t captureDeviceIndex);

		virtual ~VideoCapture();

		void OnFrame(const webrtc::VideoFrame& frame) override;

	private:
		VideoCapture();
		bool Init(size_t width, size_t height, size_t targetFps, size_t captureDeviceIndex);
		void Destroy();

		rtc::scoped_refptr<webrtc::VideoCaptureModule> _vcm;
		webrtc::VideoCaptureCapability _capability;
	};


	class CapturerTrackSource : public webrtc::VideoTrackSource {
	public:
		static rtc::scoped_refptr<CapturerTrackSource> Create(size_t width, size_t height, size_t targetFps) {
			std::unique_ptr<vi::VideoCapture> capturer;
			std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
			if (!info) {
				return nullptr;
			}
			int num_devices = info->NumberOfDevices();
			for (int i = 0; i < num_devices; ++i) {
				capturer = absl::WrapUnique(VideoCapture::Create(width, height, targetFps, i));
				if (capturer) {
					return new rtc::RefCountedObject<CapturerTrackSource>(std::move(capturer));
				}
			}

			return nullptr;
		}

		std::unique_ptr<VideoCapture>& capturer() { return _capturer; }

	protected:
		explicit CapturerTrackSource(
			std::unique_ptr<VideoCapture> capturer)
			: VideoTrackSource(/*remote=*/false), _capturer(std::move(capturer)) {}

	private:
		rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
			return _capturer.get();
		}
		std::unique_ptr<VideoCapture> _capturer;
	};
}

