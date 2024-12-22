#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <sstream> // for std::ostringstream
#include <stdexcept>
#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint> // for int64_t
#include <filesystem>
#include <fstream>

#include "../constants/C.h"
#include "../include/Buffer.h"
#include "../include/ParsableByteArray.h"
#include "../include/VideoSample.h"

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

		/*
		// randome engine needs seed. So, std::time(...) was used.
		myEngine eng{ static_cast<unsigned long>(std::time(nullptr)) };
		run well on Window and WSL, but caused a type narrowing error on M1 chip MacOS.

		This error occurred because, myEngine requires unsigned int but result type of
		std::time(nullptr) is below like this according to Windows, WSL, M chip MacOS.

		> Windows and WSL (Linux on Windows):
		On 64-bit Windows, time_t is usually defined as a 64-bit integer (e.g., __int64).
		On Linux (e.g., WSL), time_t is typically a 64-bit long.
		On both platforms, unsigned long is also 64 bits, so there’s no narrowing conversion when casting time_t to unsigned long.
		> macOS (on M1 chip):
		On macOS, time_t is defined as long, which is 64 bits.
		However, unsigned long is 64 bits as well, but the default type for result_type in the std::mt19937 engine
		(or your custom myEngine) is often unsigned int, which is 32 bits.
		This creates a narrowing conversion error because the 64-bit value from std::time
		is being assigned to a 32-bit type.

		> '::result_type' was used to handle this platform dependency problem.
		::result_type is a type alias defined in a class/struct that indicates the type of value
		the class produces. In random engines like std::mt19937, it specifies the type of generated
		numbers (e.g., unsigned int). Using myEngine::result_type ensures type safety and portability
		across platforms, avoiding narrowing conversion errors.

		> 'long' VS 'long long' in C++
		long: platform-dependent (32-bit on Windows, 64-bit on Linux).
		long long: guaranteed 64-bit or more, use for larger integers. platform-dependent also.
		*/

		std::random_device rd; // non-deterministic seed
		std::mt19937 eng(rd()); // Mersenne Twister engine with seed
		std::uniform_int_distribution<> dist(0, static_cast<int>(pickUpString.length() - 1));

		// Generate the random key
		std::ostringstream oss;
		oss << "client_";
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

	inline uint32_t findTimestamp(Buffer rtp) {
		ParsableByteArray rtpPacket(rtp);
		rtpPacket.skipBytes(C::TCP_RTP_HEAD_LEN + 4);
		return rtpPacket.readUnsignedInt();
	}

	inline uint32_t findTimestampInVideoSample(VideoSample& sample) {
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
}

#endif // UTIL_H