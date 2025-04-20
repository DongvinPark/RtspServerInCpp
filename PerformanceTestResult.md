# 성능 테스트 결과

## 테스트 방법
- 이미지 첨부 필요
- ***1 개의 컨테이너 == 1 명의 RTSP 스트리밍 시청 클라이언트가 성립함.***
- ***순수하게 RTSP 서버에만 부하를 주기 위해서***
  - 로컬 WIFI 라우터를 사용하지 않고 AWS 내부망 만을 사용함.
  - RTSP 서버 EC2와 클라이언트 EC2 를 AWS의 같은 VPC에 배치함.
  - Client Archive EC2를 RTSP 스트리밍 서버 EC2보다 고성능 & 고스펙으로 설정함.
- 특정 개수의 client 컨테이너들이 Commander 서버의 명령을 받고 동시에 RTSP 서버로 컨텐츠 재생 요청을 보냄.
- RTSP 서버는 동시접속 클라이언트 전원에게 컨텐츠 재생에 필요한 RTP 패킷들을 전송함.
- 동시 접속 클라이언트의 수는 50, 100, 150, 200, 250, 300 명으로 50명씩 늘려나감.
- 재생 컨텐츠 특성
  - Bitrate : 23 Mbps
  - Play time duration : 30 sec
  - fps : 30
  - total video sample cnt : 900
  - GOP : 60
<br><br/>

## 용어 정의
- ***동시 접속 클라이언트***
  - RTSP 서버의 입장에서 최초 접속 클라이언트와 마지막 접속 클라이언트 간의 시간 차이가 5초 미만이고 그 사이에 클라이언트들이 접속을 완료했을 때, 클라이언트 전체를 테스트 결과에 반영 가능한 유효한 ***동시 접속 클라이언트***로 간주합니다.
- ***Perfect Sending*** : 30초의 컨텐츠 재생 시간 동안에 900 개의 비디오 샘플을 모든 동시접속 클라이언트들에게 전송하는 것을 뜻합니다. 
- ***Sample Loss*** : 30초의 컨텐츠 재생 시간 동안에 수신한 비디오 샘플의 개수가 900 개보다 적은 클라이언트가 발생한 상황을 뜻합니다.
<br><br/>

## 테스트 결과 요약
1. ***아무리 코드가 좋아도 머신의 H/W적 한계를 뛰어 넘을 수는 없습니다.***
   - ***[단, 높은 부하 상황에서 더 오래 버티고, 더 천천히 성능이 하락하고, 더 균일한 사용자 경험을 제공하게 만들 수는 있습니다.]()***
   <br><br/>
2. ***C/C++로 Java의 성능을 뛰어 넘으려면 수많은 테스트를 바탕으로 생각보다 많은 것들을 개발자가 수동으로 제어해야 합니다.***
   - 어설프게 C/C++로 작성하느니 Java 나 Go 언어로 정확하게 구현하는 것이 훨씬 성능이 좋습니다.
   <br><br/>

## 테스트 결과
### 해석
- **_C++ 버전이 더 높은 부하가 가해지는 상황에서도 더 많은 비디오 샘플들을 더 작은 표준편차로 전송합니다._**
- **_C++ 버전이 Java 버전보다 더 많은 수의 동시 접속 클라이언트들에게 Perfect Sending 이 가능합니다._**
- **_Java 버전에서는 동시접속 클라이언트 숫자가 늘어남에 따라 성능이 더 빨리 하락하고, 클라이언트 별로 수신 되는 비디오 샘플 개수의 편차가 급증합니다._**
- 원인은 두 서버의 클라이언트 별 리소스 할당의 차이에 있습니다.
  - Java 버전에서는 클라이언트 1 명 당 3 개의 LinkedBlockingQueue와 7 개의 Thread를 만들어냅니다.
    - Rtp & RTSP Res Tx : 스레드 1 개와 LinkedBlockingQueue 1개 할당
    - RTSP Req Receive : 스레드 1 개 할당
    - Video & Audio Sample Reading Timer : 스레드 2 개 할당
    - Video Rtp Queue Tx & Audio Rtp Queue Tx : 스레드 2 개와 LinkedBlockingQueue 2 개 할당
    - Bitrate Record Saving : 스레드 1 개 할당
  - C++ 버전에서는 클라이언트 1 명 당 1 개의 Lockfree Queue와 1 개의 스레드만으로 충분합니다. 기능은 Java 버전과 동일하지만, 많은 부분을 별도의 스레드 생성 없이 Boost Asio의 io_context에게 맡기기 때문입니다.
    - Rtp Tx : 스레드 1 개와 Boost Lockfree Queue 1 개 할당
    - RTSP Req/Res Transaction : io_context의 async read/write 활용
    - Video Timer & Audio Timer : io_context의 steady_timer 활용
    - Video Rtp Queue Tx & Audio Rtp Queue Tx : std::shared_ptr와 Lockfree Queue 활용
    - Bitrate Record Saving : io_context의 steady_timer 활용

### 클라이언트 별 수신 비디오 샘플 개수 표준 편차
![Image](https://github.com/user-attachments/assets/29a96abc-34f5-4c90-a5ff-1a50239c6936)
<br>

### 클라이언트 별 수신 비디오 샘플 개수 평균
![Image](https://github.com/user-attachments/assets/4085cbc8-b858-49f7-ac59-461ef0dc052f)
<br>