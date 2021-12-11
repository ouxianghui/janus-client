/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "api/set_remote_description_observer_interface.h"
#include "api/rtc_error.h"
#include "api/jsep.h"
#include <functional>
#include <memory>
#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "api/stats/rtc_stats_report.h"

namespace vi {

	using SetRemoteDescCallback = std::function<void(webrtc::RTCError error)>;

	class SetRemoteDescObserver : public webrtc::SetRemoteDescriptionObserverInterface {
	public:
		SetRemoteDescObserver() {}

		void setCallback(std::shared_ptr<SetRemoteDescCallback> callback)
		{
			_callback = callback;
		}

	protected:
		void OnSetRemoteDescriptionComplete(webrtc::RTCError error) override
		{
			if (_callback) {
				(*_callback)(std::move(error));
			}
		}

	private:
		std::shared_ptr<SetRemoteDescCallback> _callback;
	};

	using CreateSessionDescSuccessCallback = std::function<void(webrtc::SessionDescriptionInterface* desc)>;
	using CreateSessionDescFailureCallback = std::function<void(webrtc::RTCError error)>;

	class CreateSessionDescObserver : public webrtc::CreateSessionDescriptionObserver {
	public:
		CreateSessionDescObserver() {}

		void setSuccessCallback(std::shared_ptr<CreateSessionDescSuccessCallback> callback)
		{
			_success = callback;
		}

		void setFailureCallback(std::shared_ptr<CreateSessionDescFailureCallback> callback)
		{
			_failure = callback;
		}

	protected:
		void OnSuccess(webrtc::SessionDescriptionInterface* desc) override
		{
			if (_success) {
				const auto& cb = _success;
				(*cb)(desc);
			}
		}

		void OnFailure(webrtc::RTCError error) override
		{
			if (_failure) {
				const auto& cb = _failure;
				(*cb)(error);
			}
		}

	private:
		std::shared_ptr<CreateSessionDescSuccessCallback> _success;
		std::shared_ptr<CreateSessionDescFailureCallback> _failure;
	};

	using SetSessionDescSuccessCallback = std::function<void()>;
	using SetSessionDescFailureCallback = std::function<void(webrtc::RTCError error)>;

	class SetSessionDescObserver : public webrtc::SetSessionDescriptionObserver {
	public:
		SetSessionDescObserver() {}

		void setSuccessCallback(std::shared_ptr<SetSessionDescSuccessCallback> callback)
		{
			_success = callback;
		}

		void setFailureCallback(std::shared_ptr<SetSessionDescFailureCallback> callback)
		{
			_failure = callback;
		}

	protected:
		void OnSuccess() override
		{
			if (_success) {
				(*_success)();
			}
		}

		void OnFailure(webrtc::RTCError error) override
		{
			if (_failure) {
				(*_failure)(error);
			}
		}

	private:
		std::shared_ptr<SetSessionDescSuccessCallback> _success;
		std::shared_ptr<SetSessionDescFailureCallback> _failure;
	};

	using DataChannelCallback = std::function<void(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)>;
	
	using StateChangeCallback = std::function<void()>;
	using MessageCallback = std::function<void(const webrtc::DataBuffer& buffer)>;
	class DCObserver : public webrtc::DataChannelObserver {
	public:
		void setStateChangeCallback(std::shared_ptr<StateChangeCallback> callback)
		{
			_stateChangeCallback = callback;
		}

		void setMessageCallback(std::shared_ptr<MessageCallback> callback)
		{
			_messageCallback = callback;
		}

	protected:
		void OnStateChange() override
		{
			if (_stateChangeCallback) {
				(*_stateChangeCallback)();
			}
		}

		//  A data buffer was successfully received.
		void OnMessage(const webrtc::DataBuffer& buffer) override
		{
			if (_messageCallback) {
				(*_messageCallback)(buffer);
			}
		}

	private:
		std::shared_ptr<StateChangeCallback> _stateChangeCallback;
		std::shared_ptr<MessageCallback> _messageCallback;
	};

	using ToneChangeCallback = std::function<void(const std::string& tone, const std::string& tone_buffer)>;
	class DtmfObserver : public webrtc::DtmfSenderObserverInterface {
	public:
		void setMessageCallback(std::shared_ptr<ToneChangeCallback> callback)
		{
			_toneChangeCallback = callback;
		}

	protected:
		// Triggered when DTMF |tone| is sent.
		// If |tone| is empty that means the DtmfSender has sent out all the given
		// tones.
		// The callback includes the state of the tone buffer at the time when
		// the tone finished playing.
		void OnToneChange(const std::string& tone, const std::string& tone_buffer) override 
		{
			if (_toneChangeCallback) {
				(*_toneChangeCallback)(tone, tone_buffer);
			}
		}

	private:
		std::shared_ptr<ToneChangeCallback> _toneChangeCallback;
	};

	using StatsCallback = std::function<void(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report)>;
	class StatsObserver : public webrtc::RTCStatsCollectorCallback {
	public:
		static rtc::scoped_refptr<StatsObserver> create() 
		{
			return rtc::scoped_refptr<StatsObserver>(new rtc::RefCountedObject<StatsObserver>());
		}

		void setCallback(std::shared_ptr<StatsCallback> callback)
		{
			_callback = callback;
		}

	protected:
		void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override
		{
			if (_callback) {
				(*_callback)(report);
			}
		}

	private:
		std::shared_ptr<StatsCallback> _callback;
	};

}						

