#ifndef C_H
#define C_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <cstdint> // For int64_t

namespace C {
    // version
    constexpr char VER[] = "2.0.0";

    // class names
    constexpr char MAIN[] = "main";
    constexpr char SNTP_REF_TIME_PROVIDER[] = "SntpRefTimeProvider";
    constexpr char PERIODIC_TASK[] = "PeriodicTask";
    constexpr char FILE_READER[] = "FileReader";
    constexpr char CONTENTS_STORAGE[] = "ContentsStorage";
    constexpr char SERVER[] = "Server";
    constexpr char SESSION[] = "Session";
    constexpr char RTSP_HANDLER[] = "RtspHandler";
    constexpr char ACS_HANDLER[] = "AcsHandler";
    constexpr char RTP_HANDLER[] = "RtpHandler";

    // General constants
    constexpr char MY_NAME[] = "alphaStreamer-3.1/2.0.0";
    constexpr int FRONT_VIDEO_VID = 0;
    constexpr int REAR_VIDEO_VID = 1;
    constexpr int SESSION_KEY_BIT_SIZE = 64;
    constexpr int SESSION_CLOSE_TIMEOUT_MS = 10*1000;
    constexpr int DELAY_BEFORE_RTP_START = 500;
    constexpr int DELAY_BEFORE_RTP_START_ON_SEEK = 100;
    constexpr int SOCKET_TIMEOUT_MS = 10*1000;
    constexpr char EMPTY_STRING[] = "";
    constexpr int ZERO = 0;
    constexpr int ONE = 1;
    constexpr int WRONG_SESSION_ID_TOLERANCE_CNT = 5;
    constexpr int URL_SPLIT_BY_SEMI_COLON_LENGTH = 3;
    constexpr int SHUTDOWN_SESSION_CLEAR_TASK_INTERVAL_MS = 30000;
    constexpr int SESSION_OBJECT_DELETE_INTERVAL_SEC = 20;
    constexpr int STOPPED_TASK_DELETE_DELAY_MS = 3000;

    constexpr int TCP_RTP_HEAD_LEN = 4; // $+(ch 1) + (len 2)
    constexpr int RTP_HEADER_LEN = 12; // Refer to https://datatracker.ietf.org/doc/html/rfc7798
    constexpr int UNSET = -1;
    constexpr int INVALID = -1;

    // Clock rates
    constexpr int H265_CLOCK_RATE = 90000; // by spec, 90kHz must be used.
    constexpr int AAC_CLOCK_RATE = 48000; // Audio has 48kHz when included in video.

    // Strings and MTU size
    constexpr char CRLF[] = "\r\n";
    constexpr char CRLF2[] = "\r\n\r\n";
    constexpr int MTU_SIZE = 1472; // UDP max payload size
    constexpr char DEFAULT_MEDIA_TYPE[] = "mp4";
    constexpr int DEFAULT_DIVIDE_NUM = 3;
    constexpr char DEFAULT_IP[] = "127.0.0.1";

    // Network settings
    constexpr int RTP_TX_QUEUE_SIZE = 5*1024;
    constexpr int FRONT_VIDEO_MAX_BYTE_SIZE = 2*1024*1024; //2MB
    constexpr int REAR_VIDEO_MAX_BYTE_SIZE = 1*1024*1024; //1MB
    constexpr int FRONT_VIDEO_SAMPLE_POOL_RTP_MAX_LEN
        = FRONT_VIDEO_MAX_BYTE_SIZE/(TCP_RTP_HEAD_LEN + MTU_SIZE) + 1; // 2132
    constexpr int REAR_VIDEO_SAMPLE_POOL_RTP_MAX_LEN
        = REAR_VIDEO_MAX_BYTE_SIZE/(TCP_RTP_HEAD_LEN + MTU_SIZE) + 1; // 1421
    constexpr int AUDIO_MAX_BYTE_SIZE = 1500;
    constexpr int RTP_RX_PORT = 9000;
    constexpr int RTCP_RX_PORT = 9001;
    constexpr int RTSP_RTP_TCP_PORT = 8554;

    // File reading
    constexpr char ASC[] = "asc";
    constexpr char ASA[] = "asa";
    constexpr char ASV[] = "asv";
    constexpr char RTSP_MSG_DELIMITER[] = "###\r\n";
    constexpr int META_LEN_BYTES = 4;

