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
7. 상수 또는 유틸리티 아카이브 역할을 하는 클래스는 다음의 사항유의 하자.
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
   <br> 그러나, lock을 잠그고 해제하는 연산은 상대적으로 비싼 연산이기 때문에, 꼭 필요할 때만 써야한다.
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
        리턴 받은 참조를 caller 쪽에서 이용해서 수정하는 것을 허용할 때.
            std::vector<Buffer>& getAllRtps();
            
        리턴 받은 참조를 caller 쪽에서 이용해서 수정하는 것을 금지할 때.
            const std::vector<Buffer>& getAllRtps();
            
        리턴 받은 참조를 Caller 쪽에서 이용해서 수정하는 것을 금지할 뿐만 아니라, const 타입 객체일 경우
        아래의 시그니처로 선언된 함수만 호출할 수 있게 제한함.
            const std::vector<Buffer>& getAllRtps() const;

    > when return value.
        int size() const;
    */
    
// 위의 case 들 중에서, 참조 타입을 리턴하는 경우 중 3 번째 상황은 아래의 예시로 설명할 수 있다.
// const 타입으로 선언된 FileReader 객체는 f1() const; 와 같은 시그니처를 가진 멤버 함수만 호출할 수 있다. 
class FileReader {
public:
    void f1() const { /* does not modify object */ }
    void f2() const { /* does not modify object */ }
    void f3() { /* modifies object */ }  // const 객체에서는 이 함수를 호출할 수가 없다!!
};

int main() {
    const FileReader reader;  // Const object
    reader.f1();  // OK
    reader.f2();  // OK
    // reader.f3();  // Error: f3 is not const-qualified
}
    
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
14. 컨테이너에서 내용물을 꺼내서 수정하고 싶으면, '&'타입으로 꺼내자.
    <br> 그렇지 않으면, 꺼낼 때 copy가 돼서 원본 객체 수정도 안 되고, 성능도 나빠질 수 있다.
```c++
void FileReader::loadRtpMemberVideoMetaData(
    int64_t videoFileSize,
    std::ifstream &inputIfstream,
    std::vector<std::vector<VideoSampleInfo>>& input2dMetaList,
    int memberId
) {
  input2dMetaList.push_back(std::vector<VideoSampleInfo>{});

  // ...

  for (const int16_t size : sizes) { // size must start with -1, refer to stream_maker.
    if (size == C::INVALID) {
      VideoSampleInfo newVSampleInfo{};
      newVSampleInfo.setOffset(offset);
      newVSampleInfo.setFlag( (sampleCount % gop) == 0 ? C::KEY_FRAME_FLAG : C::P_FRAME_FLAG );
      input2dMetaList.at(input2dMetaList.size() - 1).push_back(newVSampleInfo);
      sampleCount++;
      continue;
    }

    // 여기서 & 를 붙이지 않고 그냥 std::vector<VideoSampleInfo> 타입으로 꺼내버리면
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
16. std::pmr:: ... 류의 자료구조는 웬만해서는 쓰지 말자.
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
17. 함수 내부에서 새롭게 만든 객체를 외부로 반출시키는 방법은 웬만해서는 쓰지 말자.
    <br> java에서는 이러한 경우가 일반적이지만, C++에서는 그렇지 않다.
    <br> C++에서는 함수 범위를 벗어난 객체는 자동으로 삭제 되기 되기 때문이다.
    <br> 대신 아래와 같은 방법을 사용하라.
```c++
// 값을 반환한다. 대신 복사는 피할 수 없다.
int getLocalValue() {
    int localVar = 42;
    return localVar; // The value is safely copied or moved.
}

int main() {
    int value = getLocalValue();
    std::cout << value << std::endl; // Output: 42
}

// Dynamic Memory Allocation : raw 포인터를 반환한다. 대신 메모리 누수에 취약하다.
int* getHeapValue() {
    int* heapVar = new int(42); // Allocate on the heap.
    return heapVar;            // Return a pointer.
}

int main() {
    int* ptr = getHeapValue();
    std::cout << *ptr << std::endl; // Output: 42
    delete ptr;                     // Don't forget to free the memory!
}


// 스마트 포인터를 반환하라. std::make_unique OR std::make_shared
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
    <br> 이럴 때는 하는 수 없이, 세션에 대한 포인터를 저장하는 것 말고는 뾰족한 방법이 없다.
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
//myMap[key] = value	             >> Copies value into the map. and updates the value by new one.
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
<br><br/>
21. Graceful shutting down logic 구현이 필수적이다.
    <br> 본 프로젝트는 RTSP 스트리밍 서버이기 때문에 소켓, 파일 핸들러, 스레드 풀, boost.asio io_context 등의 자원을 서버 종료시 모두 회수해야 한다.
    <br> 서버 종료 시 자원 회수 로직은 main.cpp 에 구현돼 있으며, Future, Boost.Asio signal handling, try-catch exception handling 이 사용되었다.
    <br> 자세한 코드와 구현 시 고려한 사항은 main.cpp를 참고한다.
    <br> 특정 상황에서 반드시 실행돼야 하는 로직을 구현할 때는, 중간의 blocking 함수가 예외를 던지는지 유심히 관찰하자. 
```c++
// 아래는 main.cpp의 일부이다.
// 여기서 특히 주의해야 할 점은, 셧다운 로직을 실행시키기 이전에 어딘가 blocking 함수에서 예외를 던지며
// main.cpp가 종료돼 버릴 경우, 이어지는 셧다운 로직이 전부 실행되지 못한 채 프로그램이 종료 돼 버린다는 점이다.
using boost::asio::ip::tcp;

int main() {
    // ...
    // 여기서 boost.asio io_context를 여러 개의 스레드에서 run(); 해주는 워커 스레드 풀을 만든다.
    logger->warning("Made io_cotext.run() worker thread pool with thread cnt: " + std::to_string(threadCnt));

    // main.cpp가 사용자의 종료 명령(컨트롤 + C 키 누름) 종료 되면 안되므로,
    // exit 핸들러 역할을 하는 boost asio signal set이 종료되는 것을 알려주는 콜백을 만들어준다.
    std::promise<void> shutdownPromise;
    auto shutdownFuture = shutdownPromise.get_future();

    // 사용자가 종료 명령을 내리면, 여기에서 io_context를 바로 정지 시킨다.
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code& ec, int signal) {
        try {
            if (!ec) {
                std::cout << "\n\t>>> Received signal: " << signal << ". Stopping server...\n";
                workGuard.reset();
                if (!io_context.stopped()) {
                    io_context.stop();
                    std::cout << "\t>>> io_context stopped.\n";
                }

                // io_context 정지 후, 콜백에 값을 전달한다.
                shutdownPromise.set_value();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during signal handling: " << e.what() << std::endl;
            shutdownPromise.set_exception(std::make_exception_ptr(e));
        }
    });

    // ... init several componants ...

    Server server(io_context, contentsStorage, contentsRootPath, sntpRefTimeProvider);
    // Server 클래스 내부의 start() 부분에서 try-catch 등의 방법으로 예외를 잡지 않을 경우,
    // main.cpp가 바로 종료돼 버리면서 이어지는 셧다운 로직을 실행할 수 없게 된다.
    // 예외를 여기에서 바로 잡든지, 아니면 server.start(); 함수 내부에서 잡든지 해야 한다.
    server.start();

    // 워커 스레드 풀 내의 스레드들이 전부 io_context.run();을 종료할 때까지 기다린다.
    shutdownFuture.wait();

    // 워커 스레드 풀 내의 모든 스레드들을 모두 join 시켜서 회수한다.
    // 다른 셧다운 로직이 추가될 수 있다.
    for (auto& thread : threadVec) {
        if (thread.joinable()) {
            std::cout << "Joining io_context.run() worker thread " << thread.get_id() << "...\n";
            thread.join();
        } else {
            std::cout << "Cannot join thread : " << thread.get_id() << "\n";
        }
    }

    logger->warning("=================================================================");
    logger->warning("Dongvin, C++ RTSP Server SHUTS DOWN gracefully.");
    logger->warning("=================================================================");
    return 0;
}
```
<br><br/>
22. 클래스의 멤버 간 Circular Dependencies 문제가 복잡하게 얽혀 있을 때는 클래스 멤버들을 전부 pointer로 만드는 게 가장 간단한 해결책이다.  
    <br> 스마트 포인터를 만들 때는 full definition을 필요로 하지 않기 때문이다.
    <br> 그러나, 여기에도 주의사항이 있다.
    <br> 포인터를 이용해서 서로 참조할 때, 특히 서로 다른 클래스 두 개가 std::shared_ptr 로 서로 참조하고 있을 때는 포인터를 썼음에도 불구하고 circular dependencies 문제가 똑같이 발생하면서 reference count가 절대 0으로 떨어지지 않기 때문에 결과적으로 memory leak이 발생한다.
    <br> 이 때는 한쪽을 std::weak_ptr 로 만들어서 이러한 연결고리를 끊어야 한다.
    <br> 그리고 포인터를 이용해서 참조 관계를 설정할 때는, .h 파일에 대응하는 .cpp 파일도 만들어서 구현을 마무리 지어 놔야 한다. 그렇지 않을 경우, 'undefined reference to ...'라는 컴파일 타임 에러 메시지를 마주치게 된다.
    <br> 마지막으로, 포인터 멤버를 초기화 할 때는 생성자 내부에서 하기보다는 다른 곳에서 하는 것을 추천한다. 직접 해보니 this 포인터를 가지고 std::make_shared를 호출하는 것이 허용되지 않았기 때문이다.
