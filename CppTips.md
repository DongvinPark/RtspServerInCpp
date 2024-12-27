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
   <br> 해당 키워드를 쓰면 기본 생성자 이외의 다른 생성자(복사, 이동)가 작동을 하지 않는다..
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
<br><br/>
13. std::ifstream은 복사가 불가능하다.
    <br> 그러나, 참조를 이용해서 함수 파라미터로 전달하는 것은 가능하다.
    <br> map에 어떤 객체를 등록할 때는 copy 가 기본 동작이다. std::ifstream은 이게 불가능하기 때문에, std::ifstream을 내부 멤버로 가지고 있는 객체를 map에 등록할 때는 별도의 이동 생성자와 이동 연산자 오버로딩을 정의해야 한다.
```c++
// VideoAccess 클래스의 이동 생성자와 이동 연산자 오버로딩.

// move constructor
VideoAccess::VideoAccess(VideoAccess&& other) noexcept
    : accesses(std::move(other.accesses)), meta(std::move(other.meta)) {
    // delete unnecessary resources
    other.accesses.clear();
    other.meta.clear();
}

// move assignment operator
VideoAccess& VideoAccess::operator=(VideoAccess&& other) noexcept {
    if (this != &other) { // ban self-assignment
        close(); // delete cur resource

        // transfer resource
        accesses = std::move(other.accesses);
        meta = std::move(other.meta);

        // delete unnecessary resources
        other.accesses.clear();
        other.meta.clear();
    }
    return *this;
}
```
<br><br/>
14. 컨테이너에서 내용물을 꺼내서 수정하고 싶으면, '&'타입으로 꺼내라.
    <br> 그렇지 않으면, 꺼내면서 copy가 돼서 원본 객체 수정도 안 되고, 성능도 나빠질 수 있다.
```c++
void FileReader::loadRtpMemberVideoMetaData(
    int64_t videoFileSize,
    std::ifstream &inputIfstream,
    std::vector<std::vector<VideoSampleInfo>>& input2dMetaList,
    int memberId
) {
  input2dMetaList.push_back(std::vector<VideoSampleInfo>{});

  // ...

  for (const int16_t size : sizes) { // size must start with -1, refer to acs_maker.
    if (size == C::INVALID) {
      VideoSampleInfo newVSampleInfo{};
      newVSampleInfo.setOffset(offset);
      newVSampleInfo.setFlag( (sampleCount % gop) == 0 ? C::KEY_FRAME_FLAG : C::P_FRAME_FLAG );
      input2dMetaList.at(input2dMetaList.size() - 1).push_back(newVSampleInfo);
      sampleCount++;
      continue;
    }

    // 여기서 & 를 붙이지 않고 그냥 .at(...)으로 꺼내버리면
    // 그 많은 요소들이 전부 복사되는 수가 있다!!!
    std::vector<VideoSampleInfo>& sampleInfoList = input2dMetaList.at(input2dMetaList.size() - 1);
    VideoSampleInfo& latestVideoSampleInfo = sampleInfoList.at(sampleInfoList.size() - 1);
    
    // 수정을 위해서 사용되는 .getMetaInfoList() 함수도 당연히 '&'타입으로 리턴해야 한다!!
    latestVideoSampleInfo.getMetaInfoList().push_back(RtpMetaInfo(size, offset));
    int prevSize = latestVideoSampleInfo.getSize();
    latestVideoSampleInfo.setSize(prevSize + size);

    offset += size;
  }// for

  showVideoMinMaxSize(input2dMetaList.at(input2dMetaList.size() - 1), memberId);
}
```
<br><br/>
15. java의 ByteBuffer 클래스 내의 asShortBuffer()는 보통은 big endian으로 작동한다. 
    <br> 그렇기 때문에 C++에서는 이것을 수동으로 구현해야 한다.
    <br> big endian 은 0x1234를 address 1,2에 0x12, 0x34 순서로 담는 것이고,
    <br> littel endian은 address 1,2에 0x34, 0x12 순서로 담는 것이다.
    <br> 즉, big endian은 MSB 가 먼저, little endian은 LSB 먼저인 것이다.
