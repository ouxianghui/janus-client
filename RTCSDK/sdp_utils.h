/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-15
 **/

#pragma once

#include <string>
#include <vector>

namespace vi
{
	class SDPUtils 
	{
	public:
		static std::vector<std::string> split(const std::string& str, char delim);

		static std::string join(const std::vector<std::string>& lines, const std::string& separator = "\n");

		static void injectSimulcast(int64_t simulcast, std::vector<std::string>& lines);
	};
}