
# Alpha Streamer 3.1 - C++ refactored ver of alpha streamer 3.0
<br>

## C++17 버전에서 실행시키는 것을 권장합니다.
<br>

## Jetbrains 사의 CLion IDE를 사용하는 것을 추천합니다.
<br>

## 개발 정책
- Windows, Ubuntu Linux, M chip MacOS 의 환경에서 동일한 동작을 하게 만들었습니다.
  - 예를 들면, int64_t 와 uint32_t, result_type 등을 사용하여 동일한 동작을 보장합니다.
- Boost.asio 라이브러리를 사용하여 스레드의 직접적인 생성을 최소화 했습니다.
- new와 delete를 직접 사용하는 raw memory 연산을 최소화 했습니다.
- nullptr를 리턴하기보다는 빈 객체(empty vector, empty string)를 리턴하여 nullptr의 사용을 최소화 했습니다.
- std 라이브러리를 최대한 많이 사용하여 직접적인 메모리 관리를 최소화 했습니다.
<br><br>

## 개발환경 셋팅
- 각자의 개발 컴퓨터에 맞는 C++ 컴파일러, boost 라이브러리가 설치돼 있어야 합니다.
  - Windows : Visual Studio 커뮤니티 에디션을 설치한 후, vcpkg로 boost 라이브러리를 설치해 준 후 둘을 링크 해줍니다.
  - MacOS : GCC, G++, CMake를 터미널 명령어로 설치한 후, boost.asio를 homebrew로 설치해줍니다.
  - Linux ubuntu : GCC, G++, CMake, boost.asio를 터미널 명령어로 설치해줍니다.
- 현재 master 브랜치에 등록돼 있는 CMakeLists.txt는 native Windows Visual Studio, M series chip MacOS Visual Studio Code, WSL ubuntu terminal 환경에서 빌드 및 실행 테스트를 통과한 상태입니다.
- 셋 중 편한 환경을 선택한 후 각자의 환경에 따라서 아래의 안내대로 실행시키면 됩니다.
<br><br>

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
<br><br>

## MacOS(M series chip) Visual Studio Code 실행방법
1. mac에 Xcode, GCC, G++, CMake를 설치해준다. homebrew가 없다면 설치해준다. boost 라이브러리는 homebrew를 이용해서 설치해준다.
2. 본 프로젝트를 깃 클론한다.
3. 본 프로젝트 내의 CMakeLists.txt를 아래의 내용으로 통째로 교체한다.
```CMake
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(AlphaStreamer3.1Cpp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost (assumes installed via Homebrew)
find_package(Boost REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIRS} src)

# Add the executable
add_executable(AlphaStreamer3.1Cpp
    src/main.cpp
    src/util/Logger.cpp
)

# Link Boost libraries (if necessary)
target_link_libraries(AlphaStreamer3.1Cpp ${Boost_LIBRARIES})
```
4. 아래의 명령으로 build_and_run_on_Mchip_MacOs.sh 스크립트에 실행 권한을 준다.
```text
sudo chmod +x build_and_run_on_Mchip_MacOS.sh
```
5. 스크립트를 실행한다.
```text
./build_and_run_on_Mchip_MacOS.sh
```
6. 프로젝트 루트 디렉토리를 보면, .vscode 라는 폴더가 있을 것이다. 여기에서 c_cpp_properties.json 파일이 있을 것인데, 아래와 같이 "iucludePath" 부분에 boost 라이브러리가 설치된 경로인 "/opt/homebrew/opt/boost/include"를 추가해주도록 한다. 이렇게 해야, VS Code 에서 "#include <boost/asio.hpp>"를 정상적으로 인식할 수 있다.
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
<br><br>

## C++ Tips
1. 플랫폼 의존성에 대처하기.
<br>int64_t, int32_t, int16_t, int8_t, unsigned char를 사용하자.
<br> long 같은 것은 특히 플랫폼에 따라서 bit 사이즈가 다르다..
```text
//      randome engine needs seed. So, std::time(...) was used.
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
```
2. 파일에서 raw binary 데이터를 읽어들여야 할 때는 unsigned char로 읽어들인다.
    <br> reinterpret_cast도 까먹지 말자.
```c++
    // example
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
```
3. input file stream을 열 때는 std::ios::ate 모드를 사용하자.
    <br> 그래야 파일 사이즈를 바로 알 수 있다.
```c++
public void opneModes(){
    // 파일 포인터를 파일의 끝부분에 위치시킨다.
    std::ifstream file(inputFilePath, std::ios::binary | std::ios::ate);
    
    // 이렇게 파일 사이즈를 바로 알 수 있다.
	std::streamsize size = file.tellg();
	// 파일 사이즈를 알아낸 후에는 파일포인터를 시작지점으로 다시 옮겨 놓는다.
	file.seekg(0, std::ios::beg);
}
```
4. 일부 유틸리티는 직접 구현해야 한다.
   <br>java의 string.split(), Files.readAllBytes(...), RandomAccessFile.length() 등은 C++에는 없다..
```c++
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
```
5. java의 File 객체는 C++에서는 std::filesystem::path로 대체할 수 있다.
   <br>
```c++
int main() {
  const std::string path = "C:\\Users\\user\\CLionProjects\\CppTestPad\\test.txt";
  const std::string dirPath = "C:\\Users\\user\\CLionProjects\\CppTestPad";

  std::filesystem::path p(path);

  std::filesystem::path pCopy(dirPath);

  // std::filesystem::path 타입은 카피 가능하다!!

  for (std::filesystem::path dir : std::filesystem::directory_iterator(pCopy)) {
    if (is_directory(dir)) {
      std::cout << dir.filename() << "\n";
    } else {
      if (dir.filename().string().find("ma") != std::string::npos) {
        std::cout << "Found main !! : " << dir.filename() << "\n";
      }
    }
  }
}// end of main
```
6. java의 string.contains(...)는 string.find(...) != std::string::npos 로 대체할 수 있다.
   <br>
