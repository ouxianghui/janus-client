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
	using CreateSessionDescFailureCallback = std::function<void(const std::string& error)>;

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
				//(*cb)(error);
			}
		}

	private:
		std::shared_ptr<CreateSessionDescSuccessCallback> _success;
		std::shared_ptr<CreateSessionDescFailureCallback> _failure;
	};

	using SetSessionDescSuccessCallback = std::function<void()>;
	using SetSessionDescFailureCallback = std::function<void(const std::string& error)>;

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
				//(*_failure)(error);
			}
		}

	private:
		std::shared_ptr<SetSessionDescSuccessCallback> _success;
		std::shared_ptr<SetSessionDescFailureCallback> _failure;
	};

	using IceConnectionChangeCallback = std::function<void(webrtc::PeerConnectionInterface::IceConnectionState new_state)>;
	using IceGatheringChangeCallback = std::function<void(webrtc::PeerConnectionInterface::IceGatheringState new_state)> ;
	using IceCandidateCallback = std::function<void(const webrtc::IceCandidateInterface* candidate)>;
	using DataChannelCallback = std::function<void(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)>;
	using AddTrackCallback = std::function<void(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)>;
	using RemoveTrackCallback = std::function<void(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)>;

	class PCObserver : public webrtc::PeerConnectionObserver {
	public:
		void setIceConnectionChangeCallback(std::shared_ptr<IceConnectionChangeCallback> callback);

		void setIceGatheringChangeCallback(std::shared_ptr<IceGatheringChangeCallback> callback);

		void setIceCandidateCallback(std::shared_ptr<IceCandidateCallback> callback);

		void setDataChannelCallback(std::shared_ptr<DataChannelCallback> callback);

		void setAddTrackCallback(std::shared_ptr<AddTrackCallback> callback);

		void setRemoveTrackCallback(std::shared_ptr<RemoveTrackCallback> callback);

	protected:
		// Triggered when the SignalingState changed.
		void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

		// Triggered when media is received on a new stream from remote peer.
		void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		// Triggered when a remote peer closes a stream.
		void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		// Triggered when a remote peer opens a data channel.
		void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

		// Triggered when renegotiation is needed. For example, an ICE restart
		// has begun.
		void OnRenegotiationNeeded() override;

		// Called any time the legacy IceConnectionState changes.
		//
		// Note that our ICE states lag behind the standard slightly. The most
		// notable differences include the fact that "failed" occurs after 15
		// seconds, not 30, and this actually represents a combination ICE + DTLS
		// state, so it may be "failed" if DTLS fails while ICE succeeds.
		//
		// TODO(jonasolsson): deprecate and remove this.
		void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

		// Called any time the standards-compliant IceConnectionState changes.
		void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

		// Called any time the PeerConnectionState changes.
		void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

		// Called any time the IceGatheringState changes.
		void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

		// A new ICE candidate has been gathered.
		void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

		// Ice candidates have been removed.
		// TODO(honghaiz): Make this a pure method when all its subclasses
		// implement it.
		void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override;

		// Called when the ICE connection receiving status changes.
		void OnIceConnectionReceivingChange(bool receiving) override;

		// This is called when a receiver and its track are created.
		// TODO(zhihuang): Make this pure when all subclasses implement it.
		// Note: This is called with both Plan B and Unified Plan semantics. Unified
		// Plan users should prefer OnTrack, OnAddTrack is only called as backwards
		// compatibility (and is called in the exact same situations as OnTrack).
		void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
			const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;

		// This is called when signaling indicates a transceiver will be receiving
		// media from the remote endpoint. This is fired during a call to
		// SetRemoteDescription. The receiving track can be accessed by:
		// |transceiver->receiver()->track()| and its associated streams by
		// |transceiver->receiver()->streams()|.
		// Note: This will only be called if Unified Plan semantics are specified.
		// This behavior is specified in section 2.2.8.2.5 of the "Set the
		// RTCSessionDescription" algorithm:
		// https://w3c.github.io/webrtc-pc/#set-description
		void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

		// Called when signaling indicates that media will no longer be received on a
		// track.
		// With Plan B semantics, the given receiver will have been removed from the
		// PeerConnection and the track muted.
		// With Unified Plan semantics, the receiver will remain but the transceiver
		// will have changed direction to either sendonly or inactive.
		// https://w3c.github.io/webrtc-pc/#process-remote-track-removal
		// TODO(hbos,deadbeef): Make pure when all subclasses implement it.
		void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

	private:
		std::shared_ptr<IceConnectionChangeCallback> _iceConnectionChangeCallback;
		std::shared_ptr<IceGatheringChangeCallback> _iceGatheringChangeCallback;
		std::shared_ptr<IceCandidateCallback> _iceCandidateCallback;
		std::shared_ptr<DataChannelCallback> _dataChannelCallback;
		std::shared_ptr<AddTrackCallback> _addTrackCallback;
		std::shared_ptr<RemoveTrackCallback> _removeTrackCallback;
	};	

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
}						

