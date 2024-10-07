/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "DirtyRanges.h"
#include <iostream>
#include <utility>
#include <openssl/sha.h>
#ifdef WIN32
#include <zlib.h>
#else
#include "lz4.h"
#endif
#include "Constants.h"

namespace {
    const std::string MODULE_NAME = "DirtyRanges";
    const int DISPLAY_RANGE_INTERVAL_SIZE = 20;
    const int DECIMAL = 10;
    using Defer = std::shared_ptr<void>;
}

VIRT_PLUGIN_NAMESPACE_BEGIN

bool DirtyRanges::iterator::DecompressPart()
{
    auto compressedBuf = m_compressedParts[m_partIndex];
    int compressedSize = m_partOffsets[m_partIndex];
    size_t elementSize = sizeof(DirtyRange);
    size_t maxSize = DIRTY_RANGE_COMPRESS_SIZE * elementSize;
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(maxSize);
#ifdef WIN32
    uLong bufferLen = maxSize;
    int uncomRet = uncompress((Bytef*)(buffer.get()), &bufferLen, (const Bytef*)compressedBuf.get(), compressedSize);
    if (uncomRet != Z_OK) {
        ERRLOG("Uncompress failed, error code: %d, %d.", uncomRet, compressedSize);
        return false;
    }
    auto uncompressSize = bufferLen;
#else
    auto uncompressSize = (size_t) LZ4_decompress_safe(compressedBuf.get(), buffer.get(), compressedSize,
                                                       (int) maxSize);
#endif
    if (m_partIndex < m_compressedParts.size() - 1 && maxSize != uncompressSize) {
        // not the last parts,uncompressed size must equals max size
        ERRLOG("Uncompress dirty ranges failed. maxSize[%zu], uncompressSize[%zu]", maxSize, uncompressSize);
        m_partIndex++;
        return false;
    }
    for (int offset = 0; offset < uncompressSize;) {
        struct DirtyRange range;
        memcpy_s(&range, elementSize, buffer.get() + offset, elementSize);
        m_ranges->push_back(range);
        offset += elementSize;
    }
    INFOLOG("Uncompress dirty ranges success. m_partIndex[%zu], uncompressSize[%llu], compressedSize[%d]",
        m_partIndex, uncompressSize, compressedSize);
    m_partIndex++;
    return true;
}

void DirtyRanges::iterator::FindNextBlock()
{
    if (m_it == m_ranges->end() && m_partIndex < m_compressedParts.size()) {
        // load form next compressed part
        m_ranges->clear();
        (void) DecompressPart();
        m_it = m_ranges->begin();
    }

    uint64_t endOffset = m_block.EndOffset();
    for (; (m_it != m_ranges->end()) && (m_it->End() <= endOffset); ++m_it) {
    }
    if (m_it != m_ranges->end()) {
        ++(m_block.number);
        uint64_t startNumber = m_it->start / m_block.size;
        if (m_block.number < startNumber) {
            m_block.number = startNumber;
        }
    }
}

bool DirtyRanges::iterator::End()
{
    return m_partIndex == m_partOffsets.size() && m_it == m_ranges->end();
}

bool DirtyRanges::Initialize(const std::string &path, const std::string &taskID,
    std::shared_ptr<RepositoryHandler> &rangeFile)
{
    m_storagePath = path;
    m_taskID = taskID;
    m_rangeFile = rangeFile;
    return true;
}

DirtyRange DirtyRanges::Transform(const DirtyRange &range, const uint64_t blockSize)
{
    uint64_t blockNum = range.start / blockSize;
    uint64_t blockCount = ((range.End() + blockSize - 1) / blockSize) - blockNum;
    DirtyRange dirtyRange(blockNum * blockSize, blockCount * blockSize);
    return dirtyRange;
}

