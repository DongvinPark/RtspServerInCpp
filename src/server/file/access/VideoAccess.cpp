#include "../include/VideoAccess.h"

VideoAccess::VideoAccess() {}

VideoAccess::~VideoAccess() {
    close();
}

// Move constructor
VideoAccess::VideoAccess(VideoAccess&& other) noexcept
    : accesses(std::move(other.accesses)), meta(std::move(other.meta)) {
    // Ensure the moved-from object's resources are in a valid state
    other.accesses.clear();
    other.meta.clear();
}

// Move assignment
VideoAccess& VideoAccess::operator=(VideoAccess&& other) noexcept {
    if (this != &other) { // Prevent self-assignment
        close(); // Clean up current resources

        // Transfer ownership of resources
        accesses = std::move(other.accesses);
        meta = std::move(other.meta);

        // Ensure the moved-from object's resources are in a valid state
        other.accesses.clear();
        other.meta.clear();
    }
    return *this;
}

std::vector<std::ifstream>& VideoAccess::getAccessList() {
    return accesses;
}

std::vector<std::vector<VideoSampleInfo>>& VideoAccess::getVideoSampleInfoList() {
    return meta;
}

const std::vector<std::ifstream>& VideoAccess::getConstAccessList() const {
    return accesses;
}

const std::vector<std::vector<VideoSampleInfo>>& VideoAccess::getConstVideoSampleInfoList() const {
    return meta;
}

void VideoAccess::setMeta(const std::vector<std::vector<VideoSampleInfo>>& inputMeta) {
    meta = inputMeta;
}

int VideoAccess::getFileNumber() const {
    return meta.size();
}

void VideoAccess::close() {
    for (std::ifstream& access : accesses) {
        if (access.is_open()) {
            access.close();
        }
    }
}
