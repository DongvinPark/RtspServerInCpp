
# Alpha Streamer 3.1 - C++ Rewrite of Alpha Streamer 3.0 with Boost.Asio
<br>

### C++17 또는 그 이상 버전이 필요합니다.
### boost library 1.71.0 또는 그 이상 버전을 권장합니다.
<br>

## 개발 정책
- ISO C++17 표준(ISO/IEC 14882:2017)을 준수 합니다.
  - 특정 컴파일러에서만 지원하는 기능은 사용하지 않았습니다(Variable Length Array 등).
- Windows, Linux, M chip MacOS 운영체제에서 실행 가능합니다.
  - 하나의 CMakeLists.txt 파일로 3 종류의 OS에서 빌드 및 실행이 가능합니다.
- Boost.asio 라이브러리를 사용하여 스레드의 생성을 최소화 했습니다.
  - 클라이언트가 늘어나도 스레드를 새로 생성하지 않습니다.
- std 라이브러리를 최대한 많이 사용하여 직접적인 메모리 관리를 최소화 했습니다.
  - std::unique_ptr, std::shared_ptr, std::weak_ptr 를 활용했습니다.
  - new, delete, raw pointer의 사용을 최소화 했습니다.
<br><br/>

## OS별 빌드 및 실행 테스트 결과
- Windows
  - Window 11 : C++ 17 or later, boost 1.86.0
- Linux
  - Ubuntu 20.04 LTS : C++ 17 or later, boost 1.71.0
  - Ubuntu 22.04 LTS : C++ 17 or later, boost 1.74.0
  - Amazon Linux 2023 : C++ 17 or later, boost 1.75.0
- MacOS
  - Sequoia 15.2 : C++ 17 or later, boost 1.87.0
  - Sequoia 15.3 : C++ 17 or later, boost 1.87.0
<br><br/>

## 개발환경 셋팅
- 각자의 컴퓨터에 맞는 C++ 컴파일러, boost 라이브러리가 설치돼 있어야 합니다.
  - Windows
    - Visual Studio 커뮤니티 에디션을 설치한 후, vcpkg로 boost 라이브러리를 설치하고 둘을 링크 해줍니다.
  - MacOS
    - GCC, G++, CMake를 터미널 명령어로 설치한 후, boost.asio를 homebrew로 설치해줍니다.
  - Linux - Ubuntu Desktop
    - GCC, G++, CMake, boost.asio를 터미널 명령어로 설치해줍니다.
- 현재 master 브랜치에 등록돼 있는 CMakeLists.txt는 아래의 IDE에서 빌드 및 실행이 가능합니다.
  - native Windows Visual Studio, CLion
  - Linux ubuntu 20.04 CLion
  - M chip MacOS CLion, Visual Studio Code
- 다른 OS 또는 IDE에서는 추가 설정이 필요할 수 있습니다.
<br><br/>

## Window 환경 Visual Studio 실행방법
- Window 환경에서 실행할 경우, vcpkg라는 윈도우용 C++ 패키지 매니저 설치 및 셋팅, 해당 패키지 매니저로 boost 라이브러리 설치, Visual Studio에 boost 라이브러리 연동하기, CMakeLists.txt로 Visual Studio 프로젝트 빌드하기라는 다소 많은 단계를 거쳐야 합니다.
1. 우선 윈도우 커맨드 프롬프트를 엽니다.
윈도우 키 + R 눌러서 실행창 띄운 다음 cmd 라고 입력해서 엔터 치면 됩니다.<br><br/>

2. C 드라이브로 이동한 다음, vcpkg 공식 깃허브를 깃 클론한 후, vcpkg라는 디렉토리로 진입합니다.<br>
```text
git clone https://github.com/microsoft/vcpkg.git
```

3. 아래의 명령어를 사용해서 vcpkg의 부트스트랩 스크립트를 실행시킵니다.
   이 명령어의 맨 앞 글자에 '마침표'가 있을을 기억합니다.
```text
.\bootstrap-vcpkg.bat
```

4. 아래의 명령어를 사용해서 boost의 모든 라이브러리를 vcpkg를 이용해서 전부 설치해 줍니다. 30분 이상 걸릴 수도 있습니다.
```text
.\vcpkg install boost
```

5. 아래의 명령어를 실행하여 Visual Studio에서 boost 라이브러리를 바로 사용할 수 있게 해줍니다.
```text
.\vcpkg integrate install
```

6. 설치가 전부 끝났다면, 아래의 명령어를 실행해서 본 프로젝트를 깃 클론 한 다음, Visual Studio를 엽니다. 이때 기존의 프로젝트를 오픈하거나, 새로운 프로젝트를 만들거나, 깃 리포지토리에서 복제해 오는 등의 미리 준비된 메뉴를 선택하지 않고 그냥 IDE를 열기만 해야 합니다. Visual Studio가 열렸다면, 화면 우측에 있는 '코드를 사용하지 않고 계속'이라는 항목을 클릭하면 됩니다.
```text
git clone https://github.com/DongvinPark/MyFirstCppBoostAsio
```
<br>

