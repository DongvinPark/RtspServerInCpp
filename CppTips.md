# C++ Tips
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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
<br><br/>
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