```c++
// 본 프로젝트의 Session, StreamHandler, RtspHandler, RtpHandler가 이러한 관계에 있다.
// Session은 StreamHandler, RtspHandler, RtpHandler를 멤버 클래스로 두고 있고,
// StreamHandler, RtspHandler, RtpHandler는 Session의 포인터를 가지고 Session 내의 함수들을 호출해야 한다.
// 결과적으로 멤버 클래스가 부모 클래스를 참조해야 하는 상황인데, 이때는 멤버 클래스가 부모 포인터를 std::weak_ptr로서 참조해야 한다.
// std::weak_ptr는 소유권이 없기 때문에 reference count를 증가시키지 않고, 결과적으로 memory leak을 예방할 수 있다.
// std::weak_ptr는 circular dependencies를 끊고, memory leack를 예방하기 위한 용도로 사용한다.

// 구체적인 코드 구현은 Server.cpp의 start() 함수 내부, StreamHandler의 생성자, RtspHandler의 생성자, RtpHandler의 생성자를 참고한다.

// 일반적으로 상위 클래스는 멤버 클래스들에 대한 std::sharead_ptr를 가지고 있고, 멤버 클래스들은 상위 클래스에 대한 std::weak_ptr를 가지고 있게 구현한다. 

// std::weak_ptr는 보통 Observer 패턴, 캐싱 관리, 순환 참조 해제 등에 사용된다.

// ------- 아래는 std::weak_ptr의 간단한 사용 예시다. -------
// expired() 함수와, lock() 함수를 써서 std::weak_ptr에 대한 안전한 접근을 한다.
#include <iostream>
#include <memory>

int main() {
    std::shared_ptr<int> shared = std::make_shared<int>(10);
    std::weak_ptr<int> weak = shared; // Non-owning reference

    if (auto locked = weak.lock()) { // Check if the object still exists
        std::cout << "Value: " << *locked << "\n";
    } else {
        std::cout << "The object no longer exists.\n";
    }

    shared.reset(); // Destroy the managed object

    if (auto locked = weak.lock()) {
        std::cout << "Value: " << *locked << "\n";
    } else {
        std::cout << "The object no longer exists.\n";
    }

    return 0;
}
```
<br><br/>
23. std::unique_ptr 은 복사가 불가능하다.
    <br> 왜냐하면, 말 그대로 단 한 곳에서만 해당 포인터를 소유할 수 있기 때문이다.
    <br> 따라서, std::unique_ptr를 함수 인자로 주고 받기 위해서는 std::move()를 써야 한다.
```c++
// 예를 들면 이런 식이다.
auto rxTask = [&](){
    try {
      while (true) {
        std::unique_ptr<Buffer> bufferPtr = receive(socket);
        handleRtspRequest(std::move(bufferPtr));
      }
    } catch (const std::exception & e) {
      // ...
    }
  };
```
<br><br/>
24. java의 Math.min(), max(), abs(), round() 등의 유틸은 C++에서는 아래와 같이 사용한다.
    <br> java의 Math.round()는 반올림이다. 
    <br> C++에서는 java보다 더 많은 수학 함수를 제공하지만, 함수별로 include 해야 하는 헤더가 다를 수 있다.
```c++
// Min, Max 찾기
#include <algorithm>
#include <iostream>

int main() {
    int a = 10, b = 20;
    std::cout << "Min: " << std::min(a, b) << "\n"; // Outputs 10
    std::cout << "Max: " << std::max(a, b) << "\n"; // Outputs 20
    return 0;
}

// Abs, Round 하기
#include <cmath>
#include <iostream>

int main() {
    double x = -5.67;
    std::cout << "Absolute: " << std::abs(x) << "\n"; // Outputs 5.67
    std::cout << "Round: " << std::round(x) << "\n";  // Outputs -6
    return 0;
}


// 제곱, 제곱근 구하기
#include <cmath>
#include <iostream>

int main() {
    double base = 3, exponent = 2;
    std::cout << "Power: " << std::pow(base, exponent) << "\n"; // Outputs 9
    std::cout << "Square root: " << std::sqrt(9) << "\n";       // Outputs 3
    return 0;
}
```
<br><br/>
25. std::vector 에서 요소 포함 여부 알아내는 법
    <br> java에서는 list.contains(...)를 쓰면 되지만 C++ std::vector에서는 이런 멤버 함수가 없다. 
    <br> 대신, std::find(...) 또는 std::ranges::find(...)를 쓴다.
```c++
#include <iostream>
#include <vector>
#include <algorithm> // For std::find

int main() {
    std::vector<int> ints = {1, 2, 3, 4, 5};

    if (std::find(ints.begin(), ints.end(), 1) != ints.end()) {
        std::cout << "The vector contains 1." << std::endl;
    } else {
        std::cout << "The vector does not contain 1." << std::endl;
    }

    return 0;
}
```
<br><br/>
26. ../RtspHandler.cpp:192: undefined reference to `RtpHandler::stopVideo()' 라는 에러 메시지
    <br> 아래의 코드에서 발견된 에러다.
    <br> 포인터를 통해서 접근한 객체 내부의 멤버 함수를 호출하려고 하는데, 해당 멤버 함수가 .h 파일에서는 선언 돼 있지만, .cpp 파일에서는 구현돼 있지 않은 상황에서 발생한다.
```c++
// ...
else if (method == "PAUSE") {
        // dongvin, if pause req come at puase state, just return 200 OK response.
        if (sessionPtr->getPauseStatus()) {
          respondPause(inputBuffer);
        } else {
          sessionPtr->updatePauseStatus(true);
          respondPause(inputBuffer);

          // RtpPahndler.cpp 내부에 stopVideo() 또는 stopAudio() 라는 함수가 구현돼 있지 않은 경우에 위의 에러가 발생한다.
          sessionPtr->getRtpHandlerPtr()->stopVideo();
          sessionPtr->getRtpHandlerPtr()->stopAudio();
        }
      }
// ...
```
<br><br/>
27. 특정한 함수를 async-nonblocking 하게 실행시키는 방법
    <br> java에는 CompletableFuture 등의 방법으로 특정 Runnable을 별도의 스레드에서 async하게 실행시킬 수 있다.
    <br> C++에서도 std library를 이용해서 이와 같은 기능을 직접 만들어서 쓸 수 있고, 3 가지 방법이 존재한다.
    <br> 한 가지 반드시 기억해야할 점은, lambda 에게 포인터 변수를 전달할 때는 '값'으로 캡쳐해서 전달하는게 안전하다는 점이다. 괜히 '참조'로 캡쳐되게 했다가 dangling pointer가 되면서 SIGSEGV 메시지가 뜨면서 서버가 바로 죽을 수도 있다.
    <br> 그러나 Boost.Asio를 쓰고 있다면, io_context를 이용해서 구현한 버전을 이용하자. std::thread를 이용한 버전은 io_context에 접근할 수 없을 때만 사용하자.
```c++
#include <iostream>
#include <thread>

