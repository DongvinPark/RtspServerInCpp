#include "../include/AudioAccess.h"

AudioAccess::AudioAccess() {}

AudioAccess::~AudioAccess() {
  close();
}

// fstream is impossible to copy. So returns reference instead.
std::ifstream & AudioAccess::getAccess() {
  return access;
}

// avoid copying
std::vector<AudioSampleInfo> & AudioAccess::getMeta() {
  return meta;
}

const std::ifstream & AudioAccess::getConstAccess() const {
  return access;
}

const std::vector<AudioSampleInfo> & AudioAccess::getConstMeta() const {
  return meta;
}

void AudioAccess::openAccessFileReadOnly(const std::string &inputFilePath) {
  // open file stream in read-only mode
  access = std::ifstream(inputFilePath, std::ios::in | std::ios::binary);
}

void AudioAccess::setMeta(const std::vector<AudioSampleInfo> &inputMeta) {
  meta = inputMeta;
}

void AudioAccess::close() {
  if (access.is_open()) {
    access.close();
  }
}