    // Keys
    constexpr char SSRC_KEY[] = "ssrc=";
    constexpr char SEQ_KEY[] = "seq0=";
    constexpr char TIMESTAMP_KEY[] = "timestamp0=";
    constexpr char GOP_KEY[] = "gop=";
    constexpr char FRAME_COUNT_KEY[] = "framecount=";
    constexpr char MEDIA_INFO_KEY[] = "v=";
    constexpr char PLAY_TIME_KEY[] = "playtime=";

    // RTSP
    constexpr int RTSP_MSG_BUFFER_SIZE = 10*1024; // 10 KB
    const std::vector<std::string> RTSP_METHOD_VECTOR = {
        "DESCRIBE","SETUP","PLAY","PAUSE","TEARDOWN","SET_PARAMETER","OPTIONS" // !! "OPTIONS" must be the last.
    };
    constexpr int OK = 200;
    constexpr int BAD_REQUEST = 400;
    constexpr int METHOD_NOT_ALLOWED = 405;
    constexpr int SESSION_NOT_FOUND = 454;
    constexpr int NOT_IMPLEMENTED = 501;
    constexpr int INTERNAL_SERVER_ERROR = 500;
    const std::unordered_map<int, std::string> RTSP_STATUS_CODES_MAP = {
        {200, "OK"},
        {400, "Bad Request"},
        {405, "Method Not Allowed"},
        {454, "Session Not Found"},
        {501, "Not Implemented"},
        {500, "Internal Server Error"}
    };
    constexpr int64_t CLIENT_CONNECTION_LOSS_THRESHOLD_DURATION_MS = 20*1000; // 20 sec

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
    constexpr int KEY_FRAME_FLAG = 1;
    constexpr int P_FRAME_FLAG = 0;

    // SNTP
    constexpr char SNTP_SERVER_HOST[] = "time.android.com";
    constexpr int NTP_PORT = 123;
    constexpr int NTP_MODE_CLIENT = 3;
    constexpr int NTP_VERSION = 3;
    constexpr int HOST_RSP_TIMEOUT_MS = 3000;
    constexpr int NTP_READ_PERIOD = 3000;
    constexpr int REFERENCE_TIME_OFFSET = 16;
    constexpr int ORIGINATE_TIME_OFFSET = 24;
    constexpr int RECEIVE_TIME_OFFSET = 32;
    constexpr int TRANSMIT_TIME_OFFSET = 40;
    constexpr int NTP_PACKET_SIZE = 48;
    constexpr int64_t OFFSET_1900_TO_1970 = 2208988800L;//((365L * 70L) + 17L) * 24L * 60L * 60L;

    // Monitoring HTTP Server
    constexpr int MONITORING_HTTP_SERVER_PORT = 80;
    constexpr char QUOTATION_MARK = '"';
    const std::set<std::string> VIDEO_ID_SET = {
        "10", "11", "12", "13", "20", "21", "22", "23", "30", "31", "32", "33"
    };

    // Pause Seek
    constexpr int RE_PAUSE_DELAY_TIME_SEC = 1;
    constexpr int RE_PAUSE_DELAY_TIME_MILLIS = 1000;
    constexpr float START_NPT[] = {0.0f, -1.0f};

    // Filtering content
    constexpr int FILE_NUM_LIMIT = 10;

    // Looking Sample Control
    constexpr int INVALID_SAMPLE_NO = -1;
    constexpr char USE_P_FRAME_CONTROL[] = "PFrameControl";
    constexpr char SEND_P_FRAMES[] = "send-p";
    constexpr char STOP_P_FRAMES[] = "stop-p";
    constexpr int P_FRAME_GOP_CONTROL_FACTOR_FOR_SEEK = 2;
    constexpr int FAST_TX_FACTOR_FOR_CAM_SWITCHING = 25;
    constexpr int UPDATE_CAM_SWITCHING_STATUS_DELAY_MILLIS = 2000;

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
    constexpr int64_t INVALID_OFFSET = -1L;
    constexpr uint8_t REF_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x00;
    constexpr uint8_t FIRST_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x04;
    constexpr uint8_t SECOND_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x06;
    constexpr uint8_t THIRD_MEMBER_VIDEO_CHANNEL_FOR_AVPT_SAMPLE_Q = 0x08;
    constexpr uint8_t INVALID_BYTE = 0xFF;
    constexpr int RTP_CHANNEL_INFO_META_LENGTH = 4;
    constexpr int VIEW_NUM_FACTOR = 1;
    constexpr char HYBRID_META_PAYLOAD_PREFIX[] = "local:";
    constexpr int HYBRID_META_FACTOR_FOR_AUDIO = -1;
    constexpr char DUMMY_CONTENT_BASE[] = "rtsp://0.0.0.0:0/test";

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