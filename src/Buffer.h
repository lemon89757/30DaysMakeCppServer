#pragma once
#include <string>

class Buffer{
private:
    std::string buf;    // 也可以是 std::vector<char>，两者性能差异？
public:
    Buffer();
    ~Buffer();

    ssize_t size();
    const char* c_str();
    void clear();
    void getLine();
    void append(const char* _str, int size);
};