#include "res.h"

Res::Res(int k, const std::string& v) : key{ k }, val{ v } {}

int Res::getKey() const {
    return key;
}

std::string Res::getVal() const {
    return val;
}