bool DirtyRanges::AddRange(const DirtyRange &range)
{
    auto rit = m_ranges->rbegin();
    // 首次添加
    if (rit == m_ranges->rend()) {
        DBGLOG("Add first range.");
        m_ranges->push_back(Transform(range, DIRTY_RANGE_BLOCK_SIZE));
        DBGLOG("start[%llu](%llu), size[%llu](%llu), end[%llu](%llu).",
            range.start, rit->start, range.size, rit->size, range.End(), rit->End());
        return true;
    }

    // 非法
    if (range.start <= rit->start) {
        ERRLOG("Ranges must be inserted in order. rit[%s], range[%s]",
            rit->toString().c_str(), range.toString().c_str());
        m_ranges->clear();
        return false;
    }

    DirtyRange tmp = Transform(range, DIRTY_RANGE_BLOCK_SIZE);
    if (tmp.End() > rit->End()) {
        // 不存在交叉
        if (tmp.start > rit->End()) {
            m_ranges->push_back(tmp);
        } else {
            // 存在交叉
            rit->size += (tmp.End() - rit->End());
        }
    }

    if (NeedCompressBuff()) {
        return CompressRanges();
    }
    return true;
}

std::string DirtyRanges::GetDirtyRangeFileName()
{
    return m_storagePath + Module::PATH_SEPARATOR + m_taskID + ".dirtyRanges";
}

