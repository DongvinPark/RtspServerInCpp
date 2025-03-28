#ifndef CONTENTSSTORAGE_H
#define CONTENTSSTORAGE_H
#include <unordered_map>

#include "../include/ContentFileMeta.h"

class ContentsStorage {
public:
  explicit ContentsStorage(const std::string contentStorage);
  ~ContentsStorage();

  // rule of five. ContentsStorage is not allowed to move or copy
  ContentsStorage& operator=(const ContentsStorage&) = delete;
  ContentsStorage& operator=(ContentsStorage&&) noexcept = delete;
  ContentsStorage(ContentsStorage&&) noexcept = delete;
  ContentsStorage(const ContentsStorage&) = delete;

  void init();

  ContentFileMeta& getCid(std::string cid);
  const std::unordered_map<std::string, ContentFileMeta>& getContentFileMetaMap() const;
  void shutdown();
  std::string getContentRootPath();

private:
  std::shared_ptr<Logger> logger;
  std::filesystem::path parent;
  std::unordered_map<std::string, ContentFileMeta> readers;
  std::string contentRootPath;
};

#endif //CONTENTSSTORAGE_H
