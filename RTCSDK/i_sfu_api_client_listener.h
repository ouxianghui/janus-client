/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include "message_transport.h"

namespace vi {
	class ISfuApiClientListener
	{
	public:
		virtual ~ISfuApiClientListener() {}

		virtual void onOpened() = 0;

		virtual void onClosed() = 0;

		virtual void onFailed(int errorCode, const std::string& reason) = 0;

		virtual void onMessage(std::shared_ptr<JanusResponse> model) = 0;
	};
}