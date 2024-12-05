#ifndef C_H
#define C_H

#include <string>
#include <vector>
#include <set>

namespace C {
    // General constants
    constexpr char VERSION[] = "1.0.4";

    constexpr int TCP_RTP_HEAD_LEN = 4; // $+(ch 1) + (len 2)
    constexpr int RTP_HEADER_LEN = 12; // Refer to https://datatracker.ietf.org/doc/html/rfc7798
    constexpr int UNSET = -1;

    // Clock rates
    constexpr int H265_CLOCK_RATE = 90000; // by spec, 90kHz must be used.
    constexpr int AAC_CLOCK_RATE = 48000; // Audio has 48kHz when included in video.

    // Network ports
    constexpr int RTP_RX_PORT = 9000;
    constexpr int RTCP_RX_PORT = 9001;
    constexpr int RTSP_RTP_TCP_PORT = 8554;

    // Strings and MTU size
    constexpr char CRLF[] = "\r\n";
    constexpr char CRLF2[] = "\r\n\r\n";
    constexpr int MTU_SIZE = 1472; // UDP max payload size
    constexpr char DEFAULT_MEDIA_TYPE[] = "mp4";
    constexpr int DEFAULT_DIVIDE_NUM = 3;
    constexpr char DEFAULT_IP[] = "127.0.0.1";

    // Keys
    constexpr char SSRC_KEY[] = "ssrc=";
    constexpr char SEQ_KEY[] = "seq0=";
    constexpr char TIMESTAMP_KEY[] = "timestamp0=";
    constexpr char GOP_KEY[] = "gop=";
    constexpr char FRAME_COUNT_KEY[] = "framecount=";
    constexpr char MEDIA_INFO_KEY[] = "v=";
    constexpr char PLAY_TIME_KEY[] = "playtime=";

    // Adaptive bitrate
    constexpr char SWITCHING_KEY[] = "SwitchingInfo";
    constexpr char CAM_CHANG_KEY[] = "CameraInfo";
    constexpr char BITRATE_CHANG_KEY[] = "BitrateInfo";
    constexpr char P_FRAME_KEY[] = "PFrameInfo";
    constexpr char REF_CAM[] = "cam0";
    const std::vector<std::string> ADAPTIVE_BITRATE_REF_CAM_LIST = {"cam0-10m"};

    // Stream ID
    constexpr int VIDEO_ID = 0;
    constexpr int AUDIO_ID = 1;

    // Keyframe finding
    constexpr int NEXT_KEY = 0;
    constexpr int PREVIOUS_KEY = 1;
    constexpr int NEAREST_KEY = 2;
    constexpr int KEYFRAME_FLAG = 1;

    // SNTP
    constexpr char SNTP_SERVER_HOST[] = "time.android.com";
    constexpr int NTP_PORT = 123;
    constexpr int NTP_MODE_CLIENT = 3;
    constexpr int NTP_VERSION = 3;
    constexpr int HOST_RSP_TIMEOUT_MS = 5000; // ms
    constexpr int NTP_READ_PERIOD = 3000; // ms
    constexpr int REFERENCE_TIME_OFFSET = 16;
    constexpr int ORIGINATE_TIME_OFFSET = 24;
    constexpr int RECEIVE_TIME_OFFSET = 32;
    constexpr int TRANSMIT_TIME_OFFSET = 40;
    constexpr int NTP_PACKET_SIZE = 48;
    constexpr long OFFSET_1900_TO_1970 = ((365L * 70L) + 17L) * 24L * 60L * 60L;

    // Monitoring HTTP Server
    constexpr int MONITORING_HTTP_SERVER_PORT = 80;
    constexpr char QUOTATION_MARK = '"';
    const std::set<std::string> VIDEO_ID_SET = {
        "10", "11", "12", "13", "20", "21", "22", "23", "30", "31", "32", "33"
    };
    constexpr char DEFAULT_EMPTY_STRING[] = "";

    // Pause Seek
    constexpr int RE_PAUSE_DELAY_TIME_SEC = 1;
    constexpr float START_NPT[] = {0.0f, -1.0f};

    // Pause Switching
    constexpr int TX_SAMPLE_CNT = 10;
    constexpr int RE_PAUSE_DELAY_TIME_MS = 100;

    // Filtering content
    constexpr int FILE_NUM_LIMIT = 10;

    // P Frame control
    constexpr int INVALID_SAMPLE_NO = -1;
    constexpr char SEND_P_FRAMES[] = "send-p";
    constexpr char STOP_P_FRAMES[] = "stop-p";
    constexpr int P_FRAME_GOP_CONTROL_FACTOR_FOR_SEEK = 2;
    constexpr int DEFAULT_NO_FAST_TX_FACTOR = 0;
    constexpr int MIN_NUM_OF_FAST_TX_SAMPLES = 2;
    constexpr int FAST_TX_VIDEO_SAMPLE_CNT = 10;
    constexpr int FAST_TX_AUDIO_SAMPLE_CNT = 15;

    // Hybrid
    constexpr int FIRST_KEY_FRAME_SAMPLE_NO = 0;
    const std::set<std::string> HYBRID_MODE_SET = {"none", "low", "medium", "high", "ultra-high"};
    const std::vector<std::string> CAM_ID_LIST = {"cam0", "cam1", "cam2"};
    constexpr char CAM_DIR_PREFIX[] = "cam";
    constexpr char HYBRID_META_DIR[] = "hybrid-meta";
    constexpr char HYBRID_META_JSON_FILE_NAME[] = "_data.json";
    constexpr int MAX_CAM_DIR_NUMBER = 4;
    constexpr char KEY_FRAME_TYPE[] = "I";
    constexpr char P_FRAME_TYPE[] = "P";
    constexpr char COMMA_SEPARATOR = ',';
    constexpr char INTERLEAVED_BINARY_DATA_MARKER = '$';
    constexpr long INVALID_OFFSET = -1L;
    constexpr uint8_t REF_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x00;
    constexpr uint8_t FIRST_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x04;
    constexpr uint8_t SECOND_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x06;
    constexpr uint8_t THIRD_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x08;
    constexpr uint8_t INVALID_BYTE = 0xFF;
    constexpr int RTP_CHANNEL_INFO_META_LENGTH = 4;
    constexpr int VIEW_NUM_FACTOR = 1;
    constexpr char HYBRID_META_PAYLOAD_PREFIX[] = "local:";
    constexpr int HYBRID_META_FACTOR_FOR_AUDIO = -1;

    // Inline methods
    inline uint8_t getAvptSampleQChannel(int vid) {
        switch (vid) {
            case 0: return REF_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q;
            case 1: return FIRST_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q;
            case 2: return SECOND_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q;
            case 3: return THIRD_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q;
            default: return INVALID_BYTE;
        }
    }

    inline int getViewNum(int vid) {
        return vid + VIEW_NUM_FACTOR;
    }

    // Bitrate monitoring
    constexpr int TX_BITRATE_SAMPLING_PERIOD_MS = 1000;
}

#endif // C_H