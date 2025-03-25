#ifndef DATACONVERSION_H_
#define DATACONVERSION_H_

#include <iostream>
#include <string>
#include "xbsa/xbsa.h"
#include "xbsaclientcomm/BSAService.h"

class DataConversion {
public:

    DataConversion();
    ~DataConversion();

    static int CopyStrToChar(const std::string &src, char *dst, uint32_t dstSize); // string -> char*

    static void ConvertStrToTime(const std::string &src, struct tm &dst);

    static void ConvertObjectDescriptorIn(BSA_ObjectDescriptor *src, BsaObjectDescriptor &dst);

    static bool ConvertObjectDescriptorOut(BsaObjectDescriptor &src, BSA_ObjectDescriptor *dst);

    static void ConvertdataBlockOut(BsaDataBlock32 &src, BSA_DataBlock32 *dst);

    static void ConvertQueryObjectIn(BSA_QueryDescriptor *src, BsaQueryDescriptor &dst);

    static void ConvertdataBlockIn(BSA_DataBlock32 *src, BsaDataBlock32 &dst);

    static void U64ToBsaU64(unsigned long long u64, BsaUInt64 &b64);
};

#endif