```java
// java version
private short[] getSizes(byte[] metaData){
        short[] sizes = new short[metaData.length/2];
        ByteBuffer.wrap(metaData).asShortBuffer().get(sizes);
        return sizes;
    }
```
```c++
// C++ version
std::vector<int16_t> FileReader::getSizes(std::vector<unsigned char>& metaData) {
  std::vector<int16_t> sizes;
  sizes.reserve(metaData.size() / 2); // Reserve memory for efficiency

  for (size_t i = 0; i < metaData.size(); i += 2) {
    // Combine into a 16-bit value (big-endian)
    int16_t value = (static_cast<int16_t>(metaData[i]) << 8) | static_cast<int16_t>(metaData[i + 1]);
    sizes.push_back(value);
  }

  return sizes;
}
```
<br><br/>
16. std::pmr:: ... 류의 자료구조는 웬만해서는 쓰지 마라.
    <br> 이걸 썼을 경우, 쓰지 않았을 때와 비교해서 컴파일 요건이 까다로워지기 때문에 쓸데없이 시간을 낭비하게될 가능성이 크다.
    <br> ... prm 을 쓴 경우와 쓰지 않은 경우의 차이점은 memory allocation이 어떻게 컨트롤 되는가이다.
    <br> 전자의 경우(pmr사용), C++ 17에서 도입된 Polymorphic Memory Resource 라이브러리에 정의 된 std::pmr::polymorphic_allocator를 사용하게 되고, 후자의 경우 default allocator인 std::allocator를 사용한다는 차이점이 있다.
    <br> 전자의 경우는 정말 특별한 경우(성능이 너무나 중요하거나, 자원이 너무나 한정적이어서)에만 사용하고, 대부분의 경우엔 pmr을 쓰지 않는다.
    <br> pmr을 쓴 후 컴파일 할 경우, error: invalid use of incomplete type ... 같은 에러 메시지를 마주할 가능성이 크다.  
```c++
class ContentsStorage {
public:
  explicit ContentsStorage(const std::string contentStorage);
  ~ContentsStorage();

  ContentsStorage& init();

  FileReader& getCid(std::string cid);
  std::unordered_map<std::string, FileReader>& getReaders();
  void shutdown();
  std::string getContentRootPath() const;

private:
  std::shared_ptr<Logger> logger;
  std::filesystem::path parent;
  // 여기서 그냥 unordered_map을 쓰면 별 문제 없이 컴파일 되던 것이
  // pmr 을 쓰게될 경우, Linux환경에서는 컴파일이 거부 된다.
  //std::pmr::unordered_map<std::string, FileReader> readers;
  std::unordered_map<std::string, FileReader> readers;
  std::string contentRootPath;
};
```
<br><br/>
17. 함수 내부에서 새롭게 만든 객체를 외부로 반출시키는 방법은 웬만해서는 쓰지 마라.
    <br> java에서는 이러한 경우가 아주 많지만, C++에서는 그렇지 않다.
    <br> C++에서는 함수 범위를 벗어난 객체는 자동으로 회수 처리 되기 때문이다.
    <br> 대신 아래와 같은 방법을 사용하라.
