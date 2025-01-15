#include "../include/AcsHandler.h"

AcsHandler::AcsHandler(
  std::string sessionId,
  std::weak_ptr<Session> parentSessionPtr,
  ContentsStorage& inputContentsStorage
) {}

AcsHandler::~AcsHandler() {}

void AcsHandler::updateRtpRemoteCnt(int cnt) {
}

void AcsHandler::updateCurSampleNo(int mediaType, int idx) {
}

int AcsHandler::getCamId() {
}

void AcsHandler::shutdown() {
}

std::weak_ptr<FileReader> AcsHandler::getFileReaderPtr() {
}

void AcsHandler::setChannel(int streamId, std::vector<int> ch) {
}

void AcsHandler::initUserRequestingPlaytime(std::vector<float> timeS) {
}

void AcsHandler::setRtpInfo(RtpInfo &rtpInfo) {
}

void AcsHandler::setReader(std::weak_ptr<FileReader> reader) {
}

int AcsHandler::getLastVideoSampleNumber() {
}

int AcsHandler::getLastAudioSampleNumber() {
}

std::vector<unsigned char> AcsHandler::getAccData() {
}

std::vector<std::vector<unsigned char>> AcsHandler::getAllV0Images() {
}

std::unique_ptr<AVSampleBuffer> AcsHandler::getNextSample() {
}

bool AcsHandler::isDone(int streamId) {
}

int64_t AcsHandler::getUnitFrameTimeUs(int streamId) {
}

std::string AcsHandler::getMediaInfo() {
}

std::vector<int64_t> AcsHandler::getSsrc() {
}

int AcsHandler::getMainVideoNumber() {
}

int AcsHandler::getMaxCamNumber() {
}

std::vector<int> AcsHandler::getInitialSeq() {
}

std::vector<long> AcsHandler::getTimestamp() {
}

int64_t AcsHandler::getTimestamp0(int streamId) {
}

int64_t AcsHandler::getUnitFrameCount(int streamId) {
}

std::vector<int64_t> AcsHandler::getUnitFrameCount() {
}

std::vector<std::string> AcsHandler::getStreamUrls() {
}

void AcsHandler::setStreamUrl(int streamId, std::string url) {
}

std::vector<int64_t> AcsHandler::getPlayTimeUs() {
}

int64_t AcsHandler::getPlayTimeUs(int streamId) {
}

std::vector<int64_t> AcsHandler::getGop() {
}

void AcsHandler::setCamId(int camId) {
}

void AcsHandler::findNextSampleForSwitching(int vid, std::vector<int64_t> timeInfo) {
}

std::unique_ptr<Buffer> AcsHandler::get1stRtpOfRefSample(int streamId, int sampleNo) {
}

void AcsHandler::checkTimestamp(int streamId, ReadInfo &readInfo) {
}

int AcsHandler::findKeySampleNumber(int streamId, int64_t timeUs, int way) {
}

int AcsHandler::findSampleNumber(int streamId, int64_t timeUs) {
}

int AcsHandler::getSampleNumber(int streamId, int64_t timeUs) {
}

int AcsHandler::findNextSampleForSwitchingAudio(std::vector<int64_t> timeInfo) {
}

void AcsHandler::findNextSampleForSwitchingVideo(int nextVid, std::vector<int64_t> timeInfo) {
}

std::vector<int64_t> AcsHandler::getUnitFrameTimeUs() {
}

int64_t AcsHandler::getSamplePresentationTimeUs(int streamId, int64_t timestamp) {
}

int64_t AcsHandler::getSamplePresentationTimeUs(int streamId, int sampleTimeIndex) {
}

int AcsHandler::getSampleTimeIndex(int streamId, int64_t timestamp) {
}

int64_t AcsHandler::getTimestamp(int sampleNo) {
}
