#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <sstream> // for std::ostringstream
#include "../src/util/C.h"
#include "../src/util/Buffer.h"
#include "../src/util/ParsableByteArray.h"
#include "../src/server/file/VideoSample.h"

namespace Util {
	inline int findSequenceNumber(Buffer rtp) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 2);
		return rtpPacket.readUnsignedShort();
	}
}

#endif // UTIL_H