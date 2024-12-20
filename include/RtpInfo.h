#ifndef RTPINFO_H
#define RTPINFO_H

#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

class RtpInfo {
public:
	using KeyValueMap = std::unordered_map<std::string, std::vector<int64_t>>;

	explicit RtpInfo();
	~RtpInfo();

	// Do not use 'explicit' keyword in Copy Constructor!!
	RtpInfo(const RtpInfo& other);

	RtpInfo& operator=(const RtpInfo& other);

	const std::shared_ptr<RtpInfo> clone();
	const std::string toString();

	KeyValueMap kv;
	std::vector<std::string> urls;

private:
	void copyFrom(const RtpInfo& other);
};

#endif // RTPINFO_H