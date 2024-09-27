#ifndef _CODECONVERT_H_
#define _CODECONVERT_H_

#include <string>

class CodeConvertInterface {
public:
    virtual ~CodeConvertInterface() = 0;
};
 
class CodeConvert : public CodeConvertInterface {
public:
    static bool EncodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
    static bool DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
};

#endif
