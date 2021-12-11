#pragma once

#include <memory>
#include "api/set_remote_description_observer_interface.h"
#include "api/rtc_error.h"
#include "api/jsep.h"
#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "api/stats/rtc_stats_report.h"
#include "api/peer_connection_interface.h"
#include "api/media_stream_interface.h"
#include "api/data_channel_interface.h"
#include "api/jsep.h"
#include "api/rtp_receiver_interface.h"

namespace vi {
	class IWebrtcEventHandler 
		: public webrtc::PeerConnectionObserver
		, public webrtc::DataChannelObserver
		, public webrtc::DtmfSenderObserverInterface
	{
	public:
		virtual ~IWebrtcEventHandler() {}
		
		// webrtc::PeerConnectionObserver
		// 
		// Triggered when the SignalingState changed.
		void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}

		// Triggered when media is received on a new stream from remote peer.
		void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}

		// Triggered when a remote peer closes a stream.
		void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}

		// Triggered when a remote peer opens a data channel.
		void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {}

		// Triggered when renegotiation is needed. For example, an ICE restart
		// has begun.
		// TODO(hbos): Delete in favor of OnNegotiationNeededEvent() when downstream
		// projects have migrated.
		void OnRenegotiationNeeded() override {}

		// Used to fire spec-compliant onnegotiationneeded events, which should only
		// fire when the Operations Chain is empty. The observer is responsible for
		// queuing a task (e.g. Chromium: jump to main thread) to maybe fire the
		// event. The event identified using `event_id` must only fire if
		// PeerConnection::ShouldFireNegotiationNeededEvent() returns true since it is
		// possible for the event to become invalidated by operations subsequently
		// chained.
		void OnNegotiationNeededEvent(uint32_t event_id) override {}

		// Called any time the legacy IceConnectionState changes.
		//
		// Note that our ICE states lag behind the standard slightly. The most
		// notable differences include the fact that "failed" occurs after 15
		// seconds, not 30, and this actually represents a combination ICE + DTLS
		// state, so it may be "failed" if DTLS fails while ICE succeeds.
		//
		// TODO(jonasolsson): deprecate and remove this.
		void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}

		// Called any time the standards-compliant IceConnectionState changes.
		void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}

		// Called any time the PeerConnectionState changes.
		void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {}

		// Called any time the IceGatheringState changes.
		void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}

		// A new ICE candidate has been gathered.
		void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {}

		// Gathering of an ICE candidate failed.
		// See https://w3c.github.io/webrtc-pc/#event-icecandidateerror
		// `host_candidate` is a stringified socket address.
		void OnIceCandidateError(const std::string& host_candidate,
			const std::string& url,
			int error_code,
			const std::string& error_text) override {}

		// Gathering of an ICE candidate failed.
		// See https://w3c.github.io/webrtc-pc/#event-icecandidateerror
		void OnIceCandidateError(const std::string& address,
			int port,
			const std::string& url,
			int error_code,
			const std::string& error_text) override {}

		// Ice candidates have been removed.
		// TODO(honghaiz): Make this a pure virtual method when all its subclasses
		// implement it.
		void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override {}

		// Called when the ICE connection receiving status changes.
		void OnIceConnectionReceivingChange(bool receiving) {}

		// Called when the selected candidate pair for the ICE connection changes.
		void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent& event) override {}

		// This is called when a receiver and its track are created.
		// TODO(zhihuang): Make this pure virtual when all subclasses implement it.
		// Note: This is called with both Plan B and Unified Plan semantics. Unified
		// Plan users should prefer OnTrack, OnAddTrack is only called as backwards
		// compatibility (and is called in the exact same situations as OnTrack).
		void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override {}

		// This is called when signaling indicates a transceiver will be receiving
		// media from the remote endpoint. This is fired during a call to
		// SetRemoteDescription. The receiving track can be accessed by:
		// `transceiver->receiver()->track()` and its associated streams by
		// `transceiver->receiver()->streams()`.
		// Note: This will only be called if Unified Plan semantics are specified.
		// This behavior is specified in section 2.2.8.2.5 of the "Set the
		// RTCSessionDescription" algorithm:
		// https://w3c.github.io/webrtc-pc/#set-description
		void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override {}

		// Called when signaling indicates that media will no longer be received on a
		// track.
		// With Plan B semantics, the given receiver will have been removed from the
		// PeerConnection and the track muted.
		// With Unified Plan semantics, the receiver will remain but the transceiver
		// will have changed direction to either sendonly or inactive.
		// https://w3c.github.io/webrtc-pc/#process-remote-track-removal
		// TODO(hbos,deadbeef): Make pure virtual when all subclasses implement it.
		void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {}

		// Called when an interesting usage is detected by WebRTC.
		// An appropriate action is to add information about the context of the
		// PeerConnection and write the event to some kind of "interesting events"
		// log function.
		// The heuristics for defining what constitutes "interesting" are
		// implementation-defined.
		void OnInterestingUsage(int usage_pattern) override {}


		// webrtc::DataChannelObserver

		void OnStateChange() override {}

		//  A data buffer was successfully received.
		void OnMessage(const webrtc::DataBuffer& buffer) override {}

		// The data channel's buffered_amount has changed.
		void OnBufferedAmountChange(uint64_t sent_data_size) override {}


		// webrtc::DtmfSenderObserverInterface

		// Triggered when DTMF |tone| is sent.
		// If |tone| is empty that means the DtmfSender has sent out all the given
		// tones.
		// The callback includes the state of the tone buffer at the time when
		// the tone finished playing.
		void OnToneChange(const std::string& tone, const std::string& tone_buffer) override {}
	};
}