/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <string>
#include "x2struct.hpp"

namespace vi {
	using namespace x2struct;

	struct JanusRequest {
		std::string janus;
		std::string token;
		std::string apisecret;
		std::string transaction;

		XTOSTRUCT(O(janus, token, apisecret, transaction));
	};

	struct JanusResponse {
		std::string janus;
		std::string transaction;		
		int64_t session_id = -1;
		int64_t sender = -1;
		XTOSTRUCT(O(janus, transaction, session_id, sender));
	};

	struct JanusData {
		int64_t id;

		XTOSTRUCT(O(id));
	};

	struct Jsep {
		std::string type;
		std::string sdp;

		XTOSTRUCT(O(type, sdp));
	};

	struct JanusError {
		int64_t code;
		std::string reason;

		XTOSTRUCT(O(code), O(reason));
	};

	struct PublisherData {
		int64_t id;
		std::string display;
		std::string audio_codec;
		std::string video_codec;
		bool simulcast;
		bool talking;

		XTOSTRUCT(O(id, display, audio_codec, video_codec, simulcast, talking));
	};

	struct RoomData {
		int64_t room;
		std::string description;
		int64_t max_publishers;
		int64_t bitrate;
		bool bitrate_cap;
		int64_t fir_freq;
		std::string audiocodec;
		std::string videocodec;
		bool record;
		std::string record_dir;
		bool lock_record;
		int64_t num_participants;

		XTOSTRUCT(O(room,
			description,
			max_publishers,
			bitrate,
			bitrate_cap,
			fir_freq,
			audiocodec,
			videocodec,
			record,
			record_dir,
			lock_record,
			num_participants));
	};

	struct ParticipantData {
		int64_t id;
		std::string display;
		bool publisher;
		bool talking;

		XTOSTRUCT(O(id,
			display,
			publisher,
			talking));
	};

	struct EventData {
		std::string videoroom;
		int64_t room;
		std::string description;
		bool permanent;
		bool exists;
		int64_t id;
		int64_t private_id;
		std::vector<PublisherData> publishers;
		int64_t unpublished;
		int64_t error_code;
		std::string audio_codec;
		std::string video_codec;
		std::string display;
		std::string started;
		std::string paused;
		//std::string leaving;
		std::string left;
		std::string configured;
		std::string switched;

		XTOSTRUCT(O(videoroom,
			room,
			description,
			permanent,
			exists,
			id,
			private_id,
			publishers,
			unpublished,
			error_code,
			audio_codec,
			video_codec,
			display,
			started,
			paused,
			//leaving,
			left,
			configured,
			switched
		));
	};

	struct PluginData {
		std::string plugin;
		EventData data;

		XTOSTRUCT(O(plugin, data));
	};

	//struct JanusResponseX {
	//	std::string janus;
	//	int64_t session_id = -1;
	//	int64_t sender = -1;
	//	//CandidateData candidate;
	//	std::string reason;
	//	std::string type;
	//	//bool receiving;
	//	//bool uplink;
	//	//bool lost;
	//	//PluginData plugindata;
	//	JanusError error;
	//	//JanusData data;
	//	//Jsep jsep;
	//	std::string transaction;
	//	std::vector<RoomData> list;
	//	std::vector<ParticipantData> participants;

	//};