// boost.asio의 io_context를 사용하고 있다면, 이게 가장 안전하고 효과적인 방법이다.
// 새로운 스레드를 만들어서 detach 시킬 필요도 없고, std::async가 예상치 못하게 blockihg 하게 동작하는 문제도 피할 수 있다.
void delayedExecutorAsyncByIoContext(
		boost::asio::io_context& io_context, int delayInMillis, std::function<void()> task
) {
	auto timer = std::make_shared<boost::asio::steady_timer>(io_context);
	timer->expires_after(std::chrono::milliseconds(delayInMillis));
	// 이 때도 포인터 변수(timer)는 '참조'가 아니라 '값'으로 캡처해서 lambda에게 전달해야 안전하다.
	timer->async_wait([task, timer](const boost::system::error_code& ec) {
		if (!ec) {
			task();
		}
	});
}

// 차선책은 새로운 스레드를 만들고 detach() 시키는 것이다.
// 이것은 std::async가 예상치 못하게 blocking하게 작동하는 문제는 피할 수 있지만,
// detach 시킨 스레드가 강제 종료되는 등의 추가적인 리스크가 존재한다.
// detach() 된 스레드는 OS에서 자동으로 정리하므로, resource 누수가 일어나지 않는다.
void delayedExecutorAsyncByThread(int delayInMillis, const std::function<void()>& task) {
  std::thread([delayInMillis, task]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(delayInMillis));
      task();
  }).detach();
}

// 이것은 std::async를 이용한 버전인덴, 이 버전은 항상 non-blocking 하게 실행되지 않는다는 점에서 불완전하다.
// 그 이유는 컴파일러 별로 std::async의 구현 차이 때문이기도 하고,
// task가 함수 범위를 넘어가면서 소멸되기 직전일 때는 std library에서 dangling ptr 이슈를 막기 위해
// 고의적으로 아래의 std::async를 Blocking 하게 실행되게 만들기 때문이다.
// 이것을 막기 위해서는 무조건 std::thread를 만들어서 거기서 실행시키고 detach 시키던지,
// 아니면 delayedExecutorAsyncByFuture() 이 함수가 리턴하는 future 객체를
// delayedExecutorAsyncByFuture() 함수 범위 바깥에 존재하는 std::future 객체에 저장해야 한다.
std::future<void> delayedExecutorAsyncByFuture(
		int delayInMillis, const std::function<void()> &task
) {
	return std::async(std::launch::async, [delayInMillis, task]() {
	  std::this_thread::sleep_for(std::chrono::milliseconds(delayInMillis));
	  task();
	});
}

class MyClass {
  public:
    void printAsync() {
    std::cout << "Async Success!!" << std::endl;
  }
};

int main(){
  
  // shared ptr를 별도의 변수에 초기화 해 둔 다음, 그 변수를 이용해서
  // weak ptr를 초기화 해야 한다.
  // std::weak_ptr<MyClass> weakPtr = std::make_shared<MyClass>();
  // 와 같이 한 줄에 써 버리면 shared ptr를 참조하는 객체가 단 한 개도 없게 되면서
  // weak ptr를 lock() 해서 점근하려고 할 때 이미 expired()된 상태가 되고 만다.
  std::shared_ptr<MyClass> sharedPtr = std::make_shared<MyClass>();
  std::weak_ptr<MyClass> weakPtr = sharedPtr;

  if (auto ptr = weakPtr.lock()) {
    std::cout << "Delay starts 2!!!\n";
    
    // 람다의 캡쳐 리스트에 '참조'로 전달하는 것과 '포인터'로 전달하는 것에는 큰 차이가 있다.
    // 참조는 복사하지 않고 전달하는 방법이지만, 포인터는 기본적으로 복사로 전달한다는 점이다.
    // 여기서 만약에 [&ptr] 이런 식으로 포인터를 참조로 전달해버리면 async task가 
    // 실행되지 않는다.
    delayedExecutorAsyncByThread(1000, [ptr]() {
        ptr->printAsync();
    });
  } else {
    // 이 부분은 출력되지 않는다.
    std::cerr << "Weak Ptr is expired!!!\n";
  }

  // 이렇게 메인 스레드를 대기시키지 않으면 결과를 보기도 전에 프로그램이 종료돼 버린다.
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  return 0;
}
```
<br><br/>
28. 웬만하면 auto 또는 auto&를 사용하자. Effective Modern C++ 에서 등장한 내용이다.
    <br> 특히, range for 를 이용해서 map을 순회할 때 비효율적인 copy 동작을 예방할 수 있다.
    <br> auto를 쓰면 타입 불일치로 인해 컴파일러가 맵의 구성 요소들을 하는 수 없이 복사하는 일을 방지할 수 있다.
```c++
std::unordered_map<std::string, int> map;

// ...

// 맵을 순회하는 단순한 예제이지만, std::unordered_map의 '키'에 해당하는 부분은
// const 이기 때문에 타입의 불일치가 일어난다.
// 컴파일러는 이를 감지하고 하는 수 없이 원본 맵 내의 pair의 복사본을 만들어낸다.
// 불필요한 숨겨진 복사 행위가 일어나는 것이다.
for(const std::pair<std::string, int>& p : map){
// ...
}

// 대신 이렇게 auto를 쓰면 컴파일러가 알아서 나 대신 정확한 타입을 추론해 내므로,
// 맵 순회 시 불필요한 복사가 일어나지 않는다.
for(const auto& p : map){
// ...
}
```
<br><br/>
29. auto 와 auto& 를 잘 구분해서 써야 한다.
    <br> 그렇지 않으면, 원하지 않는 비효율적인 복사가 일어나거나,
    <br> 원본 객체가 아닌, 복사된 객체에 수정이 돼 버리는 대참사가 벌어질 수 있다.
```c++
// FileReader 클래스 내부에서 이런 일이 있었다.
void FileReader::loadRtpVideoMetaData(
const std::filesystem::path& inputCamDir, std::vector<std::filesystem::path>& videos
) {
  VideoAccess va{};

  // ... 다른 작업들을 마친 뒤,

  // VideoAccess va{} 객체 내의 meta 필드를 초기화 해주는 함수를 호출한다.
  int memberVideoId = 0;
  for (std::ifstream& access : va.getAccessList()) {
    if (access.is_open()) {
    
    
      // 여기서 auto&가 아니라, 그냥 auto videoSampleMetaList ... 이런 식으로 정의해버리면,
      // 원본 객체인 VideoAccess va{}; 내의 meta 멤버가 아니라, 복사된 meta 에다가
      // VideoSampleInfo를 추가하는 로직 오류가 발생한다.
      // 이렇게 현재 함수 범위 내에서만 존재하는 임시 복사본 객체는 현재 함수의 범위가 끝나면 자동으로 
      // 삭제되고 만다.
      auto& videoSampleMetaList = va.getVideoSampleInfoList();
      
      
      int64_t videoFileSize = Util::getFileSize(videos.at(memberVideoId));
      loadRtpMemberVideoMetaData(videoFileSize, access, videoSampleMetaList, memberVideoId);
      memberVideoId++;
    } else {
      logger->severe("Failed to open video reading ifstream!");
      throw std::runtime_error("Failed to open video reading ifstream!");
    }
  }

  videoFiles.insert({inputCamDir.filename().string(), std::move(va)});
}
```
<br><br/>
30. 템플릿 클래스 인스턴스화 문제.
    <br> 현재는 사용하지 않지만, BlockingQueue 클래스를 쓰던 때가 있었다.
    <br> 핵심은, 이러한 클래스의 경우 .h와 .cpp로 정의와 구현을 분리하지 말고 .h 내에 모든 구현을 전부 집어 넣어야 한다는 점이다.
```c++
/*
Chat GPT에 따르면, 템플릿 클래스들은 구체적인 타입과 함께 사용되지 않는 한, 인스턴스화 되지 않는다.
std::unique_ptr<Buffer>와 같은 구체적인 타입이 들어가 있는 txQ가 초기화 돼야 비로소 인스턴스화 된다.

그런데, 이러한 상태에서 .h는 존재하지만, 구현이 .cpp에 분리돼 있을 때는 컴파일러가 템플릿 클래스의
코드를 생성해 낼 때 필요한 정의를 찾지 못해서 

Undefined symbols for architecture arm64:
  "BlockingQueue<std::__1::unique_ptr<Buffer, std::__1::default_delete<Buffer>>>::put(std::__1::unique_ptr<Buffer, std::__1::default_delete<Buffer>>&&)", referenced from:
...
와 같은 링크 에러가 발생하는 것이다.

해결책은 위에서 설명한 대로, 구현을 .cpp에 따로 빼내는 것이 아니라, .h 내에 전부 집어 넣는 것이다.
*/
```
<br><br/>
31. interrupted by signal 11:SIGSEGV 에러 메시지
    <br> 이건 주로 invalid memory access 상황에서 발생하는 에러 메시지다.
    <br> 이게 뜰 경우, exception을 던지거나 로그를 남기기도 전에 프로그램이 죽기 때문에 디버깅 하기가 매우 까다롭고, 정확히 어디에서 에러가 난 것인지 표시조차 되지를 않는다;;
```c++
// 주로 아래의 상황에서 발생한다.

