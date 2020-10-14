/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "webrtc_utils.h"
#include "logger/logger.h"

namespace vi {
	void PCObserver::setIceConnectionChangeCallback(std::shared_ptr<IceConnectionChangeCallback> callback)
	{
		_iceConnectionChangeCallback = callback;
	}

	void PCObserver::setIceGatheringChangeCallback(std::shared_ptr<IceGatheringChangeCallback> callback)
	{
		_iceGatheringChangeCallback = callback;
	}

	void PCObserver::setIceCandidateCallback(std::shared_ptr<IceCandidateCallback> callback)
	{
		_iceCandidateCallback = callback;
	}

	void PCObserver::setDataChannelCallback(std::shared_ptr<DataChannelCallback> callback)
	{
		_dataChannelCallback = callback;
	}

	void PCObserver::setAddTrackCallback(std::shared_ptr<AddTrackCallback> callback)
	{
		_addTrackCallback = callback;
	}

	void PCObserver::setRemoveTrackCallback(std::shared_ptr<RemoveTrackCallback> callback)
	{
		_removeTrackCallback = callback;
	}

	// Triggered when the SignalingState changed.
	void PCObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
	{
		TLOG("OnSignalingChange: {}", new_state);
	}

	// Triggered when media is received on a new stream from remote peer.
	void PCObserver::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) 
	{
		TLOG("OnAddStream: {}", stream->id().c_str());
	}

	// Triggered when a remote peer closes a stream.
	void PCObserver::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		TLOG("OnRemoveStream: {}", stream->id().c_str());
	}

	// Triggered when a remote peer opens a data channel.
	void PCObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
	{
		TLOG("OnDataChannel: {}", data_channel->id());
		if (_dataChannelCallback) {
			(*_dataChannelCallback)(data_channel);
		}
	}

	// Triggered when renegotiation is needed. For example, an ICE restart
	// has begun.
	void PCObserver::OnRenegotiationNeeded()
	{
		TLOG("OnRenegotiationNeeded");
	}

	// Called any time the legacy IceConnectionState changes.
	//
	// Note that our ICE states lag behind the standard slightly. The most
	// notable differences include the fact that "failed" occurs after 15
	// seconds, not 30, and this actually represents a combination ICE + DTLS
	// state, so it may be "failed" if DTLS fails while ICE succeeds.
	//
	// TODO(jonasolsson): deprecate and remove this.
	void PCObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
	{
		TLOG("OnIceConnectionChange: {}", new_state);
		if (_iceConnectionChangeCallback) {
			(*_iceConnectionChangeCallback)(new_state);
		}
	}

	// Called any time the standards-compliant IceConnectionState changes.
	void PCObserver::OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
	{
		TLOG("OnStandardizedIceConnectionChange: {}", new_state);
	}

	// Called any time the PeerConnectionState changes.
	void PCObserver::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
	{
		TLOG("OnConnectionChange: {}", (int)new_state);
	}

	// Called any time the IceGatheringState changes.
	void PCObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
	{
		TLOG("OnIceGatheringChange: {}", (int)new_state);
		if (_iceGatheringChangeCallback) {
			(*_iceGatheringChangeCallback)(new_state);
		}
	}

	// A new ICE candidate has been gathered.
	void PCObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
	{
		std::string sdp;
		candidate->ToString(&sdp);
		TLOG("OnIceCandidate: {}", sdp.c_str());
		if (_iceCandidateCallback) {
			(*_iceCandidateCallback)(candidate);
		}
	}

	// Ice candidates have been removed.
	// TODO(honghaiz): Make this a pure method when all its subclasses
	// implement it.
	void PCObserver::OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) 
	{
		for (const auto& c : candidates) {
			TLOG("OnIceCandidatesRemoved: {}", c.ToString().c_str());
		}
	}

	// Called when the ICE connection receiving status changes.
	void PCObserver::OnIceConnectionReceivingChange(bool receiving) 
	{
		TLOG("OnIceConnectionReceivingChange: {}", receiving);
	}

	// This is called when a receiver and its track are created.
	// TODO(zhihuang): Make this pure when all subclasses implement it.
	// Note: This is called with both Plan B and Unified Plan semantics. Unified
	// Plan users should prefer OnTrack, OnAddTrack is only called as backwards
	// compatibility (and is called in the exact same situations as OnTrack).
	void PCObserver::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
		const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) 
	{
		for (const auto& id : receiver->stream_ids()) {
			TLOG("OnAddTrack, stream id: {}", id.c_str());
		}
	}

	// This is called when signaling indicates a transceiver will be receiving
	// media from the remote endpoint. This is fired during a call to
	// SetRemoteDescription. The receiving track can be accessed by:
	// |transceiver->receiver()->track()| and its associated streams by
	// |transceiver->receiver()->streams()|.
	// Note: This will only be called if Unified Plan semantics are specified.
	// This behavior is specified in section 2.2.8.2.5 of the "Set the
	// RTCSessionDescription" algorithm:
	// https://w3c.github.io/webrtc-pc/#set-description
	void PCObserver::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
	{
		if (auto& receiver = transceiver->receiver()) {
			TLOG("OnTrack, receiver id: {}", receiver->id().c_str());
		}

		if (auto& sender = transceiver->sender()) {
			TLOG("OnTrack, sender id: {}", sender->id().c_str());
		}

		if (_addTrackCallback) {
			(*_addTrackCallback)(transceiver);
		}
	}

	// Called when signaling indicates that media will no longer be received on a
	// track.
	// With Plan B semantics, the given receiver will have been removed from the
	// PeerConnection and the track muted.
	// With Unified Plan semantics, the receiver will remain but the transceiver
	// will have changed direction to either sendonly or inactive.
	// https://w3c.github.io/webrtc-pc/#process-remote-track-removal
	// TODO(hbos,deadbeef): Make pure when all subclasses implement it.
	void PCObserver::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
	{
		if (receiver) {
			TLOG("OnRemoveTrack, receiver id: {}", receiver->id().c_str());
		}

		if (_removeTrackCallback) {
			(*_removeTrackCallback)(receiver);
		}
	}
}