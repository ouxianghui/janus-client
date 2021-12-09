/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-11
 **/

#pragma once

#include "message_models.h"
#include "json/jsonable.hpp"
#include "absl/types/optional.h"

namespace vi {
	namespace vr {
		struct Publisher {
			absl::optional<int64_t> id;
			absl::optional<std::string> display;
			absl::optional<bool> talking;

			/*{
			 *	"type" : "<type of published stream #1 (audio|video|data)" > ,
			 *	"mindex" : "<unique mindex of published stream #1>",
			 *	"mid" : "<unique mid of of published stream #1>",
			 *	"disabled" : <if true, it means this stream is currently inactive / disabled(and so codec, description, etc.will be missing)>,
			 *	"codec" : "<codec used for published stream #1>",
			 *	"description" : "<text description of published stream #1, if any>",
			 *	"simulcast" : "<true if published stream #1 uses simulcast (VP8 and H.264 only)>",
			 *	"svc" : "<true if published stream #1 uses SVC (VP9 only)>",
			 *	"talking" : <true | false, whether the publisher stream has audio activity or not (only if audio levels are used)>,
			}*/
			struct Stream {
				absl::optional<std::string> type;
				absl::optional<int64_t> mindex;
				absl::optional<std::string> mid;
				absl::optional<bool> disabled;
				absl::optional<std::string> codec;
				absl::optional<std::string> description;
				absl::optional<bool> simulcast;
				absl::optional<bool> svc;
				absl::optional<bool> talking;

				FIELDS_MAP("type", type, "mindex", mindex, "mid", mid, "disabled", disabled, "codec", codec, "description", description, "simulcast", simulcast, "svc", svc, "talking", talking);
			};

			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("id", id, "display", display, "talking", talking, "streams", streams);
		};

		// Video Room event
		struct JoiningData {
			absl::optional<int64_t> id;
			absl::optional<std::string> display;

			FIELDS_MAP("id", id, "display", display);
		};

		struct EventData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> error_code;
			absl::optional<std::string> error;
			absl::optional<JoiningData> joining;
			absl::optional<std::string> configured;
			absl::optional<std::vector<Publisher>> publishers;
			absl::optional<int64_t> unpublished;
			absl::optional<int64_t> leaving;
			absl::optional<std::string> started;
			absl::optional<std::string> paused;
			absl::optional<std::string> switched;
			absl::optional<int64_t> id;
			absl::optional<std::string> left;
			absl::optional<std::string> audio_codec;
			absl::optional<std::string> video_codec;

			FIELDS_MAP("videoroom", videoroom,
				"error_code", error_code,
				"error", error,
				"joining", joining,
				"configured", configured,
				"publishers", publishers,
				"unpublished", unpublished,
				"leaving", leaving,
				"started", started,
				"paused", paused,
				"switched", switched,
				"id", id,
				"left", left,
				"audio_codec", audio_codec,
				"video_codec", video_codec
			);
		};

		struct EventPluginData {
			absl::optional<std::string> plugin;
			absl::optional<EventData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct VideoRoomEvent {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<EventPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
		};

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
			absl::optional<std::string> request = "join";
			absl::optional<std::string> ptype = "publisher";	
			absl::optional<int64_t> room;
			absl::optional<std::string> display;
			absl::optional<std::string> token;

			FIELDS_MAP("request", request, "ptype", ptype, "room", room, "display", display, "token", token);
		};

		struct Attendee {
			absl::optional<int64_t> id;
			absl::optional<std::string> display;

			FIELDS_MAP("id", id, "display", display);
		};

		struct PublisherJoinData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;
			absl::optional<std::string> description;
			absl::optional<int64_t> id;
			absl::optional<int64_t> private_id;
			absl::optional<std::vector<Publisher>> publishers;
			absl::optional<std::vector<Attendee>> attendees;