1. 특정 루프 또는 함수의 범위를 벗어나서 삭제된 객체에 접근할 때.
>> 예를 들어서 while(true) 루프 안에서 만든 소켓을 std::shared_ptr에 저장하지 않고
    나중에 다른 함수에서 접근하려고 할 때.

2. 비어 있는 벡터에 [0] 과 같이 인덱스 접근을 시도했을 때.

3. 이미 소멸된 객체를 가리키고 있는 std::shared_ptr를 대상으로 역참조 연산인 '*' 를 시도했을 때.

4. 서로 다른 두 스레드가 하나의 자료구조에 대해서 마구잡이로 접근하려 할 때.
>> 예를 들어서, BlockingQueue에 대해서 한 스레드는 put()을 하고, 한 스레드는 take()를 하려 하는데,
    이게 정확하게 컨트롤 되지 않을 때. 
    
5. 동일한 소켓에 서로 다른 스레드가 동시에 read 또는 write하려고 할 때.
>> socket은 기본적으로 thread safe하지 않다는 점을 기억해야 한다!!

6. boost asio의 steady timer와 같이 특정 함수 객체를 async하게 수행하는 객체를 .cancel() 한 직후 바로 삭재해버릴 때.
>> boost.asio와 같이 비동기 논블록킹 I/O를 지원하는 라이브러리를 사용할 때면 꼭 한 번 이상은 마주치게 되는 오류다.
>> 이때는 boost asio steady timer 객체를 바로 삭제하는 것이 아니라, std::sharad_ptr 같은 걸로 다른 곳에 임시로
    옮겨놓은 후 일정 시간이 지나고 나서 삭제해야 한다.
>> io_context 내에서 특정 task를 실행하려고 하는데, 그 task가 속해 있는 부모 객체(본 프로젝의 경우, PeriodicTask)가
    이미 삭제돼 버려서 invalid memory access가 발생한 경우다.
    
7. boost asio의 소켓에서 특정 버퍼를 이용해서 데이터를 전송하고 있는데, 해당 데이터가 갑자기 dangling pointer가 되거나 삭제될 때.
>>> 버퍼에는 주로 std::array, std::vecotr, raw array 등이 사용될 수 있다.
>>> 이 때는 전송이 완전히 끝난 후 해당 객체들이 삭제될 수 있도록 수동으로 메모리 관리를 해줘야 한다.
>>> std::unique_ptr, std::shared_ptr 등의 스마트 포인터를 사용해도 이러한 문제는 발생할 수 있다.
```
<br><br/>
32. unique_ptr를 요소로서 가지고 있는 BlockingQueue를 설계하는 것.
```text
    - 이런 자료구조는 multi pub/sub 환경에서 thread safe하게 디자인하기가 매우 까다롭다.
    - 최대한 단일 스레드에서 처리할 수 있게 설계하거나, 아니면 boost.asio의 io_context를 쓰거나,
    - boost 라이브러리 내의 Lock-Free, concurrent 등의 라이브러리에서 제공하는 자료구조를 쓰자.
    - 다시 한 번 강조하지만, 성능 문제가 없는 한 웬만하면 단일 스레드에서 mutex 없이 처리하는게 좋다.
```
<br><br/>
33. java의 str.startsWith("..")는 C++17에서는 rfind()를 이용해서 대체할 수 있다.
    <br> C++20에서는 str.starts_with("...") 이 std library에서 지원되지만, 그 이전 버전에서는 지원되지 않는다.
```c++
bool startsWith(const std::string& str, const std::string& prefix) {
    // 문자열이 prefix 로 시작하면 true, 아니면 false를 리턴.
    return str.rfind(prefix, 0) == 0;
}
```
<br><br/>
34. C++ 에서는 시간을 다루는 라이브러리가 많다. java의 System.currentTimeMillis() 와 System.nanoTime()을 C++에서 다음과 같이 구현할 수 있다.
    <br> currentTimeMillis()는 1970년 1월 1일 자정 ~ 현재까지의 시간 차이를 밀리초로 나타낸 값을 리턴한다.
    <br> nanoTimes()는 wall-clock이 아니며, 시간 간격(ex : 함수 수행 완료에 걸린 시간) 측정에 사용된다.
```c++
    // Util.h 에 구현돼 있다.
    
inline int64_t getCurrentTimeMillis(){
        // time_since_epoch 를 쓰고 있다.
		return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
	}

inline int64_t getElapsedTimeNanoSec(){
	    // time_since_epoch 같은 특정 고정 기준 시각 대신,
	    // steady_clock 을 사용한다.
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::steady_clock::now().time_since_epoch()
		).count();
	}
```
<br><br/>
### 35. socket을 이용하여 Async한 작업을 처리하는 객체는 어떻게 삭제해야 하는가?
    <br> Async 하다는 것은 무엇을 뜻하는가? 핵심은 '작업들이 어떤 순서로 언제 끝날지 알 수 없다'는 것이다.
    <br> 예를 들어서, 내가 '지금' boost asio steady timer를 .cancel() 시켰다고 해서 '즉시' 해당 타이머 태스크가 중단되고 바로 소멸되는 것이 아니다.
    <br> 이것을 염두에 두지 않고 마치 sync 작업을 처리할 때처럼 순서대로 내가 원할 때 바로바로 객체들을 삭제하면 여지 없이 운영체제에 의해서 SIGSEGV 같은 segmentation falt 관련 에러 메시지를 받으면서 서버가 죽는다.
    <br> boost asio 라이브러리가 OS와 긴밀하게 상호작용하면서 처리하고 있는 async 태스크들이 어떻게 처리되고 있는지를 전허 고려하지 않고 해당 태스크들이 참고하고 있는 세션 객체를 삭제해버렸기 때문이다.
    <br> 핵심은 먼저 모든 자원들을 완전히 회수한 다음, async task 들이 완전히 종료될 때까지 기다렸다가 세션 객체를 실제로 삭제시켜야 한다는 점이다.  
```c++
// 자세한 구현은 Session.h, Session.cpp, Server.h, Server.cpp 를 참조한다.
// 여러가지 방법들과 테스트를 수행해본 결과, 핵심은 Rtsp 트랜잭션을 담당하는 boost asio steady timer의 제거 타이밍이었다.
// 현재 Session 객체는 다음의 타이머들을 필요로 한다. 
// bitrate 기록, rtsp 트랜잭션 담당, 비디오 샘플 전송, 오디오 샘플 전송 이렇게 4 개다.

