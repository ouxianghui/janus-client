/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-15
 **/

#include "sdp_utils.h"
#include <regex>
#include <random>
#include <sstream>
#include <iomanip>

namespace vi
{
	bool parseMline(const std::string &line, std::string &type)
	{
		static std::regex re("^m=(\\S+) ", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_search(line, match, re)) {
			type = match[1].str();
			return true;
		}
		else {
			return false;
		}
	}

	bool parseFid(const std::string &line, std::string &ssrc, std::string &ssrcFid)
	{
		static std::regex re("a=ssrc-group:FID (\\d+) (\\d+)\\r", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_match(line, match, re)) {
			ssrc = match[1].str();
			ssrcFid = match[2].str();
			return true;
		}
		else {
			return false;
		}
	}

	bool parseCname(const std::string &line, std::string &ssrc, std::string &cname)
	{
		static std::regex re("a=ssrc:(\\d+) cname:(\\S+)\\r", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_match(line, match, re)) {
			ssrc = match[1].str();
			cname = match[2].str();
			return true;
		}
		else {
			return false;
		}
	}

	bool parseMslabel(const std::string &line, std::string &ssrc, std::string &mslabel)
	{
		static std::regex re("a=ssrc:(\\d+) mslabel:(\\S+)\\r", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_match(line, match, re)) {
			ssrc = match[1].str();
			mslabel = match[2].str();
			return true;
		}
		else {
			return false;
		}
	}

	bool parseLabel(const std::string &line, std::string &ssrc, std::string &label)
	{
		static std::regex re("a=ssrc:(\\d+) label:(\\S+)\\r", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_match(line, match, re)) {
			ssrc = match[1].str();
			label = match[2].str();
			return true;
		}
		else {
			return false;
		}
	}

	std::string genSsrc()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<unsigned long> dis(1, 0xFFFFFFFF);

		const auto value = dis(gen);
		std::stringstream result;
		result << std::setw(9) << std::setfill('0');
		result << value;
		return result.str();
	}

	void injectSsrc(std::vector<std::string> &lines, const std::string &ssrc, const std::string &cname, const std::string &mslabel, const std::string &label)
	{
		const std::string assrc = "a=ssrc:" + ssrc;
		lines.emplace_back(assrc + " cname:" + cname + "\r");
		lines.emplace_back(assrc + " msid:" + mslabel + " " + label + "\r");
		lines.emplace_back(assrc + " mslabel:" + mslabel + "\r");
		lines.emplace_back(assrc + " label:" + label + "\r");
	}

	std::string injectSsrcAndFid(std::vector<std::string> &lines, const std::string &cname, const std::string &mslabel, const std::string &label, bool fid)
	{
		const auto ssrc = genSsrc();
		const auto ssrcFid = genSsrc();
		if (fid) {
			lines.push_back("a=ssrc-group:FID " + ssrc + " " + ssrcFid + "\r");
		}
		injectSsrc(lines, ssrc, cname, mslabel, label);
		if (fid) {
			injectSsrc(lines, ssrcFid, cname, mslabel, label);
		}
		return ssrc;
	}

	std::vector<std::string> SDPUtils::split(const std::string& str, char delim) {
		std::stringstream sstr(str);
		std::string item;
		std::vector<std::string> elems;
		while (std::getline(sstr, item, delim)) {
			elems.push_back(std::move(item));
		}
		return elems;
	}

	std::string SDPUtils::join(const std::vector<std::string>& lines, const std::string& separator)
	{
		std::string result;
		size_t size = 0;
		for (const auto &l : lines) size += l.size();
		result.reserve(lines.size() * separator.size() + size + 1);

		for (size_t i = 0; i < lines.size(); ++i) {
			if (i) {
				result += separator;
			}
			result += lines[i];
		}
		return result;
	}

	void SDPUtils::injectSimulcast(int64_t simulcast, std::vector<std::string> &lines)
	{
		if (simulcast < 1 || simulcast > 2) {
			return;
		}

		std::string ssrc, ssrcFid, cname, mslabel, label;
		size_t index2inject = 0;
		bool video = false;
		bool hasSimulcast = false;

		for (size_t i = 0; i < lines.size(); ++i) {
			const auto &line = lines[i];
			std::string mtype;
			if (parseMline(line, mtype)) {
				if (!video && mtype == "video") {
					video = true;
					continue;
				}
				if (video) {
					video = false;
					index2inject = i - 1;
					continue;
				}
			}
			if (!video) {
				continue;
			}

			if (ssrc.empty()) {
				if (parseFid(line, ssrc, ssrcFid)) {
					continue;
				}
				parseCname(line, ssrc, cname);
			}

			if (ssrc.length()) {
				std::string tmpSsrc, value;
				if (cname.empty() && parseCname(line, tmpSsrc, value) && tmpSsrc == ssrc) {
					cname = value;
					continue;
				}

				if (mslabel.empty() && parseMslabel(line, tmpSsrc, value) && tmpSsrc == ssrc) {
					mslabel = value;
					continue;
				}

				if (label.empty() && parseLabel(line, tmpSsrc, value) && tmpSsrc == ssrc) {
					label = value;
					continue;
				}
			}

			if (line.find("a=ssrc-group:SIM ") == 0) {
				hasSimulcast = true;
			}
		}

		if (!hasSimulcast && ssrc.length() && cname.length() && mslabel.length() && label.length()) {
			std::stringstream group;
			std::vector<std::string> simulcastLines;
			group << "a=ssrc-group:SIM " + ssrc;
			for (int64_t i = 0; i < simulcast; ++i) {
				const auto assrc = injectSsrcAndFid(simulcastLines, cname, mslabel, label, ssrcFid.length());
				group << " " << assrc;
			}
			group << "\r";
			simulcastLines.emplace_back(group.str());
			const auto insertIt = index2inject ? lines.begin() + index2inject : lines.end();
			lines.insert(insertIt, simulcastLines.begin(), simulcastLines.end());
		}
	}
}