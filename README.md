# Rtsp Server In C++
<br>

### C++17 또는 그 이후 버전에서 작동합니다.
### boost library 1.71.0 또는 그 이상 버전을 권장합니다.
<br><br/>

## 본 프로젝트에서 배운 점 - 3 줄 요약
1. ***아무리 코드가 좋아도 머신의 H/W적 한계를 뛰어 넘을 수는 없습니다.***
    - 이건 Java도, C/C++도 똑같이 적용됩니다.
    - 단, 높은 부하 상황에서 더 오래 버티고 더 천천히 성능이 하락하게 만들 수는 있습니다.
<br><br/>
2. ***C/C++로 Java의 성능을 뛰어 넘으려면 수많은 테스트를 바탕으로 생각보다 많은 것들을 개발자가 수동으로 컨트롤해줘야 합니다.***
   - 어설프게 C/C++로 작성하느니 Java 나 Go 언어로 정확하게 구현하는 것이 훨씬 성능이 좋습니다.
<br><br/>
3. ***꼭 필요한 게 아니라면, C++ Standard Library를 사용하는 것이 성능/안정성 측면에서 유리합니다.***
<br><br/><br><br/>

## 버전 정보
- 1.0.0(2025.Feb.25) : Play, Pause, Seek, Cam Switching, Hybrid D & S, Looking Sample Control 지원
- 1.1.0(2025.April.4) : 성능 테스트 완료. RTP 패킷 전송 및 메모리 관리 아키텍처 수정.
<br><br/><br><br/>

## 개발 정책
- Java로 작성된 Rtsp Server를 C++로 재작성 합니다.
  - ***동시 접속 클라이언트들에게 최대한 동일한 성능의 스트리밍을 제공하는 것을 목표로 구현했습니다.***
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
  - [Boost Asio](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/overview/basics.html) : 고성능 Async-Nonblocking 네트워킹 제공
    - C++ 를 위한 socket을 제공합니다. 
    - 별도의 스레드 생성 없이, 주기적인 작업을 수행하는 Timer 객체를 만들 수 있습니다.
  - [Boost Strand](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/overview/core/strands.html) : io_context 내 task 간 동기화 수단 제공
    - CPU core 간의 context switching과 cache miss를 줄입니다.
  - [Boost Lockfree](https://www.boost.org/doc/libs/1_87_0/doc/html/lockfree.html) : 효율적인 multi pub/sub thread-safe queue 제공
    - non-blocking queue : mutex locking & unlocking 동작이 없습니다.
    - multi consumers and producers : 다수의 pub/sub 주체들을 지원할 수 있습니다.
<br><br/><br><br/>

## 재생 아키텍처
<img width="815" alt="Image" src="https://github.com/user-attachments/assets/4461a3a1-63eb-4460-ac8e-18662e1d49b0" /><br>
- 원본 컨텐츠를 Transcoder로 변환하여 AWS EFS(Elastic File System)에 업로드 합니다.
- EFS와 마운트된 EC2에서 RTSP 스트리밍 서버가 동작합니다.
- 클라이언트 디바이스 내부에서 작동하는 VR Player 인 AVPT 6.1이 RTSP 서버와 통신하면서 RTP 패킷을 수신합니다.
<br><br/><br><br/>

## 프로젝트 내부 구조
<img width="879" alt="Image" src="https://github.com/user-attachments/assets/31875ef2-fb14-4586-af59-937aa6142564" />
<br><br/><br><br/>

## 성능 테스트 결과
- 다수의 동시접속자들이 23 Mbps, 30 sec playtime, fps 30의 컨텐츠를 재생했을 때, 각 클라이언트들이 수신한 비디오 샘플 개수의 평균과 표준편차를 비교했습니다.
- 사용된 EC2는 t2.medium 입니다.
- 클라이언트 별로 수신한 비디오 샘플의 개수가 많고, 표준편차가 작을수록 좋습니다.
- 높은 부하가 가해질 때, C++ 버전이 성능의 감소가 Java 버전 대비 천천히 진행되는 것을 알 수 있습니다.
    - [테스트 결과 원본 데이터](https://github.com/DongvinPark/RtspServerInCpp/blob/main/performance_test_result.txt)
    - 동시접속 클라이언트 수 별 비디오 샘플 수신 횟수 평균
      ![Image](https://github.com/user-attachments/assets/59a44c7b-f18a-4748-9048-3b0ea8c7da3f)
    - 동시접속 클라이언트 수 별 비디오 샘플 수신 횟수의 표준편차
      ![Image](https://github.com/user-attachments/assets/04fd8540-1973-4c23-ab09-f79a347361eb)
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
  - Window 10 : C++ 17 or later, boost 1.86.0
  - Window 11 : C++ 17 or later, boost 1.86.0
- Linux
  - Ubuntu 20.04 LTS : C++ 17 or later, boost 1.71.0
  - Ubuntu 22.04 LTS : C++ 17 or later, boost 1.74.0
  - Amazon Linux 2023 : C++ 17 or later, boost 1.75.0
- macOS
  - Sequoia 15.2 : C++ 17 or later, boost 1.87.0
  - Sequoia 15.3 : C++ 17 or later, boost 1.87.0
<br><br/><br><br/>