bool DirtyRanges::SaveDirtyRanges()
{
    // create tmp buffer
    size_t partNum = m_partOffsets.size();
    size_t bufferSize = m_totalSize + sizeof(uint32_t) * (partNum + 1);
    std::shared_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(bufferSize);
    if (buffer == nullptr) {
        ERRLOG("Alloc memory failed. bufferSize=[%zu]", bufferSize);
        return false;
    }
    memset_s(buffer.get(), bufferSize, 0, bufferSize);
    // copy part offset size
    memcpy_s(buffer.get(), sizeof(uint32_t), &partNum, sizeof(uint32_t));
    size_t offset = sizeof(uint32_t);
    for (size_t i = 0; i < partNum; ++i) {
        memcpy_s(buffer.get() + offset, sizeof(uint32_t), &m_partOffsets[i], sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }
    // copy range buffer
    for (size_t i = 0; i < partNum; ++i) {
        memcpy_s(buffer.get() + offset, m_partOffsets[i], m_compressedParts[i].get(), m_partOffsets[i]);
        offset += m_partOffsets[i];
    }
    DBGLOG("Try to write dirty range. partNum=[%zu], bufferSize=[%zu]", partNum, bufferSize);
    return WriteToFile(buffer, bufferSize);
}

bool DirtyRanges::WriteToFile(std::shared_ptr<uint8_t[]> rangeBuffer, size_t bufferSize)
{
    std::string filename = GetDirtyRangeFileName();
    if (m_rangeFile == nullptr || m_storagePath.empty()) {
        ERRLOG("Range file ptr or storagePath is null.");
        return false;
    }
    Defer _(nullptr, [&](...) {
        if (m_rangeFile != nullptr) {
            m_rangeFile->Close();
        }
    });
    if (!m_rangeFile->Exists(m_storagePath)) {
        if (!m_rangeFile->CreateDirectory(m_storagePath)) {
            ERRLOG("Create dir error. dir=[%s]", m_storagePath.c_str());
            return false;
        }
    }
    if (m_rangeFile->Open(filename, "w+") != SUCCESS) {
        ERRLOG("Open file error. filename=[%s]", filename.c_str());
        return false;
    }
    // write sha256 and body to file
    size_t sizeToWrite = SHA256_DIGEST_LENGTH;
    std::shared_ptr<uint8_t[]> bufWrite = std::make_unique<uint8_t[]>(sizeToWrite);
    GetSha256(rangeBuffer.get(), bufferSize, bufWrite);
    size_t writeRet = m_rangeFile->Write(bufWrite, sizeToWrite);
    if (sizeToWrite != writeRet) {
        ERRLOG("Write file sha256 error. filename=[%s], writeRet=[%zu].", filename.c_str(), writeRet);
        return false;
    }
    if (m_rangeFile->Write(rangeBuffer, bufferSize) != bufferSize && m_rangeFile->Flush() != 0) {
        ERRLOG("Write file buffer error. filename=[%s], bufferSize[%zu]", filename.c_str(), bufferSize);
        return false;
    }
    INFOLOG("Write to file success. filename=[%s], bufferSize[%zu]", filename.c_str(), bufferSize);
    return true;
}

bool DirtyRanges::ReadFromFile(size_t bufferSize)
{
    std::shared_ptr<uint8_t[]> rangeBuffer = std::make_unique<uint8_t[]>(bufferSize);
    if (rangeBuffer == nullptr) {
        ERRLOG("Alloc memory failed. bufferSize[%zu]", bufferSize);
        return false;
    }
    size_t readSize = m_rangeFile->Read(rangeBuffer, bufferSize);
    if (readSize != bufferSize) {
        ERRLOG("Read file error. readSize[%zu]", readSize);
        return false;
    }
    // read and check sha256
    size_t sha256Size = SHA256_DIGEST_LENGTH;
    std::unique_ptr<uint8_t[]> sha256FromFile = std::make_unique<uint8_t[]>(sha256Size);
    if (readSize <= SHA256_DIGEST_LENGTH) {
        ERRLOG("File len error. read size[%zu]", readSize);
        return false;
    }
    memcpy_s(sha256FromFile.get(), sha256Size, rangeBuffer.get(), sha256Size);
    std::shared_ptr<uint8_t[]> hash256Calc = std::make_unique<uint8_t[]>(sha256Size);
    GetSha256(rangeBuffer.get() + sha256Size, bufferSize - sha256Size, hash256Calc);
    if (memcmp(hash256Calc.get(), sha256FromFile.get(), sha256Size)) {
        ERRLOG("File sha256 not matched.");
        return false;
    }
    // copy part size
    size_t offset = sha256Size;
    uint32_t partSize = 0;
    memcpy_s(&partSize, sizeof(uint32_t), rangeBuffer.get() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    size_t elementSize = sizeof(uint32_t);
    for (int i = 0; i < partSize && offset < bufferSize; ++i) {
        uint32_t partOffset = 0;
        memcpy_s(&partOffset, elementSize, rangeBuffer.get() + offset, elementSize);
        m_partOffsets.push_back(partOffset);
        offset += elementSize;
    }
    // copy compressed parts
    for (int i = 0; i < partSize && (offset + m_partOffsets[i] <= bufferSize); ++i) {
        uint32_t compressBufSize = m_partOffsets[i];
        std::shared_ptr<char[]> tmp = std::make_unique<char[]>(compressBufSize);
        memcpy_s(tmp.get(), compressBufSize, rangeBuffer.get() + offset, compressBufSize);
        m_compressedParts.push_back(tmp);
        offset += m_partOffsets[i];
    }
    DBGLOG("Read success. partSize[%lu]", partSize);
    return true;
}

bool DirtyRanges::CleanDirtyRanges()
{
    std::string fileName = GetDirtyRangeFileName();
    if (m_rangeFile == nullptr) {
        ERRLOG("Range file ptr is null.");
        return false;
    }
    return m_rangeFile->Remove(fileName);
}

uint64_t DirtyRanges::GetBlockNum(uint64_t blockSize)
{
    uint64_t blockNum = 0;
    DirtyRanges::iterator it = DirtyRanges::iterator(blockSize, m_partOffsets, m_compressedParts);
    for (; !it.End(); ++it) {
        ++blockNum;
    }
    return blockNum;
}

bool DirtyRanges::NeedCompressBuff()
{
    return m_ranges->size() >= DIRTY_RANGE_COMPRESS_SIZE;
}

bool DirtyRanges::CompressRanges()
{
    // set buffer from ranges
    size_t bufferSize = m_ranges->size() * sizeof(DirtyRange);
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(bufferSize);
    if (buffer == nullptr) {
        ERRLOG("Alloc memory failed. bufferSize=[%zu]", bufferSize);
        return false;
    }
    memset_s(buffer.get(), bufferSize, 0, bufferSize);
    auto iter = m_ranges->begin();
    size_t offset = 0;
    while (iter != m_ranges->end() && offset <= bufferSize) {
        memcpy_s(buffer.get() + offset, sizeof(DirtyRange), &(*iter), sizeof(DirtyRange));
        iter++;
        offset += sizeof(DirtyRange);
    }

    // compress it
    std::size_t maxCompressSize =
#ifndef WIN32
        LZ4_compressBound(bufferSize);
#else
        compressBound(bufferSize);
#endif
    std::unique_ptr<char[]> compressed = std::make_unique<char[]>(maxCompressSize);
    if (nullptr == compressed) {
        ERRLOG("Alloc memory for LZ4_compress failed.");
        return false;
    }
    memset_s(compressed.get(), maxCompressSize, 0, maxCompressSize);
#ifdef WIN32
    uLong compressSize = maxCompressSize;
    int ret = compress((Bytef*)(compressed.get()), &compressSize, (const Bytef*)buffer.get(), bufferSize);
    if (ret != Z_OK) {
        ERRLOG("Compress failed.");
        return false;
    }
    std::size_t compressBufSize = compressSize;
#else
    std::size_t compressBufSize = LZ4_compress_default(buffer.get(), compressed.get(), bufferSize, maxCompressSize);
#endif
    // add to compressed parts and reset ranges
    std::shared_ptr<char[]> tmp = std::make_unique<char[]>(compressBufSize);
    memcpy_s(tmp.get(), compressBufSize, compressed.get(), compressBufSize);
    m_compressedParts.push_back(tmp);
    m_partOffsets.push_back(compressBufSize);
    m_totalSize += compressBufSize;
    m_ranges->clear();
    INFOLOG("Compress dirty ranges success. compressBufSize[%zu], bufferSize[%zu], m_totalSize[%lu].",
        compressBufSize, bufferSize, m_totalSize);
    return true;
}

bool DirtyRanges::GetSha256(const void *inBuf, std::size_t size, std::shared_ptr<uint8_t[]> outBuf)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, inBuf, size);
    SHA256_Final(hash, &sha256);
    memcpy_s(outBuf.get(), SHA256_DIGEST_LENGTH, hash, SHA256_DIGEST_LENGTH);
    return true;
}

