/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "message_models.h"
#include <memory>

namespace vi {
	class IMessageTransportListener
	{
	public:
		virtual ~IMessageTransportListener() {}

		virtual void onOpened() = 0;

		virtual void onClosed() = 0;

		virtual void onFailed(int errorCode, const std::string& reason) = 0;

		virtual void onMessage(const std::string& json) = 0;

	};
}