```c++
// 값을 반환하라. 대신 복사는 피할 수 없다.
int getLocalValue() {
    int localVar = 42;
    return localVar; // The value is safely copied or moved.
}

int main() {
    int value = getLocalValue();
    std::cout << value << std::endl; // Output: 42
}

// Dynamic Memory Allocation : raw 포인터 반환하라.
int* getHeapValue() {
    int* heapVar = new int(42); // Allocate on the heap.
    return heapVar;            // Return a pointer.
}

int main() {
    int* ptr = getHeapValue();
    std::cout << *ptr << std::endl; // Output: 42
    delete ptr;                     // Don't forget to free the memory!
}


// 스마트 포인터 반환하라. unique OR shared
#include <iostream>
#include <memory> // For std::unique_ptr

std::unique_ptr<int> getHeapValue() {
    return std::make_unique<int>(42); // Allocate on the heap and return a unique_ptr.
}

int main() {
    auto ptr = getHeapValue(); // `ptr` now owns the heap memory.
    std::cout << *ptr << std::endl; // Output: 42
    // No need to manually delete; `std::unique_ptr` takes care of it.
    return 0;
}

#include <iostream>
#include <memory> // For std::shared_ptr

std::shared_ptr<int> getHeapValue() {
    return std::make_shared<int>(42); // Allocate on the heap and return a shared_ptr.
}

int main() {
    auto ptr = getHeapValue(); // `ptr` now shares ownership of the heap memory.
    std::cout << *ptr << std::endl; // Output: 42
    // Memory will be freed when the last shared_ptr owning it is destroyed.
    return 0;
}


// 외부 객체를 참조 타임 파라미터로 전달한 후, 함수 내부에서 수정하라.
void modifyValue(int& value) {
    value = 42; // Modify the object.
}

int main() {
    int myVar = 0;
    modifyValue(myVar);
    std::cout << myVar << std::endl; // Output: 42
}

// static local 변수로 전달하라. 단, 위의 방법들이 통하지 않을 때만!!
int& getStaticReference() {
    static int staticVar = 42; // This object persists for the lifetime of the program.
    return staticVar;
}

int main() {
    int& ref = getStaticReference();
    std::cout << ref << std::endl; // Output: 42
    ref = 100;
    std::cout << getStaticReference() << std::endl; // Output: 100
}
```
<br><br/>
18. 복사도, 이동도 불가능한 객체를 멤버로 가지고 있는 클래스는 어떻게 다루어야 하는가?
    <br> boost.asio의 io_context 객체가 대표적으로 복사도, 이동도 되지 않는다.
    <br> 본 프로젝트의 Session 객체가 바로 이러한 특징을 가지고 있는 객체다.
    <br> std::ifstream이 복사는 되지 않는 대신, 참조로서 이동은 가능한 것과 비교 된다.
    <br> 복사 또는 이동 생성자를 정의할 수 없기 때문에, map.emplace(key, val); 이런 것이 통하지 않는다.
    <br> 이럴 때는 하는 수 없이, 포인터를 저장하는 것 말고는 뾰족한 방법이 없다.
```c++
// Session 객체에 대한 포인터를 맵의 value로서 가지고 있는 Server 객체 정의하였다.
class Server {
public:
  explicit Server(
    boost::asio::io_context& inputIoContext,
    ContentsStorage& inputContentsStorage,
    const std::string &inputStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider
  );
  ~Server();

  // ... several member functions ...

private:
  // ... several member fileds ...
  
  // 주목해야 하는 부분은 여기다. Session 객체를 map.emplace(); 같은 함수를 써서
  // 생성하는 것은 이동생성자가 정의돼 있어야 가능하다.
  // 그러나, Session 객체 내부의 boost::asio::io_context io_context; 멤버가
  // 복사와 이동이 모두 금지된 객체이기 때문에 Session 객체 또한 복사외 이동이 어렵다.
  // 이럴 때는 Session 객체의 포인터를 map 등의 컨테이너에 저장하여 복사외 이동 없이도
  // container를 통한 접근 및 관리가 가능하게 만들 수 있다.
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
  
  // ... several member fileds ...
};
```
<br><br/>
19. java의 HashMap<K,V>는 C++의 std::unordered_map<K,V>로 대체할 수 있다.
    <br> std::vector 와 함께 매우 자주 사용되는 객체이므로 주요 멤버 함수의 특징을 간략하게 정리하였다.
