#ifndef RTPINFO_H
#define RTPINFO_H

#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

class RtpInfo {
public:
	using KeyValueMap = std::unordered_map<std::string, std::vector<long>>;

	explicit RtpInfo();
	explicit RtpInfo(const RtpInfo& other);
	RtpInfo& operator=(const RtpInfo& other);
	~RtpInfo();

	const std::shared_ptr<RtpInfo> clone();
	const std::string toString();

	KeyValueMap kv;
	std::vector<std::string> urls;

private:
	void copyFrom(const RtpInfo& other);
};

#endif // RTPINFO_H