// Boost.Asio 라이브러리를 이용해서 네트워킹을 처리하면, 소켓을 이용하는 입출력이 라이브러리에 의해서 대신 처리된다.
// 아직 이러한 작업이 진행되고 있는 중간에 갑자기 소켓을 닫아버리거나, 세션 객체를 삭제하는 등의 '갑작스러운' 동작을 수행하면 서버가 미처 예외를 던지기도 전에 segmentation fault 등의 예외를 던지며 바로 죽는다.

이번 RTSP 서버의 경우, 다음의 상황에서 서버가 OS로부터 SIGSEGV 신호를 받은 후 죽었다.
1. 클라이언트가 teardown 요청에 대한 응답을 소켓에 write 하고 있을 때 세션 객체를 삭제한다.
2. teardown에 대한 응답을 소켓에 wirte 한 직후에 세션 객체를 삭제한다.
3. 세션 객체 내에 존재하는 4 개의 boost asio steady timer를 .cancel() 시키고 일정 시간 기다리지 않고 바로 객체를 삭제한다.

이러한 현상을 근거로
>> teardown 요청을 소켓에 write 한 다음, 5 초간 아무 활동도 하지 않도록 대기 시킨다.
>> 5초가 지난 후, 세션 내에 존재하는 모든 boost asio steady timer 들을 cancel 시켜주고,
>> 세션 내에 존재하는 모든 자원들(소켓, 파일 핸들러 등등)을 닫아주고, 
>> 세션 내에 존재하는 모든 boost asio steady timer 태스크들이 완전히 .cancel() 될 때까지 기다란 다음,
>> 마지막으로 세션 객체를 삭제하도록 구현 했다.

 Server.cpp의 생성자와 start() 함수 코드를 보면, 세션을 삭제할 때 해당 세션을 바로 삭제하지 않고 다른 맵(shutdownSessions 맵)에다가 옮겨둔 다음,
 Server.cpp 내에서 30초 간격으로 shutdownSessions 맵을 clear 해주는 별도의 PeriodicTask가 정의돼 있는 것을 확인할 수 있다.
 
 >>> 이렇게 고의적인 delay를 주면서 천천히 세션을 제거해주니까 세션 삭제시 발생한 segmentation fault 오류를 잡아낼 수 있었다.
```
<br><br/>
36. boost asio steady timer 객체를 초기화 하는 방법
    <br> 해당 타이머는 boost aiso io_context에 주기적으로 task를 공급하는 역할을 하고, non-blocking이다.
    <br> 그렇기 때문에, 전체 프로그램 종료시 해당 타이머를 명시적으로 종료시키지 않을 경우 서버가 SIGSEGV 같은 비정상적인 에러 코드로 종료 되거나(MacOS) 아예 종료되지 않는 현상(Ubuntu & Amazon Linux)이 발생한다.
    <br> 이를 방지하기 위해서는 해당 타이머를 클래스 멤버로 정의하여 클래스의 소멸자에서 타이머를 cancel 시키는 등의 방법으로 타이머의 종료가 보장되도록 해야 한다.
```c++
// 좋은예.
class Server {
public:
    // ...
private:
    // ...
    PeriodicTask shutdownSessionRemovalTask; // 이렇게 클래스 멤버로 만들어 놓으면 원하는 때에 타이머를 .cancel() 하기 편하다.
};

// 안 좋은 예.
// boost asio steady timer를 이렇게 함수 범위 내에서만 정의하면, 해당 함수를 수행하고 난 다음에는 타이머에
// 접근하여 .cancel() 시키는 등의 동작을 할 수가 없게 되고, 해당 함수가 한 번 더 호출되기라도 하면
// 원하지 않는 타이머 태스크가 하나 더 생길 수도 있다.
void bad(){

    // 함수 바디 안에서 타이머를 시작시킨다.
    // non-blocking 이기 때문에, 함수의 실행이 여기서 멈추지 않는다.
    std::chrono::milliseconds interval(1000);
    PeriodicTask task(io_context, interval, [&](){ /* do some work */ });
    task.start();
    
    // ... 다른 일들을 처리한다.

}
```
<br><br/>
37. 네트워크 프로그래밍을 할 때는 socket buffer size를 고려하여 전송 bitrate의 급등을 막아야 한다.
    <br> API 서버가 요청 중심의 서버라면, RTSP는 전송 중심의 서버다. 클라이언트가 필요로 하는 모든 샘플들을 전부 서버가 전송해야 하기 때문이다.
    <br> 서버가 클라이언트와 통신하기 위해서는 socket을 써야하고, socket에는 당연히 물리적인 한계가 존재한다. 이러한 물리적인 한계에는 시간적 한계와 공간적 한계가 있다.
    <br> 시간적 한계는 얼마나 높은 Hz 로 단위 시간 이내에 자주 샘플을 전송할 수 있느냐이고,
    <br> 공간적 한계는 한 번에 얼마나 많은 데이터를 보낼 수 있느냐이다.
    <br> 네트워크 프로그래밍의 경우, 특히나 '공간적 한계'에 취약하다. socket buffer의 size()를 운영체제 커맨드로 MB 단위로 늘리자, t2.medium 같은 저사양 EC2에서는 단 한 명의 클라이언트의 재생조차 끝까지 감당하지 못하고 EC2가 통째로 먹통이 되었다.
    <br> 원래 t2.medium EC2의 socket read/write buffer의 사이즈는 둘 다 212992 byte(약 213 KB) 밖에 되지 않는다.
    <br> 그런데 여기에다가 MB 단위의 비디오 샘플들을 33 밀리초마다 한 번씩 전부 socket buffer에 copy 시키니 당연하게도 순간적인 송신 bitrate가 엄청난 spike를 기록하면서 서버가 down 될 수밖에 없었던 것이다.
    <br> 그리고 이러한 행위는 EC2의 credit balance를 짧은 시간 안에 급격하게 떨어뜨리기 때문에, 방치할 경우 요금 폭탄을 맞을 수도 있다.
```c++
// 현재의 구현은 다음과 같은 사항을 고려하여 설계되었다.

1. 미리 할당된 메모리 공간만을 비디오 및 오디오 TX에 활용함으로써 heap fragmentation 문제를 방지한다.
    >> 여기에는 boost object pool이 사용되었다.
    >> 관련 내용이 Session.h 의 데이터 멤버 필드에 구현돼 있다.
    
2. 샘플을 파일 스트림에서 읽어들인 후 바로 전송하는 것이 아니라, rtp 단위로 쪼개서 txQueue에 저장해 놓는다.
    >> 관련 내용이 Session.cpp, StreamHandler.cpp, RtpHandler.cpp 에 구현 돼 있다.
    
3. txQueue에 저장된 RTP 패킷들을 전송하는 일만을 담당하는 전용 PeriodicTask를 정의하여 송신 bitrate가 급등하는 것을 막는다.
    >> 즉, 샘플을 한 번에 전부 전송하는 것이 아니라, rtp 단위로 쪼개서 송신 큐에 답고, 송신 큐에서 rtp를 하나씩 꺼내서 전송하는 것이다.
    >> CPU 의 클럭수는 GHz 단위로 충분히 큰 반면 socket buffer라는 물리적 공간의 사이즈 기껏해야 수백 KB 단위로 한정돼 있기 때문에 이렇게 전송하는게 오히려 성능과 효율성의 측면에서 모두 탁월하다.
    >> txQueue로는 mutex lock & unlock이 필요없는 thread safe 큐인 boost lock free 큐를 사용하여 불필요한 locking/unlocking 연산을 없앴다.


// 아래의 내용은 이번 rtp sending architecture를 개발하면서 알게된 tip 이다.

// Amazon Linux가 설치된 머신의 socket buffer 사이즈를 알아내는 명령어이다.
sysctl net.core.wmem_max
sysctl net.core.rmem_max

