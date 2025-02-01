#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <sstream> // for std::ostringstream
#include <stdexcept>
#include <random>
#include <iostream>
#include <iomanip>
#include <cstdint> // for int64_t
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <future>
#include <thread>
#include <optional>

#include "../constants/C.h"
#include "../include/Buffer.h"
#include "../include/ParsableByteArray.h"
#include "../include/VideoSample.h"
#include "../include/HybridSampleMeta.h"

using HybridMetaMapType
	= std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;

namespace Util {

	inline std::string getRandomKey(int bitLength) {
		const std::vector<int> allowedBits = {32, 64, 128, 192, 256};
		if (std::find(allowedBits.begin(), allowedBits.end(), bitLength) == allowedBits.end()) {
			throw std::logic_error("bitLength not allowed! : " + std::to_string(bitLength));
		}

		const std::string pickUpString =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"+_-.";

		int keyLength = bitLength / 8; // convert bit len to byte len

		std::random_device rd; // non-deterministic seed
		std::mt19937 eng(rd()); // Mersenne Twister engine with seed
		std::uniform_int_distribution<> dist(0, static_cast<int>(pickUpString.length() - 1));

		// Generate the random key
		std::ostringstream oss;
		for (int i = 0; i < keyLength; ++i) {
			oss << pickUpString[dist(eng)];
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

	inline int64_t usToMs(int64_t timeUs) {
		return timeUs / 1000;
	}

	inline int64_t secToUs(float timeSec) {
		return (int64_t)(timeSec * 1000000);
	}

	// mimics the util in java.
	// String[] arr = str.split('.');
	inline std::vector<std::string> splitToVecBySingleChar(
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

	inline std::vector<std::string> splitToVecByString(
	const std::string& str, const std::string& delimiter
	) {
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos) {
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}

		// add the last element
		tokens.push_back(str.substr(start));
		return tokens;
	}

	inline std::vector<std::string> splitToVecByStringForRtspMsg(
	const std::string& str, const std::string& delimiter
	) {
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos) {
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}

		// add the last element
		tokens.push_back(str.substr(start));
		std::vector<std::string> result;
		for (auto & token : tokens){
			if (token != C::EMPTY_STRING) {
				result.push_back(token);
			}
		}
		return result;
	}

	// delete the leading and trailing whitespace chars
	inline std::string trim(const std::string& str) {
		const size_t start = str.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) return "";
		const size_t end = str.find_last_not_of(" \t\r\n");
		return str.substr(start, end - start + 1);
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
		std::vector<std::string> words = splitToVecBySingleChar(fileName, '.');
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

	inline int64_t findTimestamp(const Buffer& rtp) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 4);
		return rtpPacket.readUnsignedInt();
	}

	inline int64_t findTimestampInVideoSample(VideoSample& sample) {
		return findTimestamp(sample.getFirstRtp());
	}

	inline std::vector<unsigned char> readAllBytesFromFilePath(
		const std::filesystem::path& inputFilePath
	) {
		try {
			// open the file in binary mode
			std::ifstream file(inputFilePath, std::ios::binary | std::ios::ate);
			if (!file) {
				throw std::ios_base::failure("Failed to open the file");
			}

			// get file size and resize the buffer
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<unsigned char> buffer(size);
			if (!file.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), size)) {
				throw std::ios_base::failure("Failed to read the file");
			}

			return buffer;
		} catch (const std::exception& e) {
			std::cerr << "Error reading file: " << e.what() << "\n";
			return {}; // return empty vector in case of an error
		}
	}

	inline int64_t getFileSize(const std::filesystem::path& filePath) {
		// open in binary mode and seek to the end
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file) {
			throw std::runtime_error("Failed to open file: " + filePath.filename().string());
		}
		return file.tellg(); // current position == file size
	}

	inline int32_t convertToInt32(const std::vector<unsigned char>& metaLenBuf) {
		if (metaLenBuf.size() != 4) {
			throw std::invalid_argument("metaLenBuf must contain exactly 4 bytes.");
		}

		// mimic the (b1 << 24) | (b2 << 16) + (b3 << 8) + b4 logic in java's RandomAccessFile's
		// .readInt() method.
		return (static_cast<int32_t>(metaLenBuf[0]) << 24) |
			   (static_cast<int32_t>(metaLenBuf[1]) << 16) |
			   (static_cast<int32_t>(metaLenBuf[2]) << 8) |
			   (static_cast<int32_t>(metaLenBuf[3]));
	}

	inline std::future<void> delayedExecutorAsyncByFuture(
		int delayInMillis, const std::function<void()> &task
	) {
		return std::async(std::launch::async, [delayInMillis, task]() {
		  std::this_thread::sleep_for(std::chrono::milliseconds(delayInMillis));
		  task();
		});
	}

	inline void delayedExecutorAsyncByThread(
		int delayInMillis, const std::function<void()>& task
	) {
		std::thread([delayInMillis, task]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(delayInMillis));
			task();
		}).detach();
	}

	inline std::optional<HybridSampleMeta> getHybridSampleMetaSafe(
		const HybridMetaMapType& hybridMetaMap,
		int camId,
		const std::string& vidAndFrameType,
		int sampleNo
	) {
		// iterator is small object to copy. so, used auto, not auto&
		auto camIt = hybridMetaMap.find(camId);
		if (camIt == hybridMetaMap.end()) return std::nullopt;

		auto vidFrameTypeIt = camIt->second.find(vidAndFrameType);
		if (vidFrameTypeIt == camIt->second.end()) return std::nullopt;

		auto sampleIt = vidFrameTypeIt->second.find(sampleNo);
		if (sampleIt == vidFrameTypeIt->second.end()) return std::nullopt;

		return sampleIt->second;
	}

}

#endif // UTIL_H