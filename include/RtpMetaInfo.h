#ifndef RTPMETAINFO_H
#define RTPMETAINFO_H

#include <cstdint> // for int64_t

class RtpMetaInfo {
public:
	const int len; // rtp length
	const int64_t offset; // offset from file 0 position

	explicit RtpMetaInfo(int len, int64_t offset);

	~RtpMetaInfo() = default;
};

#endif // RTPMETAINFO_H