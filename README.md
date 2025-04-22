# Rtsp Server In C++
<br>

### C++17 또는 그 이후 버전에서 작동합니다.
### boost library 1.71.0 또는 그 이상 버전을 권장합니다.
<br><br/>

## 이 프로젝트를 시작한 이유
- **_[RTSP 스트리밍 서버에 동시접속 클라이언트가 집중될 때 async non-blocking I/O가 sync blocking I/O 보다 더 많은 클라이언트에게 더 안정적인 서비스를 제공할 수 있는가?](https://github.com/DongvinPark/RtspServerInCpp)_** 를 확인하기 위해서 입니다.
<br><br/><br><br/>

## 프로젝트에서 배운 점
### **_코드가 아무리 좋아도 H/W의 한계를 뛰어 넘을 수는 없습니다._**
- 성능은 '코드' 만으로 결정되지 않습니다.
- CPU, RAM, NIC, SSD, Router, 클라이언트와 트래픽의 특징 ... 등의 다른 H/W들과 외부 조건들의 영향을 받습니다.
<br><br/>
### **_Async Non-blocking은 만능이 아닙니다._**
- async non-blocking I/O 를 적절한 곳에 사용하지 않으면 sync blocking I/O 보다 오히려 성능이 떨어집니다.
- **적은 수의 클라이언트들이 장기간 고성능(high bitrate, low latency, real time, RTSP,...)을 필요로 하면,**
  - **sync blocking with 1 thread per client 가 적절하고,**
- **많은 수의 클라이언트들이 단기간 중/저 성능(web server, proxy, chat, msg broker, HTTP,...)을 필요로 하면,**
  - **async nonblocking with limited thread pool 이 적절합니다.**
<br><br/><br><br/>

## 설계 목표
0. **_Java로 작성된 RTSP 서버를 C++로 재작성 합니다._**
1. **_최대한 많은 비디오 샘플들을 동시 접속 클라이언트들에게 전송합니다._**
2. **_동일한 시간 동안에 클라이언트들이 받은 비디오 샘플의 개수의 편차를 최소화 합니다._**
<br><br/><br><br/>

## 성능 테스트 결과
- **_Java 버전보다 약 33% 더 많은 동시접속 클라이언트들을 감당할 수 있습니다._**
  - c6i.xlarge EC2에서 클라이언트 1 명당 23 Mbps 컨텐츠 스트리밍 시,
    - Java : 150 명
    - C++ : 200 명
- ***[테스트 결과 보고서](https://github.com/DongvinPark/RtspServerInCpp/blob/main/PerformanceTestResult.md)***
- ***[테스트 원본 데이터](https://github.com/DongvinPark/RtspServerInCpp/tree/main/originnal-test-results)***
<br><br/><br><br/>

## 프로젝트 내부 구조
<img width="879" alt="Image" src="https://github.com/user-attachments/assets/31875ef2-fb14-4586-af59-937aa6142564" /><br>
- main io_context와 worker io_context pool이 있습니다.
- main io_context는 네트워킹에 필요한 소켓을 만들고, RTSP Message Transaction을 담당합니다.
- worker io_context pool은 네트워킹 이외에 필요한 모든 async task, concurrent task, delayted task, non-blocking task 들의 스케줄링과 실행을 담당합니다. 기존에 새로운 스레드를 만들어서 처리하던 이러한 task들을 스레드 생성 없이 처리할 수 있게 해줍니다.
- Video & Audio 샘플을 읽는 태스크들은 RTP 패킷들을 lockfree queue에 집어 넣습니다.
- RTP Tx 스레드는 lockfree queue에서 RTP 패킷들 꺼내서 클라이언트에게 전송합니다.
- Content File Meta는 싱글톤 불변 클래스이며, 서버 내 모든 Session들이 이를 참조하여 RTP 패킷들의 메타정보를 읽어들입니다. 이 메티정보를 참조하여 std::ifstream(==Stream Handler)에서 비디오 스트림 재생에 필요한 데이터들을 읽어들입니다.
<br><br/><br><br/>

## 컨텐츠 전달 아키텍처
<img width="815" alt="Image" src="https://github.com/user-attachments/assets/4461a3a1-63eb-4460-ac8e-18662e1d49b0" /><br>
- 원본 컨텐츠를 Transcoder로 변환하여 AWS EFS(Elastic File System)에 업로드 합니다.
- EFS와 마운트된 EC2에서 RTSP 스트리밍 서버가 동작합니다.
- 클라이언트 디바이스 내부에서 작동하는 VR Player 인 AVPT 6.1이 RTSP 서버와 통신하면서 RTP 패킷을 수신합니다.
<br><br/><br><br/>

## Troubleshooting
1. **TEARDOWN 한 Session 제거 시 SIGSEGV 발생 문제**
   - [Async task를 수행하는 객체를 삭제하는 방법](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#35-socket%EC%9D%84-%EC%9D%B4%EC%9A%A9%ED%95%98%EC%97%AC-async%ED%95%9C-%EC%9E%91%EC%97%85%EC%9D%84-%EC%B2%98%EB%A6%AC%ED%95%98%EB%8A%94-%EA%B0%9D%EC%B2%B4%EB%8A%94-%EC%96%B4%EB%96%BB%EA%B2%8C-%EC%82%AD%EC%A0%9C%ED%95%B4%EC%95%BC-%ED%95%98%EB%8A%94%EA%B0%80)
     <br><br/>
2. **Out of Memory로 인한 서버 강제 종료 문제**
   - [boost pool와 OOM의 위험성](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#42-boostpool%EA%B3%BC-boostobject_pool%EC%9D%98-%EC%9C%84%ED%97%98%EC%84%B1--out-of-memory-%EB%B0%9C%EC%83%9D-%EA%B0%80%EB%8A%A5)
   - [C/C++ 프로그램의 메모리 사용량 추적하기](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#43-cc-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%A8%EC%9D%98-%EB%A9%94%EB%AA%A8%EB%A6%AC-%EC%82%AC%EC%9A%A9%EB%9F%89%EC%9D%84-%EC%B6%94%EC%A0%81%ED%95%98%EA%B3%A0-%EA%B4%80%EB%A0%A8-%EC%9D%B4%EC%8A%88%EB%A5%BC-%EC%A7%84%EB%8B%A8%ED%95%98%EB%8A%94-%EB%B0%A9%EB%B2%95)
   - [클라이언트 당 할당 가능한 샘플 사이즈 제한하기](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#46-%EC%8A%A4%ED%8A%B8%EB%A6%AC%EB%B0%8D-%EC%84%9C%EB%B2%84%EC%97%90%EC%84%9C-%EA%B7%BC%EB%B3%B8%EC%A0%81%EC%9C%BC%EB%A1%9C-oomout-of-memory-%EB%AC%B8%EC%A0%9C%EB%A5%BC-%EC%98%88%EB%B0%A9%ED%95%98%EB%8A%94-%EB%B0%A9%EB%B2%95)
     <br><br/>
3. **RTP 패킷 전송 성능 저하 문제**
   - [worker io_context pool 을 도입하기](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#44-boostasioio_context-pool-%EB%8F%84%EC%9E%85%EC%9D%98-%ED%95%84%EC%9A%94%EC%84%B1%EA%B3%BC-%EB%B0%A9%EB%B2%95)
   - [while true loop 블록킹하여 spin loop 제거하기](https://github.com/DongvinPark/RtspServerInCpp/blob/main/CppTips.md#45-whiletruetransmitrtp-%EB%A3%A8%ED%94%84%EC%9D%98-%EC%9C%84%ED%97%98%EC%84%B1)
<br><br/><br><br/>

## 버전 정보
- 1.0.0(2025.Feb.25) : Play, Pause, Seek, Cam Switching, Hybrid D & S, Looking Sample Control 지원
- 1.1.0(2025.April.4) : 1차 성능 테스트 완료. RTP 패킷 전송 및 메모리 관리 아키텍처 수정.
- 1.1.1(2025.April.20) : 2차 성능 테스트 완료. 클라이언트 별 RTP 버퍼 사이즈 제한 조정.
<br><br/><br><br/>

## 개발 정책
- ISO C++17 표준(ISO/IEC 14882:2017)을 준수 합니다.
  - 특정 컴파일러에서만 지원하는 기능은 사용하지 않았습니다(Variable Length Array 등).
- 멀티 플랫폼에서의 실행을 지원합니다.
  - Windows, Linux(Ubuntu, Amazon Linux), macOS(M chip)에서 실행 가능합니다.
- Boost 라이브러리를 사용합니다.
  - 다수의 유명한 기술 기업에서 사용되고 있으며, 성능과 안정성이 검증 되었습니다.
- 직접적인 메모리 관리를 최소화 했습니다.
  - RAII를 준수합니다.
  - new, delete 연산을 직접 호출하는 코드가 없습니다.
<br><br/><br><br/>

## Why C++17 and Boost Libray?
### [C++17](https://www.iso.org/standard/68564.html)
  - 고수준의 가독성 높은 코드와 기능들(class, smart pointer, iterator ...)을 이용하면서도 성능에서 손해가 발생하지 않습니다.
  - std::filesystem, std::optional 을 지원하는 최초의 모던 C++ 입니다.
### [Boost Library](https://www.boost.org/)
  - 1999 년 출시 후 현재까지 널리 사용되고 있으며 안정성이 확보된 라이브러리
    - Google, Microsoft, Amazon, Meta, Bloomberg, Goldman Sachs 등에서 사용합니다.
    - C++ 표준 라이브러리에 편입될 정도로 안정성과 성능이 검증 된 라이브러리 입니다.
      - filesystem, shared_ptr, weak_ptr, unique_ptr, thread, mutex, chrono, unordered_map 등의 기능이 boost library에서 처음 구현된 후 C++의 standard library에 편입 되었습니다.
    - boost 라이브러리 개발자 중 다수는 C++ 표준 위원회에 소속돼 있습니다.
      - ex : Bjarne Stroustrup, Herb Sutter, Howard Hinnant, Peter Dimov, Thomas Witt, Andrei Alexandrescu ... 
### [Boost Library Used in This RTSP Server Implementation](https://github.com/boostorg)
  - [Boost Asio](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/overview/basics.html) : 고성능 Async Task Scheduler
    - C++ 를 위한 socket(blocking, non-blocking)을 제공합니다. 
    - 스레드 생성 없이 delayed task, periodic task, non-blocking task를 처리할 수 있습니다. 
  - [Boost Strand](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/overview/core/strands.html) : io_context 내 task 간 동기화 수단 제공
    - CPU core 간의 context switching과 cache miss를 줄입니다.
  - [Boost Lockfree](https://www.boost.org/doc/libs/1_87_0/doc/html/lockfree.html) : 효율적인 multi pub/sub thread-safe queue 제공
    - non-blocking queue : mutex locking & unlocking 동작이 없습니다.
    - multi consumers and producers : 다수의 pub/sub 주체들을 지원할 수 있습니다.
<br><br/><br><br/>

## Install Dependencies
- 각자의 컴퓨터에 맞는 C++ 컴파일러, boost 라이브러리가 설치돼 있어야 합니다.
- 각 운영체제 & IDE 종류별 해당 프로젝트 실행 방법은 [SetUp.md](https://github.com/DongvinPark/RtspServerInCpp/blob/main/SetUp.md) 문서를 참조합니다.
  - Windows
    - Visual Studio 커뮤니티 에디션을 설치한 후, vcpkg로 boost 라이브러리를 설치하고 둘을 링크 해줍니다.
  - macOS
    - GCC, G++, CMake를 터미널 명령어로 설치한 후, boost.asio를 homebrew로 설치해줍니다.
  - Linux - Ubuntu Desktop
    - GCC, G++, CMake, boost.asio를 터미널 명령어로 설치해줍니다.
- main 브랜치는 아래의 OS 및 IDE에서 빌드 및 실행이 가능합니다.
  - native Windows
    - CLion 
    - Visual Studio
  - Linux ubuntu 20.04 LTS desktop
    - CLion
  - M chip macOS
    - CLion
    - Visual Studio Code
<br><br/><br><br/>

## 프로젝트 실행이 가능한 OS, C++ 언어, Boost 라이브러리 버전 
- Windows
  - Window 10 : C++ 17 or later, boost 1.86.0 or later
  - Window 11 : C++ 17 or later, boost 1.86.0 or later
- Linux
  - Ubuntu 20.04 LTS : C++ 17 or later, boost 1.71.0 or later
  - Ubuntu 22.04 LTS : C++ 17 or later, boost 1.74.0 or later
  - Amazon Linux 2023 : C++ 17 or later, boost 1.75.0 or later
- macOS
  - Sequoia 15.2 : C++ 17 or later, boost 1.87.0 or later
  - Sequoia 15.3 : C++ 17 or later, boost 1.87.0 or later
<br><br/><br><br/>