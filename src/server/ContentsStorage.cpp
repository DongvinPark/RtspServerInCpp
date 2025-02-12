#include "../include/ContentsStorage.h"
#include "../constants/C.h"

ContentsStorage::ContentsStorage(const std::string contentStorage)
  : logger(Logger::getLogger(C::CONTENTS_STORAGE)),
    contentRootPath(contentStorage),
    parent(std::filesystem::path(contentStorage)) {}

ContentsStorage::~ContentsStorage() {
  shutdown();
}

void ContentsStorage::init() {
  std::vector<std::filesystem::path> files;
  for (const auto entry : std::filesystem::directory_iterator(parent)) {
    if (entry.is_directory()) {
      files.push_back(entry.path());
    }
  }

  for (const std::filesystem::path contentPath : files) {
    std::string contentTitle = contentPath.filename().string();
    readers.try_emplace(contentTitle, FileReader(contentPath));
  }

  std::string availableContents = ">> ";
  for (auto& kvPair : readers) {
    kvPair.second.init();
    availableContents += kvPair.first + ", ";
  }

  logger->warning("Available contents: " + availableContents);
}

FileReader & ContentsStorage::getCid(std::string cid) {
  // return reference : FileReader is not allowed to copy
  FileReader& fileReader = readers.at(cid);
  return fileReader;
}

const std::unordered_map<std::string, FileReader> & ContentsStorage::getReaders() const {
  return readers;
}

void ContentsStorage::shutdown() {
  // circulate through reference. FileReader is not allowed to copy.
  for (auto& kvPair : readers) {
    kvPair.second.shutdown();
  }
  readers.clear();
}

std::string ContentsStorage::getContentRootPath() const {
  return contentRootPath;
}