			FIELDS_MAP("videoroom", videoroom, "room", room, "description", description, "id", id, "private_id", private_id, "publishers", publishers, "attendees", attendees);
		};

		struct PublisherJoinPluginData {
			absl::optional<std::string> plugin;
			absl::optional<PublisherJoinData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct PublisherJoinEvent {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<PublisherJoinPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
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
		//  "streams" : [
		//  {
		//  	"feed_id" : <unique ID of publisher owning the stream to subscribe to>,
		//  		"mid" : "<unique mid of the publisher stream to subscribe to; optional>"
		//  		// Optionally, simulcast or SVC targets (defaults if missing)
		//  },
		//  	// Other streams to subscribe to
		//  ]
		//}
		//\endverbatim

		struct SubscriberJoinRequest {
			absl::optional<std::string> request = "join";
			absl::optional<int64_t> room;
			absl::optional<std::string> ptype = "subscriber";
			absl::optional<int64_t> feed;
			absl::optional<int64_t> private_id;

			struct Stream {
				absl::optional<int64_t> feed;
				absl::optional<std::string> mid;
				
				FIELDS_MAP("feed", feed, "mid", mid);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("request", request,
				"room", room,
				"ptype", ptype,
				"feed", feed,
				"private_id", private_id,
				"streams", streams
			);
		};

		struct SubscriberJoinData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;
			absl::optional<int64_t> feed;
			absl::optional<std::string> display;

			FIELDS_MAP("videoroom", videoroom,
				"room", room,
				"feed", feed,
				"display", display
			);
		};

		struct SubscriberJoinPluginData {
			absl::optional<std::string> plugin;
			absl::optional<SubscriberJoinData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct SubscriberJoinEvent {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<SubscriberJoinPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
		};

		/*
		 * As you can see, it's just a matter of specifying the list of streams to
		 * subscribe to : in particular, you have to provide an array of objects,
		 * where each objects represents a specific stream(or group of streams)
		 * you're interested in. For each object, the \c feed_id indicating the
		 * publisher owning the stream(s) is mandatory, while the related \c mid
		 * is optional : this gives you some flexibility when subscribing, as
		 * only providing a \c feed_id will indicate you're interested in ALL
		 * the stream from that publisher, while providing a \c mid as well will
		 * indicate you're interested in a stream in particular. Since you can
		 * provide an array of streams, just specifying the \c feed_id or explicitly
		 * listing all the \c feed_id + \c mid combinations is equivalent : of
		 * course, different objects in the array can indicate different publishers,
		 * allowing you to combine streams from different sources in the same subscription.
		 * Notice that if a publisher stream is marked as \c disabled and you try
		 * to subscribe to it, it will be skipped silently.
		 *
		 * Depending on whether the subscription will refer to a
		 * single publisher(legacy approach) or to streams coming from different
		 * publishers(multistream), the list of streams may differ.The ability
		 * to single out the streams to subscribe to is particularly useful in
		 * case you don't want to, or can't, subscribe to all available media :
		 * e.g., you know a publisher is sending both audio and video, but video
		 * is in a codec you don't support or you don't have bandwidth for both;
		 * or maybe there are 10 participants in the room, but you only want video
		 * from the 3 most active speakers; and so on.The content of the \c streams
		 * array will shape what the SDP offer the plugin will send will look like,
		 * so that eventually a subscription for the specified streams will take place.
		 * Notice that, while for backwards compatibility you can still use the
		 * old \c feed, \c audio, \c video, \c data, \c offer_audio, \c offer_video and
		 * \c offer_data named properties, they're now deprecated and so you're
		 * highly encouraged to use this new drill - down \c streams list instead.
		 *
		 * As anticipated, if successful this request will generate a new JSEP SDP
		 * offer, which will accompany an \c attached event:
		 * {
		 * 	"videoroom" : "attached",
		 * 	"room" : <room ID>,
		 * 	"streams" : [
		 * 		{
		 * 			"mindex" : <unique m - index of this stream>,
		 * 			"mid" : "<unique mid of this stream>",
		 * 			"type" : "<type of this stream's media (audio|video|data)>",
		 * 			"feed_id" : <unique ID of the publisher originating this stream>,
		 * 			"feed_mid" : "<unique mid of this publisher's stream>",
		 * 			"feed_display" : "<display name of this publisher, if any>",
		 * 			"send" : < true | false; whether we configured the stream to relay media > ,
		 * 			"ready" : < true | false; whether this stream is ready to start sending media(will be false at the beginning) >
		 * 		},
		 * 			// Other streams in the subscription, if any
		 * 		]
		 * }
		*/
		struct AttachedData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;

			struct Stream {
				absl::optional<bool> active;
				absl::optional<int64_t> mindex;
				absl::optional<std::string> mid;
				absl::optional<std::string> type;
				absl::optional<int64_t> feed_id;
				absl::optional<std::string> feed_mid;
				absl::optional<std::string> feed_display;
				absl::optional<bool> send;
				absl::optional<bool> ready;

				FIELDS_MAP("active", active, "mindex", mindex, "mid", mid, "type", type, "feed_id", feed_id, "feed_mid", feed_mid, "feed_display", feed_display, "send", send, "ready", ready);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("videoroom", videoroom, "room", room, "streams", streams);
		};

		struct AttachedPluginData {
			absl::optional<std::string> plugin;
			absl::optional<AttachedData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct AttachedEvent {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<AttachedPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
		};

		/*
		 * This means the exact same considerations we made on \c streams before
		 * apply here as well : whatever they represent, will indicate the willingness
		 * to subscribe to the related stream.Notice that if you were already
		 * subscribed to one of the new streams indicated here, you'll subscribe
		 * to it again in a different m - line, so it's up to you to ensure you
		 * avoid duplicates(unless that's what you wanted, e.g., for testing
		 * purposes).In case the update was successful, you'll get an \c updated
		 * event, containing the updated layout of all subscriptions(pre - existing
		 * and new ones), and a new JSEP offer to renegotiate the session :
		*/
		/*
		 * {
		 * "videoroom" : "updated",
		 * "room" : <room ID>,
		 * "streams": [
		 * 	{
		 * 		"mindex" : <unique m-index of this stream>,
		 * 		"mid" : "<unique mid of this stream>",
		 * 		"type" : "<type of this stream's media (audio|video|data)>",
		 * 		"feed_id" : <unique ID of the publisher originating this stream>,
		 * 		"feed_mid" : "<unique mid of this publisher's stream>",
		 * 		"feed_display" : "<display name of this publisher, if any>",
		 * 		"send" : <true|false; whether we configured the stream to relay media>,
		 * 		"ready" : <true|false; whether this stream is ready to start sending media (will be false at the beginning)>
		 * 	},
		 * 	// Other streams in the subscription, if any; old and new
		 * ]
		}*/
		struct UpdatedData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;

			struct Stream {
				absl::optional<int64_t> mindex;
				absl::optional<std::string> mid;
				absl::optional<std::string> type;
				absl::optional<int64_t> feed_id;
				absl::optional<int64_t> feed_mid;
				absl::optional<std::string> feed_display;
				absl::optional<bool> send;
				absl::optional<bool> ready;

				FIELDS_MAP("mindex", mindex, "mid", mid, "type", type, "feed_id", feed_id, "feed_mid", feed_mid, "feed_display", feed_display, "send", send, "ready", ready);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("videoroom", videoroom, "room", room, "streams", streams);
		};

		struct UpdatedPluginData {
			absl::optional<std::string> plugin;
			absl::optional<UpdatedData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct UpdatedEvent {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<UpdatedPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
		};

		struct UnpublishRequest {
			absl::optional<std::string> request = "unpublish";

			FIELDS_MAP("request", request);
		};

		struct SubscribeRequest {
			absl::optional<std::string> request = "subscribe";

			struct Stream {
				absl::optional<int64_t> feed;
				absl::optional<std::string> mid;

				FIELDS_MAP("feed", feed, "mid", mid);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("request", request, "streams", streams);
		};

		struct UnsubscribeRequest {
			absl::optional<std::string> request = "unsubscribe";

			struct Stream {
				absl::optional<int64_t> feed;
				absl::optional<std::string> mid;
				absl::optional<std::string> sub_mid;

				FIELDS_MAP("feed", feed, "mid", mid, "sub_mid", sub_mid);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("request", request, "streams", streams);
		};

		struct StartPeerConnectionRequest {
			absl::optional<std::string> request = "start";
			absl::optional<int64_t> room;

			FIELDS_MAP("request", request, "room", room);
		};

		struct PausePeerConnectionRequest {
			absl::optional<std::string> request = "pause";
			absl::optional<int64_t> room;

			FIELDS_MAP("request", request, "room", room);
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
			absl::optional<std::string> request = "leave";

			FIELDS_MAP("request", request);
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
			absl::optional<std::string> request = "create";
			absl::optional<int64_t> room;
			absl::optional<bool> permanent;
			absl::optional<std::string> description;
			absl::optional<std::string> secret;
			absl::optional<std::string> pin;
			absl::optional<bool> is_private = false;
			absl::optional<std::vector<std::string>> allowed;

			FIELDS_MAP("request", request,
				"room", room,
				"permanent", permanent,
				"description", description,
				"secret", secret,
				"pin", pin,
				"is_private", is_private,
				"allowed", allowed);
		};

		struct RoomCurdData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;
			absl::optional<bool> permanent;
			absl::optional<bool> exists;

			FIELDS_MAP("videoroom", videoroom, "room", room, "permanent", permanent, "exists", exists);
		};

		struct RoomCurdPluginData {
			absl::optional<std::string> plugin;
			RoomCurdData data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct RoomCurdResponse {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<RoomCurdPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
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
			absl::optional<std::string> request = "edit";
			absl::optional<int64_t> room;
			absl::optional<std::string> secret;
			absl::optional<std::string> new_description;
			absl::optional<std::string> new_secret;
			absl::optional<std::string> new_pin;
			absl::optional<bool> new_is_private;
			absl::optional<bool> new_require_pvtid;
			absl::optional<int64_t> new_bitrate;
			absl::optional<int64_t> new_fir_freq;
			absl::optional<bool> new_lock_record;
			absl::optional<int64_t> new_publishers;
			absl::optional<bool> permanent;

			FIELDS_MAP("request", request,
				"room", room,
				"secret", secret,
				"new_description", new_description,
				"new_secret", new_secret,
				"new_pin", new_pin,
				"new_is_private", new_is_private,
				"new_require_pvtid", new_require_pvtid,
				"new_bitrate", new_bitrate,
				"new_fir_freq", new_fir_freq,
				"new_lock_record", new_lock_record,
				"new_publishers", new_publishers,
				"permanent", permanent);
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
			absl::optional<std::string> request = "destroy";
			absl::optional<int64_t> room;
			absl::optional<std::string> secret;
			absl::optional<bool> permanent;

			FIELDS_MAP("request", request,
				"room", room,
				"secret", secret,
				"permanent", permanent);
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
			absl::optional<std::string> request = "exists";
			absl::optional<int64_t> room;

			FIELDS_MAP("request", request, "room", room);
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
			absl::optional<std::string> request = "allowed";
			absl::optional<std::string> secret;
			absl::optional<std::string> action;
			absl::optional<int64_t> room;
			absl::optional<std::vector<std::string>> allowed;

			FIELDS_MAP("request", request,
				"secret", secret,
				"action", action,
				"room", room,
				"allowed", allowed);
		};

		/*
		 * A successful request will result in a \c success response :
		 */
		 //\verbatim
		 //{
		 //	"videoroom" : "success",
		 //	"room" : <unique numeric ID>,
		 //	"allowed" : [
		 //		// Updated, complete, list of allowed tokens (only for enable|add|remove)
		 //	]
		 //}
		 //\endverbatim
		struct TokenInfo {
			absl::optional<std::string> token;

			FIELDS_MAP("token", token);
		};

		struct AllowedData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;
			absl::optional<std::vector<TokenInfo>> allowed;
			absl::optional<int64_t> error_code;
			absl::optional<std::string> error;

			FIELDS_MAP("videoroom", videoroom, "room", room, "allowed", allowed, "error_code", error_code, "error", error);
		};

		struct AllowedPluginData {
			absl::optional<std::string> plugin;
			absl::optional<AllowedData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct AllowedResponse {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<AllowedPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
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
			absl::optional<std::string> request = "kick";
			absl::optional<std::string> secret;
			absl::optional<int64_t> room;
			absl::optional<int64_t> id;

			FIELDS_MAP("request", request,
				"secret", secret,
				"room", room,
				"id", id);
		};

		struct KickData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> error_code;
			absl::optional<std::string> error;

			FIELDS_MAP("videoroom", videoroom, "error_code", error_code, "error", error);
		};

		struct KickPluginData {
			absl::optional<std::string> plugin;
			absl::optional<KickData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct KickResponse {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<KickPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
		};

		/*   
		 * As an administrator, you can also forcibly mute / unmute any of the media
		 * streams sent by participants(i.e., audio, video and data streams),
		 * using the \c moderate requests.Notice that if the participant is self
		 * muted on a stream, and you unmute that stream with \c moderate, they
		 * will NOT be unmuted : you'll simply remove any moderation block
		 * that may have been enforced on the participant for that medium
		 * themselves.The \c moderate request has to be formatted as follows :
		 *
		 * {
		 * 	"request" : "moderate",
		 * 	"secret" : "<room secret, mandatory if configured>",
		 * 	"room" : <unique numeric ID of the room>,
		 * 	"id" : <unique numeric ID of the participant to moderate>,
		 * 	"mid" : <mid of the m - line to refer to for this moderate request>,
		 * 	"mute" : <true | false, depending on whether the media addressed by the above mid should be muted by the moderator>
		 * }
		*/
		struct ModerateRequest {
			absl::optional<std::string> request = "moderate";
			absl::optional<std::string> secret;
			absl::optional<int64_t> room;
			absl::optional<int64_t> id;
			absl::optional<std::string> mid;
			absl::optional<bool> mute = false;

			FIELDS_MAP("request", request,
				"secret", secret,
				"room", room,
				"id", id,
				"mid", mid,
				"mute", mute);
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
			absl::optional<std::string> request = "list";

			FIELDS_MAP("request", request);
		};

		struct VideoRoomInfo {
			absl::optional<int64_t> room;
			absl::optional<std::string> description;
			absl::optional<int64_t> max_publishers;
			absl::optional<int64_t> bitrate;
			absl::optional<bool> bitrate_cap;
			absl::optional<int64_t> fir_freq;
			absl::optional<std::string> audiocodec;
			absl::optional<std::string> videocodec;
			absl::optional<bool> record;
			absl::optional<std::string> record_dir;
			absl::optional<bool> lock_record;
			absl::optional<int64_t> num_participants;

			FIELDS_MAP("room", room,
				"description", description,
				"max_publishers", max_publishers,
				"bitrate", bitrate,
				"bitrate_cap", bitrate_cap,
				"fir_freq", fir_freq,
				"audiocodec", audiocodec,
				"videocodec", videocodec,
				"record", record,
				"record_dir", record_dir,
				"lock_record", lock_record,
				"num_participants", num_participants);
		};

		struct FetchRoomsListData {
			absl::optional<std::string> videoroom;
			absl::optional<std::vector<VideoRoomInfo>> list;
			absl::optional<int64_t> error_code;
			absl::optional<std::string> error;

			FIELDS_MAP("videoroom", videoroom, "list", list, "error_code", error_code, "error", error);
		};

		struct FetchRoomsListPluginData {
			absl::optional<std::string> plugin;
			absl::optional<FetchRoomsListData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct FetchRoomsListResponse {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<FetchRoomsListPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
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
			absl::optional<std::string> request = "listparticipants";
			absl::optional<int64_t> room;

			FIELDS_MAP("request", request, "room", room);
		};

		struct ParticipantInfo {
			absl::optional<int64_t> id;
			absl::optional<std::string> display;
			absl::optional<bool> publisher;
			absl::optional<bool> talking;

			FIELDS_MAP("id", id, "display", display, "publisher", publisher, "talking", talking);
		};

		struct ParticipantData {
			absl::optional<std::string> videoroom;
			absl::optional<int64_t> room;
			absl::optional<std::vector<ParticipantInfo>> participants;
			absl::optional<int64_t> error_code;
			absl::optional<std::string> error;
		
			FIELDS_MAP("videoroom", videoroom, "room", room, "participants", participants, "error_code", error_code, "error", error);
		};

		struct ParticipantPluginData {
			absl::optional<std::string> plugin;
			absl::optional<ParticipantData> data;

			FIELDS_MAP("plugin", plugin, "data", data);
		};

		struct FetchParticipantsResponse {
			absl::optional<std::string> janus;
			absl::optional<std::string> transaction;
			absl::optional<int64_t> session_id;
			absl::optional<int64_t> sender;
			absl::optional<ParticipantPluginData> plugindata;

			FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata);
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
		 //	"audiocodec" : "<audio codec to prefer among the negotiated ones; optional>",
		 //	"videocodec" : "<video codec to prefer among the negotiated ones; optional>",
		 //	"bitrate" : < bitrate cap to return via REMB; optional, overrides the global room value if present > ,
		 //	"record" : < true | false, whether this publisher should be recorded or not; optional > ,
		 //	"filename" : "<if recording, the base path/file to use for the recording files; optional>",
		 //	"display" : "<new display name to use in the room; optional>",
		 //	"audio_level_average" : "<if provided, overrided the room audio_level_average for this user; optional>",
		 //	"audio_active_packets" : "<if provided, overrided the room audio_active_packets for this user; optional>",
		 //	"display" : "<new display name to use in the room; optional>",
		 //		"descriptions" : [	// Optional
		 //	{
		 //		"mid" : "<unique mid of a stream being published>",
		 //			"description" : "<text description of the stream (e.g., My front webcam)>"
		 //	},
		 //		// Other descriptions, if any
		 //		]}
		 //}
		 //\endverbatim
		struct PublishRequest {
			absl::optional<std::string> request = "publish";
			absl::optional<std::string> audiocodec;
			absl::optional<std::string> videocodec;
			absl::optional<int64_t> bitrate;
			absl::optional<bool> record;
			absl::optional<std::string> filename;
			absl::optional<std::string> display;
			absl::optional<int64_t> audio_level_average;
			absl::optional<int64_t> audio_active_packets;

			struct Description {
				absl::optional<std::string> mid;
				absl::optional<std::string> description;

				FIELDS_MAP("mid", mid, "description", description);
			};
			absl::optional<std::vector<Description>> descriptions;

			FIELDS_MAP("request", request,
				"audiocodec", audiocodec,
				"videocodec", videocodec,
				"bitrate", bitrate,
				"record", record,
				"filename", filename,
				"display", display,
				"audio_level_average", audio_level_average,
				"audio_active_packets", audio_active_packets,
				"descriptions", descriptions
			);
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
		//  "mid" : < mid of the m - line to refer to for this configure request; optional > ,
		//  "send" : < true | false, depending on whether the mindex media should be relayed or not; optional > ,
		//  "substream" : < substream to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"temporal" : < temporal layers to receive(0 - 2), in case simulcasting is enabled; optional > ,
		//	"fallback" : <How much time(in us, default 250000) without receiving packets will make us drop to the substream below>,
		//	"spatial_layer" : < spatial layer to receive(0 - 2), in case VP9 - SVC is enabled; optional > ,
		//	"temporal_layer" : < temporal layers to receive(0 - 2), in case VP9 - SVC is enabled; optional > ,
		//	"audio_level_average" : "<if provided, overrides the room audio_level_average for this user; optional>",
		//	"audio_active_packets" : "<if provided, overrides the room audio_active_packets for this user; optional>",
		//	"restart" : <trigger an ICE restart; optional>
		//}
		//\endverbatim

		struct SubscriberConfigureRequest {
			absl::optional<std::string> request = "configure";
			absl::optional<std::string> mid;
			absl::optional<bool> send = false;
			absl::optional<int64_t> substream;
			absl::optional<int64_t> temporal;
			absl::optional<int64_t> fallback;
			absl::optional<int64_t> spatial_layer;
			absl::optional<int64_t> temporal_layer;
			absl::optional<int64_t> audio_level_average;
			absl::optional<int64_t> audio_active_packets;
			absl::optional<bool> restart = false;

			FIELDS_MAP("request", request,
				"mid", mid,
				"send", send,
				"restart", restart,
				"substream", substream,
				"temporal", temporal,
				"fallback", fallback,
				"spatial_layer", spatial_layer,
				"temporal_layer", temporal_layer,
				"audio_level_average", audio_level_average,
				"audio_active_packets", audio_active_packets
			);
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
		//	"bitrate" : < bitrate cap to return via REMB; optional, overrides the global room value if present(unless bitrate_cap is set) > ,
		//	"keyframe" : <true | false, whether we should send this publisher a keyframe request>,
		//	"record" : < true | false, whether this publisher should be recorded or not; optional > ,
		//	"filename" : "<if recording, the base path/file to use for the recording files; optional>",
		//	"display" : "<new display name to use in the room; optional>",
		//	"audio_active_packets" : "<new audio_active_packets to overwrite in the room one; optional>",
		//	"audio_level_average" : "<new audio_level_average to overwrite the room one; optional>",
		//	"mid" : < mid of the m - line to refer to for this configure request; optional > ,
		//		"send" : < true | false, depending on whether the media addressed by the above mid should be relayed or not; optional > ,
		//		"descriptions" : [
		//			// Updated descriptions for the published streams; see "publish" for syntax; optional
		//		]
		//}
		//\endverbatim
		struct PublisherConfigureRequest {
			absl::optional<std::string> request = "configure";
			absl::optional<bool> audio = false;
			absl::optional<bool> video = false;
			absl::optional<bool> data = false;
			absl::optional<std::string> mid;
			absl::optional<bool> send = false;
			absl::optional<int64_t> bitrate;
			absl::optional<bool> keyframe;
			absl::optional<bool> record;
			absl::optional<std::string> filename;
			absl::optional<std::string> display;
			absl::optional<int64_t> audio_level_average;
			absl::optional<int64_t> audio_active_packets;

			struct Description {
				absl::optional<std::string> mid;
				absl::optional<std::string> description;

				FIELDS_MAP("mid", mid, "description", description);
			};
			absl::optional<std::vector<Description>> descriptions;

			FIELDS_MAP("request", request,
				"audio", audio,
				"video", video,
				"data", data,
				"mid", mid,
				"send", send,
				"bitrate", bitrate,
				"keyframe", keyframe,
				"record", record,
				"filename", filename,
				"display", display,
				"audio_level_average", audio_level_average,
				"audio_active_packets", audio_active_packets,
				"descriptions", descriptions
			);
		};

		//\verbatim
		//{
		//	"request" : "switch",
		//	"feed" : < unique ID of the new publisher to switch to; mandatory > ,
		//	"streams" : [
		//	{
		//		"feed" : <unique ID of the publisher the new source is from>,
		//			"mid" : "<unique mid of the source we want to switch to>",
		//			"sub_mid" : "<unique mid of the stream we want to pipe the new source to>"
		//	},
		//	{
		//		// Other updates, if any
		//	}
		//	]
		//}
		//\endverbatim
		struct SwitchPublisherRequest {
			absl::optional<std::string> request = "switch";
			
			struct Stream {
				absl::optional<int64_t> feed_id;
				absl::optional<std::string> mid;
				absl::optional<std::string> sub_mid;
				
				FIELDS_MAP("feed_id", feed_id, "mid", mid, "sub_mid", sub_mid);
			};
			absl::optional<std::vector<Stream>> streams;

			FIELDS_MAP("request", request, "streams", streams);
		};
	}
}