7. Visual Studio의 File > Open > CMake 메뉴를 클릭한 다음, 본 프로젝트의 CMakeLists.txt를 선택하여 열어줍니다. Visual Stuio 2022 기준 여기까지 해주면 IDE가 알아서 빌드 및 프로젝트 임포트까지 다 해줍니다.
<br> 그래도 잘 되지 않을 경우, 본 프로젝트의 github URL을 Chat GPT에게 전달하면서, "how can I open this project in Visual Studio with boost library using vcpkg?"라고 물어보면 친절히 알려줄 것입니다.
<br><br/>

## Window 환경 CLion 실행방법
- boost 라이브러리를 설치하는 것은 Visual Studio에서 본 프로젝트를 실행하는 것과 똑같습니다. Window 환경에서 실행할 경우, vcpkg라는 윈도우용 C++ 패키지 매니저 설치 및 셋팅, 해당 패키지 매니저로 boost 라이브러리 설치, CLion에 boost 포함시키라는 단계를 거쳐야 하지만, Visual Studio에서 실행시키는 것보다는 좀 더 간단합니다.
1. 우선 윈도우 커맨드 프롬프트를 엽니다.
윈도우 키 + R 눌러서 실행창 띄운 다음 cmd 라고 입력해서 엔터를 치면 됩니다.<br>

2. C 드라이브로 이동한 다음,
vcpkg 공식 깃허브를 깃 클론 완료한 다음, vcpkg라는 디렉토리로 진입합니다.<br>
```text
git clone https://github.com/microsoft/vcpkg.git
```

3. 아래의 명령어를 사용해서 vcpkg의 부트스트랩 스크립트를 실행시킵니다.
   이 명령어의 맨 앞 글자에 '마침표'가 있을을 기억합니다.
```text
.\bootstrap-vcpkg.bat
```

4. 아래의 명령어를 사용해서 boost의 모든 라이브러리를 vcpkg를 이용해서 전부 설치해줍니다. 30분 이상 걸릴 수도 있습니다.
```text
.\vcpkg install boost
```

5. 설치가 전부 끝났다면, CLion을 실행한 후,
```text
File > setting > Build, Execution, Deployment > CMake
```
 의 순서로 메뉴에 진입합니다.
그 안의 여러 칸들 중,
"CMake options"이라고 표기돼 있는 칸에 아래의 내용을 입력합니다.
```text
-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```
이게 정확하게 작동하기 위해서는,
vcpkg 깃허브 리포지토리를 정확하게 C 드라이브 최상위 디렉토리에다가 클론한 상태여야 합니다. 중요한 것은, -DCMAKE_TOOLCHAIN_FILE이라는 환경 변수에 주입해주는 값이 vcpkg.cmake 라는 파일이 실제로 위치하는 디렉토리 경로여야 한다는 점입니다.
<br><br/>

## Linux & MacOS(M series chip) CLion 실행방법
1. Linux OS가 설치된 AWS EC2 또는 Ubuntu 가 설치된 실제 컴퓨터를 준비합니다.
2. 현재 깃허브 리포지토리를 준비된 머신에 클론합니다.
3. 준비된 머신에 GCC, G++, CMake, Boost.Asio를 설치합니다. Mac이라면 homebrew를 사용합니다.
4. 리눅스 환경이라면, fast_build_and_run_for_dev_linux_foreground.sh 파일에 'chmod +x .sh 파일명' 명령어로 실행권한을 준 다음, 실행하면 됩니다.<br>
5. MacOS 환경이라면, delete_build_and_run_on_Mchip_MacOS.sh 파일에 'chmod +x .sh 파일명' 명령어로 실행권한을 준 다음, 실행하면 됩니다.<br>
6. Linux 또는 MacOS 환경에서 JetBrains CLion IDE를 사용하고, 3번 단계에서 언급한 툴들이 전부 설치돼 있다면 현재 프로젝트를 IDE로 열고 바로 실행시키면 됩니다.
<br><br/>

## MacOS(M series chip) Visual Studio Code 실행방법
1. mac에 Xcode, GCC, G++, CMake를 설치해줍니다. homebrew가 없다면 설치해줍니다. boost 라이브러리는 homebrew를 이용해서 설치해줍니다.
2. 본 프로젝트를 깃 클론합니다.
3. 아래의 명령으로 delete_build_and_run_on_Mchip_MacOS.sh 스크립트에 실행 권한을 줍니다.
```text
sudo chmod +x delete_build_and_run_on_Mchip_MacOS.sh
```
4. 스크립트를 실행합니다.
```text
./delete_build_and_run_on_Mchip_MacOS.sh
```
5. 프로젝트 루트 디렉토리를 보면, .vscode 라는 폴더가 있을 것입니다. 여기에서 c_cpp_properties.json 파일이 있을 것인데, 아래와 같이 "iucludePath" 부분에 boost 라이브러리가 설치된 경로인 "/opt/homebrew/opt/boost/include"를 추가해줍니다. 이렇게 해야, VS Code 에서 "#include <boost/asio.hpp>"를 정상적으로 인식할 수 있습니다.
```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "/opt/homebrew/opt/boost/include"
            ],
            "defines": [],
            "macFrameworkPath": [
                "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks"
            ],
            "compilerPath": "/usr/bin/clang",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "macos-clang-arm64"
        }
    ],
    "version": 4
}
```