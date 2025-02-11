#include "../include/AudioAccess.h"

AudioAccess::AudioAccess() {}

AudioAccess::~AudioAccess() {
  close();
}

AudioAccess::AudioAccess(AudioAccess &&other) noexcept
  : access(std::move(other.access)), meta(std::move(other.meta)) {
  other.close();
  other.meta.clear();
}

AudioAccess & AudioAccess::operator=(AudioAccess &&other) noexcept {
  if (this != &other) { // ban self assignment
    close();
    access = std::move(other.access);
    meta = std::move(other.meta);
    other.meta.clear();
  }
  return *this;
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

void AudioAccess::setMeta(const std::vector<AudioSampleInfo> &inputMeta) {
  meta = inputMeta;
}

void AudioAccess::close() {
  if (access.is_open()) {
    access.close();
  }
}
