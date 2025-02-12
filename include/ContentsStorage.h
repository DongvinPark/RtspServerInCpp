#ifndef CONTENTSSTORAGE_H
#define CONTENTSSTORAGE_H
#include <unordered_map>

#include "../include/FileReader.h"

class ContentsStorage {
public:
  explicit ContentsStorage(const std::string contentStorage);
  ~ContentsStorage();

  void init();

  FileReader& getCid(std::string cid);
  const std::unordered_map<std::string, FileReader>& getReaders() const;
  void shutdown();
  std::string getContentRootPath() const;

private:
  std::shared_ptr<Logger> logger;
  std::filesystem::path parent;
  std::unordered_map<std::string, FileReader> readers;
  std::string contentRootPath;
};

#endif //CONTENTSSTORAGE_H
