
# Alpha Streamer 3.1 - C++ refactored ver of alpha streamer 3.0
<br>

## Window 환경 Visual Studio 실행방법
- Window 환경에서 실행할 경우, vcpkg라는 윈도우용 C++ 패키지 매니저 설치 및 셋팅, 해당 패키지 매니저로 boost 라이브러리 설치, Visual Studio에 boost 라이브러리 연동하기, CMakeLists.txt로 Visual Studio 프로젝트 빌드하기라는 다소 많은 단계를 거쳐야 한다.
1. 우선 윈도우 커맨드 프롬프트를 연다.
윈도우 키 + R 눌러서 실행창 띄운 다음 cmd 라고 입력해서 엔터 치면 된다.<br><br/>

2. C 드라이브로 이동한 다음, vcpkg 공식 깃허브를 깃 클론한 후, vcpkg라는 디렉토리로 진입한다.<br>
```text
git clone https://github.com/microsoft/vcpkg.git
```

3. 아래의 명령어를 사용해서 vcpkg의 부트스트랩 스크립트를 실행시킨다.
   이 명령어의 맨 앞 글자에 '마침표'가 있을을 기억하라.
```text
.\bootstrap-vcpkg.bat
```

4. 아래의 명령어를 사용해서 boost의 모든 라이브러리를 vcpkg를 이용해서 전부 설치한다. 30분 이상 걸릴 수도 있다.
```text
.\vcpkg install boost
```

5. 아래의 명령어를 사용해서 Visual Studio에서 boost 라이브러리를 바로 사용할 수 있게 해준다.
```text
.\vcpkg integrate install
```

6. 설치가 전부 끝났다면, 아래의 명령어를 실행해서 본 프로젝트를 깃 클론 한 다음, Visual Studio를 연다. 이때 기존의 프로젝트를 오픈하거나, 새로운 프로젝트를 만들거나, 깃 리포지토리에서 복제해 오는 등의 미리 준비된 메뉴를 선택하지 않고 그냥 IDE를 열기만 해야 한다.
```text
git clone https://github.com/DongvinPark/MyFirstCppBoostAsio
```
<br>

7. Visual Studio의 File > Open > CMake 메뉴를 클릭한 다음, 본 프로젝트의 CMakeLists.txt를 선택하여 열어준다. Visual Stuio 2022 기준 여기까지 해주면 IDE가 알아서 빌드 및 프로젝트 임포트까지 다 해준다.
<br> 그래도 잘 되지 않을 경우, 본 프로젝트의 github URL을 Chat GPT에게 전달하면서, "how can I open this project in Visual Studio with boost library using vcpkg?"라고 물어보면 친절히 알려줄 것이다.
<br><br>

## Window 환경 CLion 실행방법
- boost 라이브러리를 설치하는 것은 Visual Studio에서 본 프로젝트를 실행하는 것과 거의 똑같다. Window 환경에서 실행할 경우, vcpkg라는 윈도우용 C++ 패키지 매니저 설치 및 셋팅, 해당 패키지 매니저로 boost 라이브러리 설치, CLion에 boost 포함시키라는 단계를 거쳐야 하지만, Visual Studio에서 실행시키는 것보다는 좀 더 간단하다.
1. 우선 윈도우 커맨드 프롬프트를 연다.
윈도우 키 + R 눌러서 실행창 띄운 다음 cmd 라고 입력해서 엔터치면 된다.<br>

2. C 드라이브로 이동한 다음,
vcpkg 공식 깃허브를 깃 클론 완료한 다음, vcpkg라는 디렉토리로 진입한다.<br>
```text
git clone https://github.com/microsoft/vcpkg.git
```

3. 아래의 명령어를 사용해서 vcpkg의 부트스트랩 스트립트를 실행시킨다.
   이 명령어의 맨 앞 글자에 '마침표'가 있을을 기억하라.
```text
.\bootstrap-vcpkg.bat
```

4. 아래의 명령어를 사용해서 boost의 모든 라이브러리를 vcpkg를 이용해서 전부 설치한다. 30분 이상 걸릴 수도 있다.
```text
.\vcpkg install boost
```

5. 설치가 전부 끝났다면, CLion을 실행한 후,
```text
File > setting > Build, Execution, Deployment > CMake
```
 의 순서로 메뉴에 진입한다.
