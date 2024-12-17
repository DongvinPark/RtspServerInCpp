#ifndef RTPMETAINFO_H
#define RTPMETAINFO_H

class RtpMetaInfo {
public:
	int len; // rtp length
	long offset; // offset from file 0 position

	explicit RtpMetaInfo(int len, long offset);

	~RtpMetaInfo() = default;
};

#endif // RTPMETAINFO_H