// Amazon Linux가 설치된 머신에서 socket buffer size를 특정 값으로 변경할 때 사용하는 명령어이다.
sudo sysctl -w net.core.wmem_max=5242880
sudo sysctl -w net.core.rmem_max=5242880
```
<br><br/>
38. Map을 탐색할 때는 바로 .at(key) 또는 map[key] 으로 객체를 꺼내지 말고, find()를 호출해서 iterator로 꺼내자.
    <br> .at(key) 또는 map[key]로 바로 꺼내면, 해당 key가 맵에 존재하지 않을 때 out of range 예외가 뜨면서 앱이 바로 죽는다.
    <br> 이 방법은 map 뿐만 아니라, std::vector<T> 등의 다른 컨텐이너에서도 활용할 수 있다.
```c++
// RtpHandler 내의 readVideoSample() 가 성능을 위해서 noexcept로 선언 돼 있고,
// out of range exception을 막가 위해서 iterator 기반 find로 맵을 탐색하고 있다. 
void RtpHandler::readVideoSample(
  VideoSampleRtp* videoSampleRtpPtr,
  const VideoSampleInfo& curFrontVideoSampleInfo,
  const VideoSampleInfo& curRearVideoSampleInfo,
  int camId,
  int vid,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap
) noexcept {
  // camIt 가 바로 map.find(key)의 결과로서 반환 되는 iterator 다.
  const auto camIt = camIdVideoFileStreamMap.find(camId);
  if (camIt == camIdVideoFileStreamMap.end() || camIt->second.size() < 2) {
    logger->severe("Dongvin, invalid camId or insufficient video file streams! camId: " + std::to_string(camId));
    return;
  }

  // iterator를 이용해서 필요한 객체의 참조를 얻는다.
  std::ifstream& frontVideoFileReadingStream = camIt->second[0];
  std::ifstream& rearVideoFileReadingStream = camIt->second[1];

  // ... std::ifstream 들을 이용해서 비디오 샘플들을 읽는다 ...
}
```
<br><br/>
39. boost.asio 의 tcp 소켓에서 async_read_some()을 사용할 때 주의할 점
    <br> 정확히 1 개의 요청을 받이들이는 것이 아니라, 뭔가 요청이 소켓 버퍼에 들어오면 주어진 버퍼 사이즈가 허락하는 한 전부 읽어들이는 것이다.
    <br> 따라서 다수의 RTSP 요청이 운 나쁘게도 거의 동시에 왔을 경우, 여러 개의 요청을 한 번의 async_read_some()으로 전부 읽어들일 수도 있다.
    <br> async_read_some() 으로 읽어들인 buffer의 내용을 가지고 별도의 처리 없이 바로 rtsp msg 처리를 하면 2 개의 요청을 하나로 인식하는 바람에 다음 요청 때는 CSeq 번호가 꼬이면서 서버가 클라이언트에게 400 bad request를 리턴하게 된다.
    <br> 이 문제를 피하기 위해서는 buffer의 내용을 다른 곳으로 옮겨 놓은 후, 여기에서 별도로 rtsp request 를 하나씩 꺼내서 처리해야 한다. 즉, 요청들에 대한 parsing을 해야 한다.
```c++
// 그 방법이 Session.cpp 의 최하단에 있는 asyncReceive() 함수에 구현 돼 있다.
// 세션이 시작될 때, asyncReceive() 함수가 최초로 호출 된다.
void Session::asyncReceive() {
  // ... 오류 처리 ...
  
  auto buf = std::make_shared<std::vector<unsigned char>>(C::RTSP_MSG_BUFFER_SIZE);
  socketPtr->async_read_some(
    boost::asio::buffer(*buf),
    [this, buf](const boost::system::error_code& error, std::size_t bytesRead) {
      // ... 오류 처리 ...
      
      // 방금 읽어들을 요청 1 개 또는 여러 개를 별도의 버퍼에 담는다.
      rtspBuffer.append(reinterpret_cast<char*>(buf->data()), bytesRead);
      
      // 다음의 while 루프에서 정확하게 딱 1 개의 rtsp 요청을 꺼내서 처리한다.
      // 모든 RTSP 요청은 반드시 \r\n\r\n(== C::CRLF2 상수) 으로 끝나야 한다는 점을 이용한 것이다.
      while (true) {
        size_t pos = rtspBuffer.find(C::CRLF2);
        if (pos == std::string::npos) break; // 처리할 요청이 없으면 루프를 끝낸다.
        
        std::string request = rtspBuffer.substr(0, pos + 4);
        rtspBuffer.erase(0, pos + 4);
        
        auto bufferPtr = std::make_unique<Buffer>(
          std::vector<unsigned char>(request.begin(), request.end()), 0, request.size()
        );
        handleRtspRequest(*bufferPtr);
        
        // ... 응답 내용 출력 ...
        
        transmitRtspRes(std::move(bufferPtr));
      }// end of while

      // 다음 요청을 기다리기 위해서 재귀적으로 자기 자신을 호출하여 요청 처리 루프를 만든다.
      asyncReceive();
    }// end of lambda
  );// end of async_read_some()
}
```
<br><br/>
40. C++ 표준을 준수하자.
    <br> 어떤 기능들은 특정 컴파일러에서는 컴파일 및 실행이 되지만, 다른 컴파일러에서는 컴파일에 실패하는 경우가 있다.
    <br> VLA(Variable Length Array)가 대표적으로 이러한 경우다.
    <br> 다른 사례가 발생하면 여기에 추가로 기록하자.
```c++
case 1 : VLA
// 겉으로 보기에는 잘 작동할 것처럼 보이지만, 아래와 같이 런타입에 특정 타입의 배열의 크기를 정하는 것은
// C++ 17 표준이 아니다. C++ 표준을 엄격하게 준수하는 MSVC에서는 아래의 코드 때문에 컴파일이 되지 않는다.
// C++ 표준에서는 컴파일 타임에 배열의 길이가 상수로서 결정되어야 한다.
// VLA는 GCC, G++, Apple CLang에서는 non-standard extension으로 작동하지만, Window MSVC에서는 지원하지 않는다.

        // ...
        const int rtpLen = Util::getRtpPacketLength(
            videoSampleRtpPtr->data[2], videoSampleRtpPtr->data[3]
        );
        const int len = 4 + rtpLen;

        // 바로 이 부분 때문에 컴파일 타임 에러가 발생한다. 런타임에 따라서 배열의 크기를 결정했기 때문이다.
        // 배열 대신 std::vector를 쓰게 바꾸자 컴파일이 되었다.
        unsigned char rtp[len];

        std::memcpy(rtp, videoSampleRtpPtr->data, len);

        std::vector<unsigned char> buf;
        for (unsigned char c : rtp) buf.push_back(c);
        // ...


case 2 : "falling off the end of a function" or "implicit return undefined behavior"

        // 아래의 함수는 void가 아니지만, 컴파일 및 실행을 하면 실행이 되기는 한다.
        // GCC, G++, Apple CLang에서는 되지만, Window MSVC에서는 컴파일 타임 에러가 뜬다.
        bool Session::isPlayDone(int streamId) {
            // void가 아닌데, 아무 것도 리턴하지 않는다...
        }