std::string DirtyRanges::Serialize() const
{
    Json::Value val;
    val["DirtyRanges"]["m_isFull"] = m_isFull;
    val["DirtyRanges"]["m_isRestore"] = m_isRestore;
    val["DirtyRanges"]["m_isForDedupe"] = m_isForDedupe;
    // Convert List to String, format is: start1:size1,start2:size2,......startN:sizeN
    std::ostringstream oss;
    auto it = m_ranges->begin();
    for (; it != m_ranges->end(); ++it) {
        if (it != m_ranges->begin()) {
            // use comma to separate two elements,the last one does not need comma
            oss << ",";
        }
        oss << it->start << ":" << it->size;
    }
    val["DirtyRanges"]["m_ranges"] = oss.str();
    Json::FastWriter fastWriter;
    return fastWriter.write(val);
}

bool DirtyRanges::Deserialize(const Json::Value &val)
{
    m_isFull = val["DirtyRanges"]["m_isFull"].asBool();
    m_isRestore = val["DirtyRanges"]["m_isRestore"].asBool();
    m_isForDedupe = val["DirtyRanges"].isMember("m_isForDedupe") ? val["DirtyRanges"]["m_isForDedupe"].asBool() : false;
    std::string strRanges = val["DirtyRanges"]["m_ranges"].asString();
    if (strRanges.empty()) {
        WARNLOG("Ranges is empty. it is ok.");
        return true;
    }
    m_ranges->clear();
    if (!String2DirtyRanges(strRanges)) {
        ERRLOG("String2DirtyRanges failed.");
        return false;
    }
    return true;
}

