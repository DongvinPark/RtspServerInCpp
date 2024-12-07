#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <sstream> // for std::ostringstream
#include <stdexcept>
#include <random>
#include <sstream>
#include <iomanip>
#include "../src/util/C.h"
#include "../src/util/Buffer.h"
#include "../src/util/ParsableByteArray.h"
#include "../src/server/file/VideoSample.h"

namespace Util {

	inline std::string getRandomKey(int bitLength) {
		std::vector<int> allowedBits = {32, 64, 128, 192, 256};
		bool found = false;
		for (int bits : allowedBits) {
			if (bitLength == bits) {
				found = true;
				break;
			}
		}
		if (found == false) {
			throw std::logic_error("bitLength not allowed! : " + std::to_string(bitLength));
		}
		std::string pickUpString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvxyz"
			"0123456789"
			"+_-.";

		int keyLength = bitLength / 8;

		// make simple random int generator
		// range is [ 0 : pickUpString.length()-1 ]. closed range!
		using myEngine = std::default_random_engine;
		using myDistribution = std::uniform_int_distribution<>;

		// randome engine needs seed.
		myEngine eng{ static_cast<unsigned long>(std::time(nullptr)) };
		myDistribution dist{ 0, static_cast<int>(pickUpString.length() - 1)};
		auto getRandomCharIdx = [&]() {return dist(eng); };

		std::ostringstream oss;
		oss << "client_";
		for (int i = 0; i < keyLength; ++i) {
			int idx = getRandomCharIdx();
			oss << pickUpString[idx];
		}
		return oss.str();
	}

	inline int indexOf(std::vector<unsigned char>& array, unsigned char target, int start, int end) {
		for (int i = start; i < end; ++i) {
			if (array[i] == target) return i;
		}
		return C::INVALID;
	}

	inline int indexOf(Buffer& buffer, unsigned char target) {
		return indexOf(buffer.buf, target, buffer.offset, buffer.limit);
	}

	inline bool isVideo(int streamId) {
		return streamId == C::VIDEO_ID;
	}

	inline long usToMs(long timeUs) {
		return timeUs / 1000;
	}

	inline long secToUs(float timeSec) {
		return (long)(timeSec * 1000000);
	}

	// mimics the util in java.
	// String[] arr = str.split('.');
	inline std::vector<std::string> splitToVec(
		const std::string& str, char delimiter
	) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;

		// used std::getline() to split the inputstring
		// with input delimeter.
		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	inline std::string getNameOnly(const std::string& fileName) {
		// find the last occurrence of the dot
		size_t pos = fileName.find_last_of('.');
		// if no dot is found, return the original file name
		if (pos == std::string::npos) {
			return fileName;
		}
		return fileName.substr(0, pos);
	}

	inline std::string getExtension(std::string fileName) {
		std::vector<std::string> words = splitToVec(fileName, '.');
		return (words.size() == 2) ? words[1] : "";
	}

	inline bool isTypeOf(std::string fileName, std::string type) {
		std::string ext = getExtension(fileName);
		return ext == type;
	}

	inline bool isOdd(int a) {
		return (a & 0x01) == 1;
	}

	inline std::string toHex(unsigned char a) {
		std::ostringstream oss;
		// mimics the java util - Integer.toHexString();
		oss << "0x"
			<< std::hex << std::uppercase
			// to make string like 0x2A
			<< std::setfill('0') << std::setw(2)
			<< static_cast<int>(a);
		return oss.str();
	}

	inline bool isRtpHead(unsigned char b) {
		return (b & 0xFF) == '$';
	}

	inline int findSequenceNumber(Buffer rtp) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 2);
		return rtpPacket.readUnsignedShort();
	}

	inline void writeSequenceNumber(Buffer& rtp, int seqNum) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 2);
		rtpPacket.writeUnsignedShort(seqNum);
	}

	inline int findSequenceNumberInVideoSample(VideoSample& sample) {
		Buffer firstRtp = sample.getFirstRtp();
		return findSequenceNumber(firstRtp);
	}

	inline uint32_t findTimestamp(Buffer rtp) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 4);
		return rtpPacket.readUnsignedInt();
	}

	inline uint32_t findTimestampInVideoSample(VideoSample& sample) {
		return findTimestamp(sample.getFirstRtp());
	}
}

#endif // UTIL_H