	struct ReconnectRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct KeepAliveRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct AttachRequest : public JanusRequest {
		int64_t session_id;
		std::string plugin;
		std::string opaque_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, plugin, opaque_id));
	};

	struct AttachResponse : public JanusResponse {
		JanusData data;
		XTOSTRUCT(I(JanusResponse), O(data));
	};

	struct DetachRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id));
	};

	struct HangupRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id));
	};

	struct HangupResponse : public JanusResponse {
		std::string reason;
		XTOSTRUCT(I(JanusResponse), O(reason));
	};

	struct MediaResponse : public JanusResponse {
		std::string type;
		bool receiving;
		XTOSTRUCT(I(JanusResponse), O(type, receiving));
	};

	struct SlowlinkResponse : public JanusResponse {
		bool uplink;
		bool lost;
		XTOSTRUCT(I(JanusResponse), O(uplink, lost));
	};

	struct EventResponse : public JanusResponse {
		PluginData plugindata;
		Jsep jsep;
		XTOSTRUCT(I(JanusResponse), O(plugindata, jsep));
	};

	struct DestroyRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct CreateSessionResponse : public JanusResponse {
		JanusData data;
		XTOSTRUCT(I(JanusResponse), O(data));
	};

	struct CandidateData {
		std::string candidate;
		std::string sdpMid;
		int64_t sdpMLineIndex;
		bool completed;
		XTOSTRUCT(O(candidate, sdpMid, sdpMLineIndex, completed));
	};

	struct TrickleRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		CandidateData candidate;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, candidate));
	};

	struct TrickleResponse : public JanusResponse {
		CandidateData candidate;
		XTOSTRUCT(I(JanusResponse), O(candidate));

	};
	struct MessageRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		std::string body;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, body));
	};

	struct JsepRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		std::string body;
		std::string jsep;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, body, jsep));
	};

	namespace vr {
		/*
		 * This covers almost all the synchronous requests.All the asynchronous requests,
		 * plus a couple of additional synchronous requests we'll cover later, refer
		 * to participants instead, namely on how they can publish, subscribe, or
		 * more in general manage the media streams they may be sending or receiving.
		 *
		 * Considering the different nature of publishers and subscribers in the room,
		 * and more importantly how you establish PeerConnections in the respective
		 * cases, their API requests are addressed in separate subsections.
		 *
		 * \subsection vroompub VideoRoom Publishers
		 *
		 * In a VideoRoom, publishers are those participant handles that are able
		 * (although may choose not to, more on this later) publish media in the
		 * room, and as such become feeds that you can subscribe to.
		 *
		 * To specify that a handle will be associated with a publisher, you must use
		 * the \c join request with \c ptype set to \c publisher(note that, as it
		 * will be explained later, you can also use \c joinandconfigure for the
		 * purpose).The exact syntax of the request is the following :
		*/
		//\verbatim
		//{
		//	"request" : "join",
		//	"ptype" : "publisher",
		//	"room" : <unique ID of the room to join>,
		//	"id" : < unique ID to register for the publisher; optional, will be chosen by the plugin if missing > ,
		//	"display" : "<display name for the publisher; optional>",
		//	"token" : "<invitation token, in case the room has an ACL; optional>"
		//}
		//\endverbatim
		struct PublisherJoinRequest {
			std::string request = "join";
			int64_t room;
			std::string ptype = "publisher";
			std::string display;
			std::string token;
			XTOSTRUCT(O(request, room, ptype, display, token));
		};

		/* In a VideoRoom, subscribers are NOT participants, but simply handles
		 * that will be used exclusively to receive media from a specific publisher
		 * in the room.Since they're not participants per se, they're basically
		 * streams that can be(and typically are) associated to publisher handles
		 * as the ones we introduced in the previous section, whether active or not.
		 * In fact, the typical use case is publishers being notified about new
		 * participants becoming active in the room, and as a result new subscriber
		 * sessions being created to receive their media streams; as soon as the
		 * publisher goes away, the subscriber handle is removed as well.As such,
		 *these subscriber sessions are dependent on feedback obtained by
		 * publishers, and can't exist on their own, unless you feed them the
		 * right info out of band(which is impossible in rooms configured with require_pvtid).
		 *
		 * To specify that a handle will be associated with a subscriber, you must use
		 * the \c join request with \c ptype set to \c subscriber and specify which
		 * feed to subscribe to.The exact syntax of the request is the following :
		*/
		//\verbatim
		//{
		//	"request" : "join",
		//	"ptype" : "subscriber",
		//	"room" : <unique ID of the room to subscribe in>,
		//	"feed" : < unique ID of the publisher to subscribe to; mandatory > ,
		//	"private_id" : < unique ID of the publisher that originated this request; optional, unless mandated by the room configuration > ,
		//	"close_pc" : < true | false, depending on whether or not the PeerConnection should be automatically closed when the publisher leaves; true by default > ,
		//	"audio" : < true | false, depending on whether or not audio should be relayed; true by default > ,
		//	"video" : < true | false, depending on whether or not video should be relayed; true by default > ,
		//	"data" : < true | false, depending on whether or not data should be relayed; true by default > ,
		//	"offer_audio" : < true | false; whether or not audio should be negotiated; true by default if the publisher has audio > ,
		//	"offer_video" : < true | false; whether or not video should be negotiated; true by default if the publisher has video > ,
		//	"offer_data" : < true | false; whether or not datachannels should be negotiated; true by default if the publisher has datachannels > ,
		//	"substream" : < substream to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"temporal" : < temporal layers to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"fallback" : <How much time(in us, default 250000) without receiving packets will make us drop to the substream below>,
		//	"spatial_layer" : < spatial layer to receive(0 - 2), in case VP9 - SVC is enabled; optional > ,
		//	"temporal_layer" : < temporal layers to receive(0 - 2), in case VP9 - SVC is enabled; optional >
		//}
		//\endverbatim

		struct SubscriberJoinRequest {
			std::string request = "join";
			int64_t room;
			std::string ptype = "subscriber";
			int64_t feed;
			int64_t private_id;

			XTOSTRUCT(O(request,
				room,
				ptype,
				feed,
				private_id
			));
		};

		struct UnpublishRequest {
			std::string request = "unpublish";
			XTOSTRUCT(O(request));
		};

		struct StartRequest {
			std::string request = "start";
			int64_t room;
			XTOSTRUCT(O(request, room));
		};

		struct PauseRequest {
			std::string request = "pause";
			int64_t room;
			XTOSTRUCT(O(request, room));
		};

		/*To conclude, you can leave a room you previously joined as publisher
		* using the \c leave request.This will also implicitly unpublish you
		* if you were an active publisher in the room.The \c leave request
		* looks like follows :
		*/
		//\verbatim
		//{
		//	"request" : "leave"
		//}
		//\endverbatim
		/*
		* If successful, the response will look like this:
		*
		\verbatim
		{
			"videoroom" : "event",
			"leaving" : "ok"
		}
		\endverbatim
		*/

		/*
		* Finally, to stop the subscription to the mountpoint and tear down the
		* related PeerConnection, you can use the \c leave request.Since context
		* is implicit, no other argument is required :
		*
		\verbatim
		{
			"request" : "leave"
		}
		\endverbatim
		*/

		/* If successful, the plugin will attempt to tear down the PeerConnection,
		* and will send back a \c left event:
		*
		\verbatim
		{
			"videoroom" : "event",
			"left" : "ok",
		}
		\endverbatim
		*/

		struct LeaveRequest {
			std::string request = "leave";
			XTOSTRUCT(O(request));
		};

		//\verbatim
		//{
		//	"request" : "create",
		//	"room" : <unique numeric ID, optional, chosen by plugin if missing>,
		//	"permanent" : <true | false, whether the room should be saved in the config file, default = false>,
		//	"description" : "<pretty name of the room, optional>",
		//	"secret" : "<password required to edit/destroy the room, optional>",
		//	"pin" : "<password required to join the room, optional>",
		//	"is_private" : <true | false, whether the room should appear in a list request>,
		//	"allowed" : [array of string tokens users can use to join this room, optional],
		//	...
		//}
		//\endverbatim
		struct CreateRoomRequest {
			std::string request = "create";
			int64_t room;
			bool permanent;
			std::string description;
			std::string secret;
			std::string pin;
			bool is_private = false;
			std::vector<std::string> allowed;

			XTOSTRUCT(O(request),
				O(room),
				O(permanent),
				O(description),
				O(secret),
				O(pin),
				O(is_private),
				O(allowed));
		};

		struct RoomActionData {
			std::string videoroom;
			int64_t room;
			bool permanent;

			XTOSTRUCT(O(videoroom, room, permanent));
		};

		struct RoomActionPluginData {
			std::string plugin;
			RoomActionData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct RoomActionResponse : public JanusResponse {
			RoomActionPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
		};

		/*
		 * For the sake of brevity, not all of the available settings are listed
		 * here.You can refer to the name of the properties in the configuration
		 * file as a reference, as the ones used to programmatically create a new
		 * room are exactly the same.
		 *
		 * A successful creation procedure will result in a \c created response :
		*
		\verbatim
		*/
		//{
		//	"videoroom" : "created",
		//	"room" : <unique numeric ID>,
		//	"permanent" : <true if saved to config file, false if not>
		//}
		//\endverbatim

		/*
		 * Notice that, in general, all users can create rooms. If you want to
		 * limit this functionality, you can configure an admin \c admin_key in
		 * the plugin settings. When configured, only "create" requests that
		 * include the correct \c admin_key value in an "admin_key" property
		 * will succeed, and will be rejected otherwise. Notice that you can
		 * optionally extend this functionality to RTP forwarding as well, in
		 * order to only allow trusted clients to use that feature.
		 *
		 * Once a room has been created, you can still edit some (but not all)
		 * of its properties using the \c edit request. This allows you to modify
		 * the room description, secret, pin and whether it's private or not: you
		 * won't be able to modify other more static properties, like the room ID,
		 * the sampling rate, the extensions-related stuff and so on. If you're
		 * interested in changing the ACL, instead, check the \c allowed message.
		 * An \c edit request has to be formatted as follows:
		 */
		 //\verbatim
		 //{
		 //	"request" : "edit",
		 //	"room" : <unique numeric ID of the room to edit>,
		 //	"secret" : "<room secret, mandatory if configured>",
		 //	"new_description" : "<new pretty name of the room, optional>",
		 //	"new_secret" : "<new password required to edit/destroy the room, optional>",
		 //	"new_pin" : "<new password required to join the room, optional>",
		 //	"new_is_private" : <true | false, whether the room should appear in a list request>,
		 //	"new_require_pvtid" : <true | false, whether the room should require private_id from subscribers>,
		 //	"new_bitrate" : <new bitrate cap to force on all publishers(except those with custom overrides)>,
		 //	"new_fir_freq" : <new period for regular PLI keyframe requests to publishers>,
		 //	"new_publishers" : <new cap on the number of concurrent active WebRTC publishers>,
		 //	"new_lock_record" : <true | false, whether recording state can only be changed when providing the room secret>,
		 //	"permanent" : <true | false, whether the room should be also removed from the config file, default = false>
		 //}
		 //\endverbatim
		struct EditRoomRequest {
			std::string request = "edit";
			int64_t room;
			std::string secret;
			std::string new_description;
			std::string new_secret;
			std::string new_pin;
			bool new_is_private;
			bool new_require_pvtid;
			int64_t new_bitrate;
			int64_t new_fir_freq;
			bool new_lock_record;
			int64_t new_publishers;
			bool permanent;

			XTOSTRUCT(O(request,
				room,
				secret,
				new_description,
				new_secret,
				new_pin,
				new_is_private,
				new_require_pvtid,
				new_bitrate,
				new_fir_freq,
				new_lock_record,
				new_publishers,
				permanent));
		};

		/*
		 * This will also result in a \c destroyed event being sent to all the
		 * participants in the video room, which will look like this:
		 */
		 //\verbatim
		 //{
		 //	"request" : "destroy",
		 //	"room" : <unique numeric ID of the room to destroy>,
		 //	"secret" : "<room secret, mandatory if configured>",
		 //	"permanent" : <true | false, whether the room should be also removed from the config file, default = false>
		 //}
		 //\endverbatim
		struct DestroyRoomRequest {
			std::string request = "destroy";
			int64_t room;
			std::string secret;
			bool permanent;

			XTOSTRUCT(O(request,
				room,
				secret,
				permanent));
		};

		/*
		 * You can check whether a room exists using the \c exists request,
		 *which has to be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "exists",
		//	"room" : <unique numeric ID of the room to check>
		//}
		//\endverbatim
		struct ExistsRequest {
			std::string request = "exists";
			int64_t room;

			XTOSTRUCT(O(request, room));
		};

		/*
		 * You can configure whether to check tokens or add / remove people who can join
		 * a room using the \c allowed request, which has to be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "allowed",
		//	"secret" : "<room secret, mandatory if configured>",
		//	"action" : "enable|disable|add|remove",
		//	"room" : <unique numeric ID of the room to update>,
		//	"allowed" : [
		//		// Array of strings (tokens users might pass in "join", only for add|remove)
		//	]
		//}
		//\endverbatim
		struct AllowedRequest {
			std::string request = "allowed";
			std::string secret;
			std::string action;
			int64_t room;
			std::vector<std::string> allowed;

			XTOSTRUCT(O(request,
				secret,
				action,
				room,
				allowed));
		};

		/*
		 * If you're the administrator of a room (that is, you created it and have access
		 * to the secret) you can kick participants using the \c kick request.Notice
		 * that this only kicks the user out of the room, but does not prevent them from
		 * re - joining: to ban them, you need to first remove them from the list of
		 * authorized users(see \c allowed request) and then \c kick them.The \c kick
		 * request has to be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "kick",
		//	"secret" : "<room secret, mandatory if configured>",
		//	"room" : <unique numeric ID of the room>,
		//	"id" : <unique numeric ID of the participant to kick>
		//}
		//\endverbatim
		struct KickRequest {
			std::string request = "kick";
			std::string secret;
			int64_t room;
			int64_t id;

			XTOSTRUCT(O(request,
				secret,
				room,
				id));
		};

		/*
		* To get a list of the available rooms(excluded those configured or
		*created as private rooms) you can make use of the \c list request,
		*which has to be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "list"
		//}
		//\endverbatim
		struct FetchRoomsListRequest {
			std::string request = "list";

			XTOSTRUCT(O(request));
		};

		/*
		* To get a list of the participants in a specific room, instead, you
		* can make use of the \c listparticipants request, which has to be
		* formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "listparticipants",
		//	"room" : <unique numeric ID of the room>
		//}
		//\endverbatim
		struct FetchParticipantsRequest {
			std::string request = "listparticipants";
			int64_t room;

			XTOSTRUCT(O(request, room));
		};

		/*
		 * If you're interested in publishing media within a room, you can do that
		 * with a \c publish request. This request MUST be accompanied by a JSEP
		 * SDP offer to negotiate a new PeerConnection. The plugin will match it
		 * to the room configuration (e.g., to make sure the codecs you negotiated
		 * are allowed in the room), and will reply with a JSEP SDP answer to
		 * close the circle and complete the setup of the PeerConnection. As soon
		 * as the PeerConnection has been established, the publisher will become
		 * active, and a new active feed other participants can subscribe to.
		 *
		 * The syntax of a \c publish request is the following:
		 */
		 //\verbatim
		 //{
		 //	"request" : "publish",
		 //	"audio" : < true | false, depending on whether or not audio should be relayed; true by default > ,
		 //	"video" : < true | false, depending on whether or not video should be relayed; true by default > ,
		 //	"data" : < true | false, depending on whether or not data should be relayed; true by default > ,
		 //	"audiocodec" : "<audio codec to prefer among the negotiated ones; optional>",
		 //	"videocodec" : "<video codec to prefer among the negotiated ones; optional>",
		 //	"bitrate" : < bitrate cap to return via REMB; optional, overrides the global room value if present > ,
		 //	"record" : < true | false, whether this publisher should be recorded or not; optional > ,
		 //	"filename" : "<if recording, the base path/file to use for the recording files; optional>",
		 //	"display" : "<new display name to use in the room; optional>",
		 //	"audio_level_average" : "<if provided, overrided the room audio_level_average for this user; optional>",
		 //	"audio_active_packets" : "<if provided, overrided the room audio_active_packets for this user; optional>"
		 //}
		 //\endverbatim
		struct PublishRequest {
			std::string request = "publish";
			bool video;
			bool audio;
			bool data;
			std::string audiocodec;
			std::string videocodec;
			int64_t bitrate;
			bool record;
			std::string filename;
			std::string display;
			int64_t audio_level_average;
			int64_t audio_active_packets;

			XTOSTRUCT(O(request,
				video,
				audio,
				data,
				audiocodec,
				videocodec,
				bitrate,
				record,
				filename,
				display,
				audio_level_average,
				audio_active_packets
			));
		};

		/*
		 * For more drill - down manipulations of a subscription, a \c configure
		 * request can be used instead.This request allows subscribers to dynamically
		 * change some properties associated to their media subscription, e.g.,
		 *in terms of what should and should not be sent at a specific time.A
		 * \c configure request must be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "configure",
		//	"audio" : < true | false, depending on whether audio should be relayed or not; optional > ,
		//	"video" : < true | false, depending on whether video should be relayed or not; optional > ,
		//	"data" : < true | false, depending on whether datachannel messages should be relayed or not; optional > ,
		//	"substream" : < substream to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"temporal" : < temporal layers to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"fallback" : <How much time(in us, default 250000) without receiving packets will make us drop to the substream below>,
		//	"spatial_layer" : < spatial layer to receive(0 - 2), in case VP9 - SVC is enabled; optional > ,
		//	"temporal_layer" : < temporal layers to receive(0 - 2), in case VP9 - SVC is enabled; optional > ,
		//	"audio_level_average" : "<if provided, overrides the room audio_level_average for this user; optional>",
		//	"audio_active_packets" : "<if provided, overrides the room audio_active_packets for this user; optional>"
		//}
		//\endverbatim

		struct SubscriberConfigureRequest {
			std::string request = "configure";
			bool video;
			bool audio;
			bool data;
			int64_t substream;
			int64_t temporal;
			int64_t fallback;
			int64_t spatial_layer;
			int64_t temporal_layer;
			int64_t audio_level_average;
			int64_t audio_active_packets;

			XTOSTRUCT(O(request,
				video,
				audio,
				data,
				substream,
				temporal,
				fallback,
				spatial_layer,
				temporal_layer,
				audio_level_average,
				audio_active_packets
			));
		};

		/*
		 * Notice that the same event will also be sent whenever the publisher
		 * feed disappears for reasons other than an explicit \c unpublish, e.g.,
		 *because the handle was closed or the user lost their connection.
		 * Besides, notice that you can publish and unpublish multiple times
		 * within the context of the same publisher handle.
		 *
		 * As anticipated above, you can use a request called \c configure to
		 * tweak some of the properties of an active publisher session.This
		 * request must be formatted as follows :
		*/
		//\verbatim
		//{
		//	"request" : "configure",
		//	"audio" : < true | false, depending on whether or not audio should be relayed; true by default > ,
		//	"video" : < true | false, depending on whether or not video should be relayed; true by default > ,
		//	"data" : < true | false, depending on whether or not data should be relayed; true by default > ,
		//	"bitrate" : < bitrate cap to return via REMB; optional, overrides the global room value if present(unless bitrate_cap is set) > ,
		//	"keyframe" : <true | false, whether we should send this publisher a keyframe request>,
		//	"record" : < true | false, whether this publisher should be recorded or not; optional > ,
		//	"filename" : "<if recording, the base path/file to use for the recording files; optional>",
		//	"display" : "<new display name to use in the room; optional>",
		//	"audio_active_packets" : "<new audio_active_packets to overwrite in the room one; optional>",
		//	"audio_level_average" : "<new audio_level_average to overwrite the room one; optional>",
		//}
		//\endverbatim
		struct PublisherConfigureRequest {
			std::string request = "configure";
			bool video;
			bool audio;
			bool data;
			int64_t bitrate;
			bool keyframe;
			bool record;
			std::string filename;
			std::string display;
			int64_t audio_level_average;
			int64_t audio_active_packets;

			XTOSTRUCT(O(request,
				video,
				audio,
				data,
				bitrate,
				keyframe,
				record,
				filename,
				display,
				audio_level_average,
				audio_active_packets
			));
		};

		//\verbatim
		//{
		//	"request" : "switch",
		//	"feed" : < unique ID of the new publisher to switch to; mandatory > ,
		//	"audio" : < true | false, depending on whether audio should be relayed or not; optional > ,
		//	"video" : < true | false, depending on whether video should be relayed or not; optional > ,
		//	"data" : < true | false, depending on whether datachannel messages should be relayed or not; optional >
		//}
		//\endverbatim
		struct SwitchRequest {
			std::string request = "switch";
			int64_t feed;
			bool video;
			bool audio;
			bool data;

			XTOSTRUCT(O(request,
				feed,
				video,
				audio,
				data
			));
		};
	}
	

}