bool DirtyRanges::String2DirtyRanges(const std::string &ranges)
{
    if (ranges.empty()) {
        WARNLOG("Ranges is empty. it is ok.");
        return true;
    }
    std::string rangeElement;
    std::string::size_type commaPos = 0;
    std::string::size_type prePos = 0;
    while ((commaPos = ranges.find_first_of(',', commaPos)) != std::string::npos) {
        // process the elements that used comma to separate
        rangeElement = ranges.substr(prePos, commaPos - prePos);
        if (!ParserRange(rangeElement)) {
            return false;
        }
        prePos = ++commaPos;
    }

    // process the last one element
    rangeElement = ranges.substr(prePos, commaPos - prePos);
    return ParserRange(rangeElement);
}

bool DirtyRanges::ParserRange(const std::string &strRange)
{
    std::string::size_type colonPos = 0;
    if ((colonPos = strRange.find(':')) == std::string::npos) {
        ERRLOG("Range str format is invalid, no colon in %s.", strRange.c_str());
        return false;
    }
    uint64_t startAddr = std::strtoull(strRange.substr(0, colonPos).c_str(), nullptr, DECIMAL);
    uint64_t offset = std::strtoull(strRange.substr(colonPos + 1).c_str(), nullptr, DECIMAL);
    m_ranges->push_back(DirtyRange(startAddr, offset));
    return true;
}

bool DirtyRanges::FlushToStorage()
{
    return CompressRanges() && SaveDirtyRanges();
}

bool DirtyRanges::LoadFromStorage()
{
    std::string fileName = GetDirtyRangeFileName();
    if (m_rangeFile == nullptr) {
        ERRLOG("Range file ptr is null.");
        return false;
    }
    Defer _(nullptr, [&](...) {
        if (m_rangeFile != nullptr) {
            m_rangeFile->Close();
        }
    });
    clear();
    if (m_rangeFile->Open(fileName, "r") != SUCCESS) {
        ERRLOG("Open file error. fileName=[%s]", fileName.c_str());
        return false;
    }
    size_t size = m_rangeFile->FileSize(fileName);
    if (size <= 0) {
        ERRLOG("Get size failed. fileName=[%s], size=[%zu]", fileName.c_str(), size);
        return false;
    }
    if (!ReadFromFile(size)) {
        ERRLOG("Read file failed. fileName=[%s], size=[%zu]", fileName.c_str(), size);
        return false;
    }
    INFOLOG("Load dirty range success. fileName=[%s].", fileName.c_str());
    return true;
}

void DirtyRanges::Display()
{
    // short info
    if (m_ranges->size() <= DISPLAY_RANGE_INTERVAL_SIZE) {
        DBGLOG("DirtyRanges display is short.[%s]", this->Serialize().c_str());
        return;
    }

    DBGLOG("DirtyRanges: m_isFull[%d], m_isRestore[%d], m_isForDedupe[%d], m_ranges size[%zu]",
        m_isFull, m_isRestore, m_isForDedupe, m_ranges->size());
    DBGLOG("DirtyRange:{");
    std::size_t index = 0;
    std::ostringstream oss;
    std::list<DirtyRange>::const_iterator it = m_ranges->begin();
    for (; it != m_ranges->end(); it++) {
        oss << it->start << ":" << it->size << ",";
        index++;
        if (index % DISPLAY_RANGE_INTERVAL_SIZE == 0) {
            DBGLOG("%s", WIPE_SENSITIVE(oss.str()).c_str());
            // clear
            oss.str("");
            index = 0;
        }
    }
    DBGLOG("%s }", WIPE_SENSITIVE(oss.str()).c_str());
}

VIRT_PLUGIN_NAMESPACE_END
