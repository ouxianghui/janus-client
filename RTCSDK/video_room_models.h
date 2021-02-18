/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-11
 **/

#pragma once

#include "message_models.h"

namespace vi {
	namespace vr {
		struct Publisher {
			int64_t id;
			std::string display;
			bool talking;

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
				std::string type;
				int64_t mindex;
				std::string mid;
				bool disabled = false;
				std::string codec;
				std::string description;
				bool simulcast = false;
				bool svc = false;
				bool talking = false;

				XTOSTRUCT(O(type, mindex, mid, disabled, codec, description, simulcast, svc, talking));
			};
			std::vector<Stream> streams;
			XTOSTRUCT(O(id, display, talking, streams));
		};

		// Video Room event
		struct JoiningData {
			int64_t id;
			std::string display;

			XTOSTRUCT(O(id, display));
		};

		struct EventData {
			std::string videoroom;
			int64_t error_code = 0;
			std::string error;
			JoiningData joining;
			std::string configured;
			std::vector<Publisher> publishers;
			int64_t unpublished = -1;
			int64_t leaving = -1;
			std::string started;
			std::string paused;
			std::string switched;
			int64_t id = -1;
			std::string left;
			std::string audio_codec;
			std::string video_codec;

			XTOSTRUCT(O(videoroom,
				error_code,
				error,
				joining,
				configured,
				publishers,
				unpublished,
				leaving,
				started,
				paused,
				switched,
				id,
				left,
				audio_codec,
				video_codec
			));
		};

		struct EventPluginData {
			std::string plugin;
			EventData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct VideoRoomEvent : public JanusResponse {
			EventPluginData plugindata;
			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string request = "join";
			std::string ptype = "publisher";	
			int64_t room;
			std::string display;
			//std::string token;
			XTOSTRUCT(O(request, ptype, room, display/*, token*/));
		};

		struct Attendee {
			int64_t id;
			std::string display;

			XTOSTRUCT(O(id, display));
		};

		struct PublisherJoinData {
			std::string videoroom;
			int64_t room;
			std::string description;
			int64_t id;
			int64_t private_id;
			std::vector<Publisher> publishers;
			std::vector<Attendee> attendees;

			XTOSTRUCT(O(videoroom, room, description, id, private_id, publishers, attendees));
		};

		struct PublisherJoinPluginData {
			std::string plugin;
			PublisherJoinData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct PublisherJoinEvent : public JanusResponse {
			PublisherJoinPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string request = "join";
			int64_t room;
			std::string ptype = "subscriber";
			int64_t feed;
			int64_t private_id;

			struct Stream {
				int64_t feed;
				std::string mid;
				XTOSTRUCT(O(feed, mid));
			};
			std::vector<Stream> streams;

			XTOSTRUCT(O(request,
				room,
				ptype,
				feed,
				private_id,
				streams
			));
		};

		struct SubscriberJoinData {
			std::string videoroom;
			int64_t room;
			int64_t feed;
			std::string display;

			XTOSTRUCT(O(videoroom, room, feed, display));
		};

		struct SubscriberJoinPluginData {
			std::string plugin;
			SubscriberJoinData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct SubscriberJoinEvent : public JanusResponse {
			SubscriberJoinPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string videoroom;
			int64_t room;

			struct Stream {
				bool active = false;
				int64_t mindex;
				std::string mid;
				std::string type;
				int64_t feed_id;
				std::string feed_mid;
				std::string feed_display;
				bool send = false;
				bool ready = false;

				XTOSTRUCT(O(active, mindex, mid, type, feed_id, feed_mid, feed_display, send, ready));
			};
			std::vector<Stream> streams;

			XTOSTRUCT(O(videoroom, room, streams));
		};

		struct AttachedPluginData {
			std::string plugin;
			AttachedData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct AttachedEvent : public JanusResponse {
			AttachedPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string videoroom;
			int64_t room;

			struct Stream {
				int64_t mindex;
				std::string mid;
				std::string type;
				int64_t feed_id;
				int64_t feed_mid;
				std::string feed_display;
				bool send = false;
				bool ready = false;

				XTOSTRUCT(O(mindex, mid, type, feed_id, feed_mid, feed_display, send, ready));
			};
			std::vector<Stream> streams;

			XTOSTRUCT(O(videoroom, room, streams));
		};

		struct UpdatedPluginData {
			std::string plugin;
			UpdatedData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct UpdatedEvent : public JanusResponse {
			UpdatedPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
		};

		struct UnpublishRequest {
			std::string request = "unpublish";
			XTOSTRUCT(O(request));
		};

		struct SubscribeRequest {
			std::string request = "subscribe";

			struct Stream {
				int64_t feed;
				std::string mid;
				XTOSTRUCT(O(feed, mid));
			};
			std::vector<Stream> streams;

			XTOSTRUCT(O(request, streams));
		};

		struct UnsubscribeRequest {
			std::string request = "unsubscribe";

			struct Stream {
				int64_t feed;
				std::string mid;
				std::string sub_mid;
				XTOSTRUCT(O(feed, mid, sub_mid));
			};
			std::vector<Stream> streams;

