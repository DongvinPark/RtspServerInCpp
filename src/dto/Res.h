#pragma once
#ifndef RES_H
#define RES_H

#include <string>

class Res {
private:
    int key;
    std::string val;

public:
    explicit Res(int k, const std::string& v);
    int getKey() const;
    std::string getVal() const;
};

#endif // RES_H