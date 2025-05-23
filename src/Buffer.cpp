#include <iostream>

#include "Buffer.h"

Buffer::Buffer(){

}

Buffer::~Buffer(){

}

void Buffer::append(const char* _str, int _size){
    for(int i = 0; i < _size; i++){
        if(_str[i] == '\0') break;
        buf.push_back(_str[i]);
    }
}

ssize_t Buffer::size(){
    return buf.size();
}

const char* Buffer::c_str(){
    return buf.c_str();
}

void Buffer::clear(){
    buf.clear();
}

void Buffer::getLine(){
    buf.clear();
    std::getline(std::cin, buf);
}

void Buffer::setBuf(const char* _buf){
    buf.clear();
    buf.append(_buf);
}