```
<br><br/>
41. 참조를 캐싱하는 방법 - '참조'와 '포인터' 간의 관계
    <br> const reference를 리턴하는 함수들을 연쇄적으로 호출해야 한다고 가정하자.
    <br> 함수 호출은 분명 H/W 적 오버헤드가 발생한다.
    <br> 결론적으로 특정 객체의 참조에서 주소값을 알아내서 그 주소값을 가지고 호출하면 연쇄적인 함수 호출을 최소화 할 수 있다.
    <br> 이것이 가능한 이유는, C++의 참조는 컴파일 시 C 스타일 포인터로 변환되기 때문이다.
    <br> C++ 의 참조는 '안전한 사용을 위해 제한 사항을 더 많이 적용한 포인터'다.
    <br> 좀 더 간단하게는, '참조는 포인터 dereferencing이 완료된 상태'다.
    <br> 아래의 표를 보면 이 의문에도 답 할 수 있게 된다. "참조 타입 멤버 변수는 왜 반드시 생성자에서 초기화 해줘야 하는가?"
```c++
// 그래서 StreamHandler 에서 이런 동작이 허용된다.
// getConstVideoSampleInfoList()는 const std::vector<std::vector<VideoSampleInfo>>& 를 리턴하기 때문이다. 
// cachedCam0frontVSampleMetaListPtr 의 타입은 const std::vector<VideoSampleInfo>* 이다.
    if (cam0Iter != contentMeta.end()) {
      const auto& frontVMeta
        = cam0Iter->second.getConstVideoSampleInfoList().at(C::FRONT_VIDEO_VID);
      const auto& rearVMeta
        = cam0Iter->second.getConstVideoSampleInfoList().at(C::REAR_VIDEO_VID);
      
      // '참조'에서 '참조가 가리키는 객체의 주소값'을 가져다가 별도의 포인터 변수에 복사해 놓는다.
      // 이 포인터 변수를 역참조하면 반복적인 함수 호출을 단 1회로 줄일 수 있다.
      cachedCam0frontVSampleMetaListPtr = &frontVMeta;
      cachedCam0rearVSampleMetaListPtr = &rearVMeta;
    }
    
// 똑같은 기능을 C++과 C에서 어떻게 서로 다르게 구현하고 있는지를 보면 참조와 포인터 간의 관계와 차이점을 명확히 알 수 있다.
#include <stdio.h>
void increment(int* x) {  // Pass by pointer
    (*x)++;
}
int main() {
    int a = 5;
    increment(&a);  // Pass address of a
    printf("%d", a); // Output: 6
}
// C로 작성한 프로그램을 C++의 참조로 재작성해보면 다음과 같다.
#include <iostream>
void increment(int& x) {  // Pass by reference
    x++;
}
int main() {
    int a = 5;
    increment(a);  // a is modified directly
    std::cout << a; // Output: 6
}
    
// '참조'는 C에서는 존재하지 않는 C++ 고유의 기능이다. '참조'는 결국 '안전한 사용을 위해서 제한 사항이 몇 가지 적용된 포인터'다.
// 둘 간의 차이점을 GPT가 정리해주었다.
```
| Feature            | C++ References | C Pointers |
|--------------------|---------------|------------|
| Can be `nullptr`? | ❌ No (must always bind to an object) | ✅ Yes (can be NULL) |
| Can be reassigned? | ❌ No (always refers to the same object) | ✅ Yes (can point to different objects) |
| Syntax complexity | ✅ Simple (`x` behaves like normal variable) | ❌ Requires `*` (dereferencing) and `&` (address-of) |
| Memory usage      | ✅ Often optimized | ✅ Explicit, but requires careful management |
| Required for function parameter modification | ✅ Yes | ✅ Yes | 

<br><br/>
42. boost::pool과 boost::object_pool의 위험성 : Out Of Memory 발생 가능
    <br> 부스트 라이브러리의 pool과 object_pool은 Bad Access(Segmentation Fault)를 방지해준다.
    <br> 그러나, 정확하게 사용하지 않으면 메모리 공간의 엄청난 낭비를 초래할 수도 있다.
    <br> 본 프로젝트에서는 비디오 샘플을 읽어들이기 위한 공간을 3MB로 설정하고 계속 재활용하게 만들려고 했으나, pool의 동작을 정확하게 테스트하지 않고 사용했기에 엄청난 메모리 낭비가 발생했었다.
    <br> 분명 동시접속 사용자 수가 겨우 36명 정도밖에 안 되는데, Out Of Memory 이슈로 Linux kernel에 의해서 자꾸 서버가 강제 종료됐던 것이다. 조사 결과, 1 명당 약 100 MB 씩이나 잡아먹고 있었다.
    <br> 이 문제는 결국 boost pool과 object_pool을 전혀 사용하지 않고, std::shared_ptr로 할당한 비디오/오디오 샘플을 참조하고 있는 RTP 패킷 객체의 개수를 일일이 추적해서 해결했다. 참조 중인 RTP 패킷의 개수가 0이 된 비디오/오디오 샘플이 표준 라이브러리에 의해 자동으로 회수되게끔 코드를 전면 수정한 것이다.
    <br> 그 결과, 클라이언트 1 명당 메모리 사용량을 약 4 MB 정도로 대폭 낮출 수 있었다.
```c++
boost pool과 object_pool은 Segmentation Fault 예방에는 좋지만,
충분히 테스트 하지 않고 사용할 경우 매우 많은 메모리 공간을 낭비하게 만들 수 있다. 

boost pool와 object_pool의 기본 동작은 다음과 같다.

pool 과 object pool은 기본적으로 생성 직후 1 개의 chunk를 할당하며,
1 개의 chunk 안에는 32개의 object가 들어갈 수 있는 공간이 할당된다.

1 개의 chuck에 32개의 객체가 있고, 1 객체당 3MB를 할당했으니 
클라이언트 1 명당 당연하게도 100 MB 가까운 메모리를 잡아먹은 것이다.

pool.set_next_size(1);
이렇게 할당하는 것은 pool 생성 이후에 공간이 부족해질 경우, object 1 개에 해당하는
사이즈만큼만 추가 할당하게 만드는 것이다.
처음 pool, object_pool이 생성될 때 32개의 객체들이 들어갈 공간을 할당하는 것을
막아주는 것이 아니다.

그리고 이것을 고려해서 pool, object_pool이 생성된 직후

최초의 pool.construct();가 호출되기 이전에 pool.set_next_size(1);를 호출하고,

최초의 object_pool.construct();가 호출되지 이전에 object_pool.set_next_size(1);를
호출하더라도,
아래의 43. 번 팁에서 설명할 valgrind를 사용해서 검사했을 때,
클라이언트 1 명당 메모리 소모량이 30~40 MB로 여전히 매우 높았다.

pool, object_pool이 최초 생성 직후부터 오직 1개의 chunk만을 할당하고,
각 chunk 별로 1 개의 객체만 할당하게 만들기 위해서는 최초의 .construt();가
호출되기 이전에
boost::object_pool<MyObject> pool(boost::default_user_allocator_new_delete(), 1);
이런 식으로 초기화 해야 한다.

추가로, pool과 object_pool에서는 .destroy()를 명시적으로 호출해주지 않으면
할당된 메모리가 회수되지 않으며,
.free() 함수는 할당했던 메모리를 실제로 회수하는 것이 아니라, 단지 '사용 가능하다'라고
마킹만 해두는 함수다.
```

<br><br/>
### 43. C/C++ 프로그램의 메모리 사용량을 추적하고, 관련 이슈를 진단하는 방법
    <br> valgrind를 사용하면 C/C++ 프로그램의 메모리 사용량을 시간의 흐름에 따라서 추적할 수 있다.
    <br> 그리고 이것을 그래프로 나타낼 수 있다.
    <br> 본 프로젝트를 Amazon Linux EC2에서 성능테스트를 진행하면서 실시한 메모리 검사 방법을 소개한다.
```shell
valgrind를 설치해야 한다. Chat GPT에게 물어보면 된다.
메모리 검사를 진행하기 전에, /tmp 디렉토리에 대한 쓰기 권한을 허용해 줘야 한다.
그후, 아래의 두 가지 명령어를 실행한다.
sudo chmod 777 .
sudo chmod 777 /tmp

./fast_build_and_run_for_dev_linux_foreground.sh
명령어로 본 프로잭트를 실행했다가 Ctrl + C 키로 종료시킨 후,
build 디렉토리로 이동한다. 해당 디렉토리 안에는
RtspServerInCpp 라는 executable binary 파일이 생성돼 있을 것이다.

그 후,
valgrind --tool=massif --massif-out-file=/tmp/massif_output_1client2.txt ./RtspServerInCpp
명령어를 실행한후, 성능 테스트를 진행하고 Ctrl + C 키를 눌러서 테스트를 종료한다.