```c++
// 요약하면 아래와 같다.
//Summary of Behaviors
//myMap[key] = value	             >> Copies value into the map..
//myMap.insert({key, value})	     >> Copies value into the map.
//myMap[key] = std::move(value)	     >> Moves value into the map only when move constructor was difined
//myMap.emplace(key, value)	     >> Constructs value in place (no copies) only when move constructor was defined

#include <iostream>
#include <unordered_map>
#include <string>

// 아래는 std::unordered_map을 사용하는 예시 프로그램이다.
int main() {
    std::unordered_map<int, std::string> myMap;

    // Insert elements
    // 맵에 key value 쌍을 저장할 때의 기본 동작은 copy이다.
    myMap[1] = "One";
    myMap[2] = "Two";
    myMap.insert({3, "Three"});

    // Access elements
    // 맵에서 key value pair를 가져올 때도 별도의 참조(&) 타입으로 가져오지 않으면
    // copy 동작으로 키값 쌍을 가져오게 된다.
    std::cout << "Key 1: " << myMap[1] << '\n';
    std::cout << "Key 3: " << myMap.at(3) << '\n';

    // Check if a key exists
    if (myMap.count(2)) {
        std::cout << "Key 2 exists.\n";
    }

    // Iterate over the map
    // 이렇게 & 타입을 이용할 경우, 복사가 아니라 참조로써 맵 내 요소들에 접근할 수 있다.
    for (const auto& pair : myMap) {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << '\n';
    }

    // Erase an element
    myMap.erase(2);

    // Print size
    std::cout << "Size: " << myMap.size() << '\n';

    return 0;
}

//------------- 복사 없이 맵에 요소 등록하기.

// 맵에 사용자 타입 객체(사용자가 정의한 클래스)를 복사 없이 담고 싶다면 두 가지 방법이 있다.

// 첫째는 객체에 이동 생성자를 정의해 둔 후, std::move(value_);로 키값 쌍을 등록하는 것이다.
// 단 이 방법이 통하려면, 공통적으로 '이동 생성자'가 정확하게 정의되어야 한다!!
#include <unordered_map>
#include <string>
#include <iostream>

struct Data {
    std::string value;
    Data(const std::string& val) : value(val) {}
};

int main() {
    std::unordered_map<int, Data> myMap;

    Data d("example");
    myMap[1] = std::move(d);  // The value `d` is moved into the map.

    std::cout << myMap[1].value << '\n';  // Output: example
    std::cout << d.value << '\n';         // Output: (empty string, as `d` has been moved)
}

// 둘째는 map.emplace()를 이용하는 것이다.
// 하지만 이 경우에도 '이동 생성지'가 정의돼 있지 않을 경우, emplace()는 내부적으로
// 임시 객체를 만든 후 그 객체를 맵 안으로 복사하는 방식으로 작동하기 때문에, 제대로된
// non-copy 삽입이 되게 하려면 결국 이동 생성자를 정의해야 한다.
// 19번 팁과 같이 이동 생성자를 정의할 수 없는 경우에는 하는 수 없이
// 값 타입의 포인터를 맵에 저장하는 방식으로 하는 것 외에는 다른 방법이 별로 없다.
#include <unordered_map>
#include <string>
#include <iostream>

struct Data {
    std::string value;
    Data(const std::string& val) : value(val) {}
};

int main() {
    std::unordered_map<int, Data> myMap;

    myMap.emplace(1, "example");  // Directly constructs the value in place.

    std::cout << myMap[1].value << '\n';  // Output: example
}
```
<br><br/>
20. C++에서도 Circular Referencing 또는 Circular Dependencies 문제가 발생한다.
    <br> 본 프로젝트에서는 이러한 문제가 Session과 Server 객체를 정의하면서 발생하였다.
    <br> Session 객체는 생성자에서 Server 객체의 참조를 인자로 받고, Server 객체는 멤버 std::unordered_map에서 value 타입으로 Session 객체의 포인터를 받아들이게 정의 돼 있기 때문이다.
    <br> 이 문제는 forward declaration 으로 해결할 수 있다.
    <br> 단, 이것은 Session과 Server가 서로의 참조 또는 포인터만을 멤버로 가지고 있기 때문에 가능한 방법이며, 참조나 포인터가 아닌 경우에 어떤 동작을 하게 될지는 아직 테스트 하지 않았다.
```c++
// Server.h 다. Session 객체를 먼저 선언해줌으로써 순환 참조 문제를 방지한다.
class Session;
class ContentsStorage;
class SntpRefTimeProvider;

class Server {
public:
  explicit Server(
    boost::asio::io_context& inputIoContext,
    ContentsStorage& inputContentsStorage,
    const std::string &inputStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider
  );
  ~Server();

  // ... public member functions ...

private:
  // ... private members ...
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
};

//----------------------------------

// Session.h 다. 여기에서도 Server 클래스를 먼저 선언해줌으로써 순환 참조를 방지한다.
class Server;
class ContentsStorage;
class SntpRefTimeProvider;

class Session {
public:
  explicit Session(
    boost::asio::io_context& inputIoContext,
    boost::asio::ip::tcp::socket& inputSocket,
    std::string inputSessionId,
    Server& inputServer,
    ContentsStorage& inputContentsStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider
  );
  ~Session();

  // ... public member functions ...

private:
  // ... private members ...
  Server& parentServer;
};

#endif //SESSION_H

```





