그 안의 여러 칸들 중,
"CMake options"에다가 다음을 입력한다.
```text
-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```
이게 정확하게 작동하기 위해서는,
vcpkg 깃허브 리포지토리를 정확하게 C 드라이브 최상위 디렉토리에다가 클론한 상태여야 한다. 중요한 것은, -DCMAKE_TOOLCHAIN_FILE이 필요로 하는 값이 vcpkg.cmake 라는 파일이 실제로 위치하는 디렉토리 경로여야 한다는 점이다.
<br><br>

## Linux & MacOS(M series chip) CLion 실행방법
1. Linux OS가 설치된 AWS EC2 또는 Ubuntu 가 설치된 실제 컴퓨터를 준비한다.
2. 현재 깃허브 리포지토리를 준비된 머신에 클론한다.
3. 준비된 머신에 GCC, G++, CMake, Boost.Asio를 설치한다. Mac이라면 homebrew를 사용한다.
4. CmakeLists.txt의 내용을 아래의 내용으로 대체한 후, file(GLOB_RECURSE SOURCES ... ) 부분에서 '...' 부분에 src 디렉토리 내부의 모든 C++ 파일들을 입력해 준다.
```CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(AlphaStreamer3.1Cpp VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the Boost library
find_package(Boost REQUIRED COMPONENTS system)

# Include the directories for header files
include_directories(${Boost_INCLUDE_DIRS} include src)

# Collect all source files from subdirectories
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/dto/Res.cpp
)

# Create the executable
add_executable(AlphaStreamer3.1Cpp ${SOURCES})

# Link Boost libraries to the project
target_link_libraries(AlphaStreamer3.1Cpp ${Boost_LIBRARIES})
```
5. 리눅스 환경이라면, build_and_run_on_linux로 시작하는 .sh 파일에 'chmod +x .sh 파일명' 명령어로 실행권한을 준 다음, 실행하면 된다.<br>
6. MacOS 환경이라면, 프로젝트 root 디렉토리 내에 build 디렉토리를 만든 다음, 그 디렉토리로 이동해서 'cmake ..', 'make' 명령어를 순서대로 실행하여 실행파일을 만든다. 그 후 실행파일을 './실행파일이름' 명령어로 수동으로 실행하면 된다.<br>
7. Linux 또는 MacOS 환경에서 JetBrains CLion IDE를 사용하고, 3번 단계에서 언급한 툴들이 전부 설치돼 있다면 현재 프로젝트를 IDE로 열고 4.에서 설명한 대로 CMakeLists.txt를 변경한 다음 실행시키면 된다.
<br><br>

## MacOS(M series chip) Xcode 실행방법
1. mac에 Xcode, GCC, G++, CMake를 설치해준다. homebrew가 없다면 설치해준다. boost 라이브러리는 homebrew를 이용해서 설치해준다.
2. 본 프로젝트를 깃 클론한다.
3. 본 프로젝트 내의 CMakeLists.txt를 아래의 내용으로 통째로 교체한다.
```CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(AlphaStreamer3.1Cpp VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the Boost library
find_package(Boost REQUIRED COMPONENTS system)

# Include the directories for header files
include_directories(${Boost_INCLUDE_DIRS} include src)

# Collect all source files from subdirectories
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/dto/Res.h
    src/dto/Res.cc
)

# Create the executable
add_executable(AlphaStreamer3.1Cpp ${SOURCES})

# Link Boost libraries to the project
target_link_libraries(AlphaStreamer3.1Cpp ${Boost_LIBRARIES})
```
4. 깃 클론 완료한 본 프로젝트의 root 디렉토리로 이동해서 아래의 명령어를 실행한다. 그러면 본 프로젝트를 바탕으로 XcodeProject라는 폴더가 만들어진다.
```text
cmake -G Xcode -B XcodeProject
```
5. XcodeProject 폴더에 들어가서 AlphaStreamer3.1Cpp.xcodeproj라는 Xcode 프로젝트 파일을 실행한다. 그러면 Xcode IDE가 열린다.
6. Xcode 최상단 메뉴에서 Product >> Scheme >> Edit Scheme에 진입한 후, 왼쪽의 Run 탭에서 Executable 메뉴에서 AlphaStreamer3.1Cpp라는 본 프로젝트를 선택한 후 close 한다.
7. 그 후, IDE 왼쪽 상단의 'Run' 버튼(재생버튼 모양으로 생김)을 눌러서 실행한다.