```c++
bool FileReader::handleConfigFile(const std::filesystem::path &inputCidDirectory) {
  for (std::filesystem::path dir : std::filesystem::directory_iterator(inputCidDirectory)) {
    if (!is_directory(dir) && dir.filename().string().find("acc") != std::string::npos) {
      configFile = dir;
      return true;
    }
  }
  return false;
}
```
7. 상수 또는 유틸리티 아카이브 역할을 하는 클래스는 다음의 사항들에 유의하고, std_shared_ptr<>을 쓰자.
   <br> 본 프로젝트의 Logger.h & .cpp, C.h, Util.h 클래스가 유틸리티 역할을 한다.
```text
/*
    When writing a class to be used globally, such as a Logger or a constant value archive,
    the keywords static inline constexpr const can be helpful.

    static: Ensures the variable belongs to the class itself, not any instance.
    inline (C++17 or later): Allows the variable to appear in multiple translation units
    without violating the One Definition Rule (ODR).
    
    What is the ODR?
    The ODR (One Definition Rule) governs the use and definition of variables, functions,
    and types in C++. It consists of two main principles:
    A definition must appear only once across all translation units.
    Declarations can appear multiple times, but only one definition is allowed.
    
    What is a Translation Unit?
    A translation unit in C++ is the basic unit of compilation.
    It consists of a .cpp file along with all its included headers (via #include)
    and preprocessed code.

    Why Define Constants in Header Files?
    Defining constants in .cpp files instead of .h files can cause linker errors, such as:
    "Logger::Logger: cannot access private member."

    This error occurs because when the constants are defined in a .cpp file, the linker
    must resolve their references across the entire project.
    If only Logger.h is included in a translation unit (e.g., in main.cpp), the linker
    cannot find the definitions in Logger.cpp, causing the error.

    Solution:
    Always define global constants in the .h file like below!
    This ensures the constants are accessible across multiple translation
    units without violating the ODR.
    */
```
8. 기본 생성자가 아닌 다른 생성자에는 explicit 키워드를 쓰면 안 된다.
   <br> 해당 키워드를 쓰면 생성자가 작동을 하지 않는다..
```c++
class RtpInfo {
public:
	using KeyValueMap = std::unordered_map<std::string, std::vector<int64_t>>;

	explicit RtpInfo();
	~RtpInfo();

	// Do not use 'explicit' keyword in Copy Constructor!!
	RtpInfo(const RtpInfo& other);

	RtpInfo& operator=(const RtpInfo& other);

	const std::shared_ptr<RtpInfo> clone();
	const std::string toString();

	KeyValueMap kv;
	std::vector<std::string> urls;

private:
	void copyFrom(const RtpInfo& other);
};

#endif // RTPINFO_H
```
9. java의 synchronized 키워드는 std::mutex 락으로 대체할 수 있다.
   <br>
```c++
RtpInfo FileReader::getRtpInfoCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  // call Copy Constructor
  RtpInfo rtpInfoCopy(rtpInfo);
  return rtpInfoCopy;
}
```
10. 함수 시그니처 작성에는 다음과 같은 방법들이 있다.
   <br> 크게는 리턴 타입이 '참조'인지, 아니면 그냥 '값'인지로 나눌 수 있다.
```text
    /*
    function signature tip;
    
    > when returning '&'.
        permit modification:
            std::vector<Buffer>& getAllRtps();
        forbidden modification:
            const std::vector<Buffer>& getAllRtps();
        forbidden modification and can be called only by const object
            const std::vector<Buffer>& getAllRtps() const;

    > when return value.
        int size() const;
    */
```
11. 반복자는 copy, sort 등에 유용하게 사용된다.
   <br> 추가 사례들이 발생하면 여기에 기록하자.
```c++
// return copied vector
std::vector<AudioSampleInfo> FileReader::getAudioMetaCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  return std::vector<AudioSampleInfo>(
    audioFile.getConstMeta().begin(), audioFile.getConstMeta().end()
  );
}

// custom sort
bool FileReader::handleCamDirectories(const std::filesystem::path &inputCidDirectory) {
  std::vector<std::filesystem::path> camDirectoryList;
  for (std::filesystem::path camDirectory : std::filesystem::directory_iterator(inputCidDirectory)) {
    camDirectoryList.push_back(camDirectory);
  }
  // sort ascending order of filenames
  std::sort(
    camDirectoryList.begin(), camDirectoryList.end()
    , [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
      return lhs.filename() < rhs.filename();
    }
  );
// ... do something
}
```
12. boost.asio의 io_context를 필요로 하는 클래스들한테는 io_context의 참조를 전달한다.
   <br> 근원 io_context를 괜히 다른 클래스에서 만들지 말고, main에서 만들자.
   <br> make_work_guard를 쓰면 async task가 전혀 없을 때 io_context가 스스로 종료하는 행위를 막을 수 있다.
```c++
int main(){
    boost::asio::io_context io_context;
    auto workGuard = boost::asio::make_work_guard(io_context);
    std::vector<std::thread> threadVec;
    unsigned int threadCnt = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCnt; ++i) {
        threadVec.emplace_back(
            [&io_context]() {io_context.run(); }
        );
    }

    SntpRefTimeProvider sntpRefTimeProvider(io_context);
    sntpRefTimeProvider.start();
}
```