그 다음,
ms_print /tmp/massif_output_1client2.txt | less > result.txt
명령어를 실행해서 그래프 데이터를 .txt 형태로 저장한다.

메모리 사용량 그래프를 출력해본다.
cat result.txt
```
```text
>>> 아래의 result.txt 출력 결과는클라이언트가 1 명일 때, RtspServerInCpp 서버 전체의
 메모리 사용량을 그래프로 나타낸 것이다.
Y축은 MB 단위로 표현한 메모리 사용량이고,
X축은 실행된 CPU 명령어의 개수를 (1백만 개) 단위로 표시한 '시간축'이다.
#,@,: 등의 기호가 보이는데, 각각이 의미하는 바는 GPT에게 물어본 결과, 다음과 같다.
# (sharp):
Total memory allocated on the heap including heap overhead
(actual malloc'd + allocator metadata).

@ (at):
Memory allocated directly by the user
(i.e., what your code requested via new, malloc, etc.).
>>> std::make_shared<T>를 사용했다면, 여기에 기록된다.

: (colon):
Represents memory allocations from individual call stacks.
Each colon corresponds to a specific allocation stack.
More colons = more contributing call stacks.

* (asterisk) [not shown in your graph but sometimes appears]:
If memory grows rapidly or peaks sharply, * can represent overflowed/condensed graph points (compression).

[root@ip-172-31-1-183 build]# cat result.txt
--------------------------------------------------------------------------------
Command:            ./RtspServerInCpp
Massif arguments:   --massif-out-file=/tmp/massif_output_1client2.txt
ms_print arguments: /tmp/massif_output_1client2.txt
--------------------------------------------------------------------------------


    MB
4.017^                  #
     |                  #                                @   :
     |                  #                                @   :            :
     |                  #       :                        @   :   :    :   :
     |             @    #       :       :       ::  ::   @   :   :    :  ::
     |             @    #       :    :  :       :   :    @   :   :    :  ::
     |             @    #       :    :  :       :   :    @   :   :   ::  ::
     |             @    #   :   :    :  :   :   :   :    @   :   ::  ::  ::
     |         @@  @    #   :   :    :  :   :   :   :    @   :   ::  ::  ::
     |         @   @    #  :: : :    :  :   :   :   :    @   :   ::  ::  ::  :
     |         @ ::@::::#:@::@::::::::::::::::::: ::: :::@::::@:::@::::@::::@:
     |         @ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |        :@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |       @:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |     : @:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |    @::@:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |    @::@:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |   :@::@:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     |  @:@::@:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
     | :@:@::@:@ ::@::: #:@::@::::::::::::::::::: ::: : :@::::@:::@::::@::::@:
   0 +----------------------------------------------------------------------->Mi
     0                                                                   653.6

Number of snapshots: 97
 Detailed snapshots: [2, 4, 7, 9, 12, 16 (peak), 19, 22, 52, 62, 72, 82, 92]
 
 ... 후략
```

<br><br/>
### 44. boost::asio::io_context pool 도입의 필요성과 방법
    <br> io_context는 boost::asio의 핵심이다. 네트워킹(read, write, async_read, async_write), steady timer, strand, work guard 등이 전부 여기에 의존한다.
    <br> 그렇기 때문에 모든 task들이 전부 io_context에 의존할 경우 오히려 성능이 급격하게 하락하는 문제가 발생한다. io_context는 task 큐이자, scheculer이자, dispatcher이기 때문에 과도하게 task들이 몰릴 경우 task 큐가 쌓이면서 딜레이가 발생할 수밖에 없기 때문이다.
    <br> 성능 테스트 이전에는 모든 task들을 전부 1 개의 단일한 io_context에서 처리하는 구조였는데, 이 경우 동시접속자 수가 t2.medium ec2에서 36명을 초과하자 성능이 급격하게 낮아졌다.
    <br> 이 문제는 단순히 io_conext.run()을 돌리는 스레드의 개수를 늘린다고 해서 해결되지 않는다.
    <br> io_context 인스턴스의 숫자 자체를 늘려서 io_context pool을 만들고, 각종 task들을 이 pool에 균등하게 분배시켜야 하며,
    <br> rtp 패킷 전송과 같이 매우 많은 회수로 실행시켜야 하는 task는 io_context가 아니라 아예 별도의 detached while loop 스레드에서 실행되게 만들어서 io_context의 부하를 줄여야 해결된다.
```c++
자세한 구현은 main.cpp, Server.cpp, Session.cpp 코드를 참조한다.
이때 중요한 것은, acceptor, socket을 만드는 것에 사용되는 '메인 io_context'는
절대 건드리지 말고 새로운 io_context pool만을 추가로 생성자에 전달해야 한다.

acceptor(== server socket의 역할)와 socket은 기존의 메인 io_context에서 만들게
놔두고, socket 생성 후 해당 소켓 또는 다른 steady timer(본 프로젝트에서는 PeriodicTask)
를 이용하는 task들을 메인 io_context가 아니라, io_context pool에게 전달하는 것이다.

io_context.run(); 함수는 boost asio work_guard 없이는 작업이 없을 때 곧장 종료돼
버리므로, io_context.run();을 호출하기 전에 work_guard를 모든 io_context 인스턴스 각각에
대해서 1 번씩 다 초기화 해줘야 한다. 
```

<br><br/>
### 45. while(true){transmitRtp();} 루프의 위험성
    <br> 이런 루프를 busy-wait loop 또는 spin loop라고 한다.
    <br> 이 루프에서는 blocking, 또는 sleep 동작이 없다.
    <br> 즉, 이 루프를 실행시키는 CPU core가 다른 일을 할 수 있도록 놔주는 시간이 전혀 없다는 뜻이다.
    <br> 그 결과, 다른 일을 해야 하는 CPU가 현재의 루프에 묶이게 되면서 다은 일들의 실행이 죄다 뒤려 밀려나는 극도로 비효율적인 상황이 발생한다.
    <br> 이러한 상황을 막기 위해서는 C++의 경우 condition variable과 mutex를 이용해서 blocking을 구현하거나, 이 프로젝트에서와 같이 queue가 비어 있을 경우 의도적으로 while 루프를 실행시키는 스레드를 잠들게 만들어야 한다.
    <br> 결론적으로, 'blocking 동작'이 무조건 비효율적인 동작은 아닌 것이다.
```c++
자세한 구현은 Session.cpp 내의
void Session::start() {...} 함수 내부의 detached 된 while loop 를 참고한다.
```

<br><br/>
### 46. 스트리밍 서버에서 근본적으로 OOM(Out Of Memory) 문제를 예방하는 방법
    <br> 결론적으로, 비디오/오디오 샘플을 std::shared_ptr 등의 방법으로 할당하면서 byte 수를 기록해놨다가 일정 기준을 넘어가면 할당을 금지키시는 것이다.
    <br> 물론, RTP 패킷을 전송완료 했다면 byte 숫자 값을 그만큼 감소시켜야 한다.
```c++
Session.h 내의 
  std::atomic<int64_t> allocatedBytesForSample = 0;
라는 멤버 변수가 사용되고 있는 코드들을 추적해보면 된다.

해당 allocatedBytesForSample 값이 일정 기준을 초과할 경우, 비디오/오디오 샘플 리딩 작업을
시작하지 않게 구현하였다.

이러한 제한 사항이 없을 경우,
동시접속자들이 급증해서 서버가 rtp 패킷들을 전송하는 속도가  느려지고,
rtp 패킷들을 만들기 위해서 할당한 비디오/오디오 샘플 데이터가 전송되기를 대기하면서
메모리에 비디오/오디오 데이터가 점점 쌓이게 된다.
한번 할당한 비디오/오디오 샘플은 세션이 종료되거나, 전송이 완료될 때까지 메모리에서 회수해서는 안 되기 때문이다.

다수의 클랑이언트 각각에 대해서 이러한 '메모리 누적 현상'이 지속될 경우, 서버는 결국
Out Of Memory 이슈를 감지한 OS kernel에 의해서 강제종료 된다.
```