			XTOSTRUCT(O(request, streams));
		};

		struct StartPeerConnectionRequest {
			std::string request = "start";
			int64_t room;
			XTOSTRUCT(O(request, room));
		};

		struct PausePeerConnectionRequest {
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

		struct RoomCurdData {
			std::string videoroom;
			int64_t room;
			bool permanent;
			bool exists;

			XTOSTRUCT(O(videoroom, room, permanent, exists));
		};

		struct RoomCurdPluginData {
			std::string plugin;
			RoomCurdData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct RoomCurdResponse : public JanusResponse {
			RoomCurdPluginData plugindata;

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
			std::string token;
			XTOSTRUCT(O(token));
		};

		struct AllowedData {
			std::string videoroom;
			int64_t room;
			std::vector<TokenInfo> allowed;
			int64_t error_code = 0;
			std::string error;
			XTOSTRUCT(O(videoroom, room, allowed, error_code, error));
		};

		struct AllowedPluginData {
			std::string plugin;
			AllowedData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct AllowedResponse : public JanusResponse {
			AllowedPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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

		struct KickData {
			std::string videoroom;
			int64_t error_code = 0;
			std::string error;
			XTOSTRUCT(O(videoroom, error_code, error));
		};

		struct KickPluginData {
			std::string plugin;
			KickData data;

			XTOSTRUCT(O(plugin, data));
		};

		struct KickResponse : public JanusResponse {
			KickPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string request = "moderate";
			std::string secret;
			int64_t room;
			int64_t id;
			std::string mid;
			bool mute = false;

			XTOSTRUCT(O(request,
				secret,
				room,
				id,
				mid, 
				mute));
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

		struct VideoRoomInfo {
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

		struct FetchRoomsListData {
			std::string videoroom;
			std::vector<VideoRoomInfo> list;
			int64_t error_code = 0;
			std::string error;
			XTOSTRUCT(O(videoroom, list, error_code, error));
		};

		struct FetchRoomsListPluginData {
			std::string plugin;
			FetchRoomsListData data;
			XTOSTRUCT(O(plugin, data));
		};

		struct FetchRoomsListResponse : public JanusResponse {
			FetchRoomsListPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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

		struct ParticipantInfo {
			int64_t id;
			std::string display;
			bool publisher;
			bool talking;

			XTOSTRUCT(O(id, display, publisher, talking));
		};

		struct ParticipantData {
			std::string videoroom;
			int64_t room;
			std::vector<ParticipantInfo> participants;
			int64_t error_code = 0;
			std::string error;
			XTOSTRUCT(O(videoroom, room, participants, error_code, error));
		};

		struct ParticipantPluginData {
			std::string plugin;
			ParticipantData data;
			XTOSTRUCT(O(plugin, data));
		};

		struct FetchParticipantsResponse : public JanusResponse {
			ParticipantPluginData plugindata;

			XTOSTRUCT(I(JanusResponse), O(plugindata));
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
			std::string request = "publish";
			std::string audiocodec;
			std::string videocodec;
			int64_t bitrate;
			bool record;
			std::string filename;
			std::string display;
			int64_t audio_level_average;
			int64_t audio_active_packets;

			struct Description {
				std::string mid;
				std::string description;
				XTOSTRUCT(O(mid, description));
			};
			std::vector<Description> descriptions;
			XTOSTRUCT(O(request,
				audiocodec,
				videocodec,
				bitrate,
				record,
				filename,
				display,
				audio_level_average,
				audio_active_packets,
				descriptions
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
			std::string request = "configure";
			std::string mid;
			bool send = false;
			//int64_t substream;
			//int64_t temporal;
			//int64_t fallback;
			//int64_t spatial_layer;
			//int64_t temporal_layer;
			//int64_t audio_level_average;
			//int64_t audio_active_packets;
			bool restart = false;
			XTOSTRUCT(O(request,
				mid,
				send,
				restart
				//substream,
				//temporal,
				//fallback,
				//spatial_layer,
				//temporal_layer,
				//audio_level_average,
				//audio_active_packets
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
			std::string request = "configure";
			bool audio = false;
			bool video = false;
			bool data = false;
			std::string mid;
			bool send = false;
			int64_t bitrate;
			bool keyframe;
			bool record;
			std::string filename;
			std::string display;
			int64_t audio_level_average;
			int64_t audio_active_packets;

			struct Description {
				std::string mid;
				std::string description;
				XTOSTRUCT(O(mid, description));
			};
			std::vector<Description> descriptions;

			XTOSTRUCT(O(request,
				audio,
				video,
				data,
				mid,
				send,
				bitrate,
				keyframe,
				record,
				filename,
				display,
				audio_level_average,
				audio_active_packets, 
				descriptions
			));
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
			std::string request = "switch";
			
			struct Stream {
				int64_t feed_id;
				std::string mid;
				std::string sub_mid;
				XTOSTRUCT(O(feed_id, mid, sub_mid));
			};
			std::vector<Stream> streams;
			XTOSTRUCT(O(request, streams));
		};
	}
}
