/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "helper_utils.h"

namespace vi {

	// Helper methods to parse a media object
	bool HelperUtils::isAudioSendEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return true;	// Default
		if (media.value().audio == false)
			return false;	// Generic audio has precedence
		if (!media.value().audioSend)
			return true;	// Default
		return (media.value().audioSend == true);
	}

	bool HelperUtils::isAudioSendRequired(const absl::optional<MediaConfig>& media) {
		if (!media)
			return false;	// Default
		if (media.value().audio == false || media.value().audioSend.value_or(false) == false)
			return false;	// If we're not asking to capture audio, it's not required
		if (!media.value().failIfNoAudio)
			return false;	// Default
		return (media.value().failIfNoAudio.value() == true);
	}

	bool HelperUtils::isAudioRecvEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return true;	// Default
		if (media.value().audio == false)
			return false;	// Generic audio has precedence
		if (!media.value().audioRecv)
			return true;	// Default
		return (media.value().audioRecv.value() == true);
	}

	bool HelperUtils::isVideoSendEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return true;	// Default
		if (media.value().video == false)
			return false;	// Generic video has precedence
		if (!media.value().videoSend)
			return true;	// Default
		return (media.value().videoSend.value() == true);
	}

	bool HelperUtils::isVideoSendRequired(const absl::optional<MediaConfig>& media) {
		if (!media)
			return false;	// Default
		if (media.value().video == false || media.value().videoSend.value_or(false) == false)
			return false;	// If we're not asking to capture video, it's not required
		if (!media.value().failIfNoVideo)
			return false;	// Default
		return (media.value().failIfNoVideo.value() == true);
	}

	bool HelperUtils::isVideoRecvEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return true;	// Default
		if (media.value().video == false)
			return false;	// Generic video has precedence
		if (!media.value().videoRecv)
			return true;	// Default
		return (media.value().videoRecv.value() == true);
	}

	bool HelperUtils::isScreenSendEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return false;
		return true;
	}

	bool HelperUtils::isDataEnabled(const absl::optional<MediaConfig>& media) {
		if (!media)
			return false;	// Default
		return (media.value().data == true);
	}

	bool HelperUtils::isTrickleEnabled(const absl::optional<bool>& trickle) {
		if (!trickle)
			return true;	// Default is true
		return (trickle == true);
	}
}