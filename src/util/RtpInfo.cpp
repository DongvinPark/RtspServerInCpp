#include "RtpInfo.h"

RtpInfo::RtpInfo() = default;

RtpInfo::RtpInfo(const RtpInfo& other) {
	copyFrom(other);
}

RtpInfo& RtpInfo::operator=(const RtpInfo& other) {
	// 'this' keyward means 'address of current class instance'.
	// That is, 'this' means 'pointer of current class instance'.
	// result of 'dereference operation to pointer' is '&'.
	if (this != &other) {
		copyFrom(other);
	}
	return *this;
}

RtpInfo::~RtpInfo() = default;

void RtpInfo::copyFrom(const RtpInfo& other) {
	kv = other.kv;
	urls = other.urls;
}

const std::shared_ptr<RtpInfo> RtpInfo::clone() {
	return std::make_shared<RtpInfo>(*this);
}

const std::string RtpInfo::toString() {
	std::ostringstream oss;
	oss << "{";

	// just like map.entrySet(); in java.
	for (const auto& [key, value] : kv) {
		oss << key << ": [";
		for (size_t i = 0; i < value.size(); ++i) {
			oss << value[i];
			if (i != value.size() - 1) {
				oss << ",";
			}
		}
		oss << "], ";
	}
	oss.seekp(-2, std::ios_base::end); // to remove last comma ans space
	oss << "}";

	oss << ", stream url: [";
	for (size_t i = 0; i < urls.size(); ++i) {
		oss << urls[i];
		if (i != urls.size() - 1) {
			oss << ", ";
		}
	}
	oss << "]";

	return oss.str();
}