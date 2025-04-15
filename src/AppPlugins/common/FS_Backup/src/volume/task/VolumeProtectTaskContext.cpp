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
#include "log/Log.h"
#include "common/VolumeProtectMacros.h"
#include "native/FileSystemAPI.h"
#include "VolumeProtector.h"
#include "VolumeBlockReader.h"
#include "VolumeBlockWriter.h"
#include "VolumeBlockHasher.h"
#include "VolumeProtectTaskContext.h"

using namespace volumeprotect;
using namespace volumeprotect::task;

namespace {
    constexpr uint64_t NUM2 = 2;
    constexpr uint32_t BITS_PER_UINT8 = 8;
    constexpr uint32_t BITMAP_RSHIFT = 3; // 2^3 = 8
}

// implement VolumeBlockAllocator...

VolumeBlockAllocator::VolumeBlockAllocator(uint32_t blockSize, uint32_t blockNum)
    : m_pool(nullptr), m_allocTable(nullptr), m_blockSize(blockSize), m_blockNum(blockNum)
{
    m_pool = new uint8_t[blockSize * blockNum];
    m_allocTable = new bool[blockNum];
    memset_s(m_allocTable, blockNum * sizeof(bool), 0, blockNum * sizeof(bool));
}

VolumeBlockAllocator::~VolumeBlockAllocator()
{
    if (m_pool) {
        delete [] m_pool;
        m_pool = nullptr;
    }
    if (m_allocTable) {
        delete [] m_allocTable;
        m_allocTable = nullptr;
    }
}

uint8_t* VolumeBlockAllocator::Bmalloc()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    for (int i = 0; i < static_cast<int>(m_blockNum); i++) {
        if (!m_allocTable[i]) {
            m_allocTable[i] = true;
            uint8_t* ptr = m_pool + (m_blockSize * i);
            DBGLOG("Bmalloc index = %d, address = %p", i, ptr);
            return ptr;
        }
    }
    return nullptr;
}

void VolumeBlockAllocator::Bfree(uint8_t* ptr)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    uint64_t index = (ptr - m_pool) / static_cast<uint64_t>(m_blockSize);
    DBGLOG("Bfree address = %p, index = %llu", ptr, index);
    if ((ptr - m_pool) % m_blockSize == 0) {
        m_allocTable[index] = false;
        return;
    }
    // reach err here
    throw std::runtime_error("Bfree error: bad address");
}

// implement BlockHashingContext...

BlockHashingContext::BlockHashingContext(uint64_t pSize, uint64_t lSize)
    : lastestSize(lSize), previousSize(pSize)
{
    lastestTable = new uint8_t[lSize];
    previousTable = new uint8_t[pSize];
    memset_s(lastestTable, sizeof(uint8_t) * lSize, 0, sizeof(uint8_t) * lSize);
    memset_s(previousTable, sizeof(uint8_t) * pSize, 0, sizeof(uint8_t) * pSize);
}

BlockHashingContext::BlockHashingContext(uint64_t lSize)
    : lastestSize(lSize), previousSize(0), previousTable(nullptr)
{
    lastestTable = new uint8_t[lSize];
    memset_s(lastestTable, sizeof(uint8_t) * lSize, 0, sizeof(uint8_t) * lSize);
}

BlockHashingContext::~BlockHashingContext()
{
    if (lastestTable != nullptr) {
        delete[] lastestTable;
        lastestTable = nullptr;
    }
    if (previousTable == nullptr) {
        delete[] previousTable;
        previousTable = nullptr;
    }
}

// implement Bitmap...

Bitmap::Bitmap(uint64_t size)
{
    m_capacity = size / BITS_PER_UINT8 + 1;
    m_table = new uint8_t[m_capacity]; // avoid using make_unique (requiring CXX14)
    memset_s(m_table, sizeof(uint8_t) * m_capacity, 0, sizeof(uint8_t) * m_capacity);
}

Bitmap::Bitmap(uint8_t* ptr, uint64_t capacity)
    : m_capacity(capacity), m_table(ptr)
{}

Bitmap::~Bitmap()
{
    if (m_table != nullptr) {
        delete[] m_table;
        m_table = nullptr;
    }
}

void Bitmap::Set(uint64_t index)
{
    if (index >= m_capacity * BITS_PER_UINT8) { // illegal argument
        return;
    }
    m_table[index >> BITMAP_RSHIFT] |= (((uint8_t)1) << (index % BITS_PER_UINT8));
}

bool Bitmap::Test(uint64_t index) const
{
    if (index >= m_capacity * BITS_PER_UINT8) { // illegal argument
        return false;
    }
    return (m_table[index >> BITMAP_RSHIFT] & (((uint8_t)1) << (index % BITS_PER_UINT8))) != 0;
}

uint64_t Bitmap::FirstIndexUnset() const
{
    for (uint64_t index = 0; index < m_capacity * BITS_PER_UINT8; ++index) {
        if (!Test(index)) {
            return index;
        }
    }
    return MaxIndex() + 1;
}

uint64_t Bitmap::Capacity() const
{
    return m_capacity;
}

uint64_t Bitmap::MaxIndex() const
{
    return m_capacity * BITS_PER_UINT8 - 1;
}

uint64_t Bitmap::TotalSetCount() const
{
    uint64_t totalSetCount = 0;
    for (uint64_t index = 0; index < m_capacity * BITS_PER_UINT8; ++index) {
        totalSetCount += Test(index) ? 1 : 0;
    }
    return totalSetCount;
}

const uint8_t* Bitmap::Ptr() const
{
    return m_table;
}

// implement VolumeTaskSession...

uint64_t VolumeTaskSession::MaxIndex() const
{
    return TotalBlocks() - 1; // index start from zero
}

uint64_t VolumeTaskSession::TotalBlocks() const
{
    uint64_t numBlocks = sharedConfig->sessionSize / sharedConfig->blockSize;
    if (sharedConfig->sessionSize % sharedConfig->blockSize != 0) {
        numBlocks++;
    }
    return numBlocks;
}

bool VolumeTaskSession::IsTerminated() const
{
    DBGLOG("check session terminated, readerTask: %d, hasherTask: %d, writerTask: %d",
        readerTask == nullptr ? TaskStatus::SUCCEED : readerTask->GetStatus(),
        hasherTask == nullptr ? TaskStatus::SUCCEED : hasherTask->GetStatus(),
        writerTask == nullptr ? TaskStatus::SUCCEED : writerTask->GetStatus());
    return (
        (readerTask == nullptr || readerTask->IsTerminated()) &&
        (hasherTask == nullptr || hasherTask->IsTerminated()) &&
        (writerTask == nullptr || writerTask->IsTerminated()));
}

bool VolumeTaskSession::IsFailed() const
{
    DBGLOG("check session failed, readerTask: %d, hasherTask: %d, writerTask: %d",
        readerTask == nullptr ? TaskStatus::SUCCEED : readerTask->GetStatus(),
        hasherTask == nullptr ? TaskStatus::SUCCEED : hasherTask->GetStatus(),
        writerTask == nullptr ? TaskStatus::SUCCEED : writerTask->GetStatus());
    return (
        (readerTask != nullptr && readerTask->IsFailed()) ||
        (hasherTask != nullptr && hasherTask->IsFailed()) ||
        (writerTask != nullptr && writerTask->IsFailed()));
}

void VolumeTaskSession::Abort() const
{
    if (readerTask != nullptr) {
        readerTask->Abort();
    }
    if (hasherTask != nullptr) {
        hasherTask->Abort();
    }
    if (writerTask != nullptr) {
        writerTask->Abort();
    }
}

ErrCodeType VolumeTaskSession::GetErrorCode() const
{
    // we consider reader failure to be the root case of the session failure
    if (readerTask != nullptr && readerTask->GetErrorCode() != VOLUMEPROTECT_ERR_SUCCESS) {
        return readerTask->GetErrorCode();
    }
    // if only writer failed, set session error code to the writer error code
    if (writerTask != nullptr && writerTask->GetErrorCode() != VOLUMEPROTECT_ERR_SUCCESS) {
        return writerTask->GetErrorCode();
    }
    return VOLUMEPROTECT_ERR_SUCCESS;
}

// implement TaskStatisticTrait ...

void TaskStatisticTrait::UpdateRunningSessionStatistics(std::shared_ptr<VolumeTaskSession> session)
{
    std::lock_guard<std::mutex> lock(m_statisticMutex);
    auto counter = session->sharedContext->counter;
    DBGLOG("UpdateRunningSessionStatistics: bytesToReaded: %llu, bytesRead: %llu, "
        "blocksToHash: %llu, blocksHashed: %llu, "
        "bytesToWrite: %llu, bytesWritten: %llu",
        counter->bytesToRead.load(), counter->bytesRead.load(),
        counter->blocksToHash.load(), counter->blocksHashed.load(),
        counter->bytesToWrite.load(), counter->bytesWritten.load());
    m_currentSessionStatistics.bytesToRead = counter->bytesToRead;
    m_currentSessionStatistics.bytesRead = counter->bytesRead;
    m_currentSessionStatistics.blocksToHash = counter->blocksToHash;
    m_currentSessionStatistics.blocksHashed = counter->blocksHashed;
    m_currentSessionStatistics.bytesToWrite = counter->bytesToWrite;
    m_currentSessionStatistics.bytesWritten = counter->bytesWritten;
}

void TaskStatisticTrait::UpdateCompletedSessionStatistics(std::shared_ptr<VolumeTaskSession> session)
{
    std::lock_guard<std::mutex> lock(m_statisticMutex);
    auto counter = session->sharedContext->counter;
    DBGLOG("UpdateCompletedSessionStatistics: bytesToReaded: %llu, bytesRead: %llu, "
        "blocksToHash: %llu, blocksHashed: %llu, "
        "bytesToWrite: %llu, bytesWritten: %llu",
        counter->bytesToRead.load(), counter->bytesRead.load(),
        counter->blocksToHash.load(), counter->blocksHashed.load(),
        counter->bytesToWrite.load(), counter->bytesWritten.load());
    m_completedSessionStatistics.bytesToRead += counter->bytesToRead;
    m_completedSessionStatistics.bytesRead += counter->bytesRead;
    m_completedSessionStatistics.blocksToHash += counter->blocksToHash;
    m_completedSessionStatistics.blocksHashed += counter->blocksHashed;
    m_completedSessionStatistics.bytesToWrite += counter->bytesToWrite;
    m_completedSessionStatistics.bytesWritten += counter->bytesWritten;
    memset_s(&m_currentSessionStatistics, sizeof(TaskStatistics), 0, sizeof(TaskStatistics));
}


// implement BackupTaskCheckpoint
CheckpointSnapshot::CheckpointSnapshot(uint64_t length)
    : bitmapBufferBytesLength(length)
{
    processedBitmapBuffer = new uint8_t[length];
    writtenBitmapBuffer = new uint8_t[length];
    memset_s(processedBitmapBuffer, sizeof(uint8_t) * length, 0, sizeof(uint8_t) * length);
    memset_s(writtenBitmapBuffer, sizeof(uint8_t) * length, 0, sizeof(uint8_t) * length);
}

CheckpointSnapshot::~CheckpointSnapshot()
{
    if (processedBitmapBuffer != nullptr) {
        delete[] processedBitmapBuffer;
        processedBitmapBuffer = nullptr;
    }
    if (writtenBitmapBuffer != nullptr) {
        delete[] writtenBitmapBuffer;
        writtenBitmapBuffer = nullptr;
    }
}

std::shared_ptr<CheckpointSnapshot> CheckpointSnapshot::LoadFrom(const std::string& filepath)
{
    uint64_t totalSize = fsapi::GetFileSize(filepath);
    if (totalSize == 0 || totalSize % NUM2 != 0) {
        ERRLOG("size of checkpoint snapshot file should be divided by 3!, length = %", totalSize);
        return nullptr;
    }
    uint64_t bitmapBytes = totalSize / NUM2;
    uint8_t* buffer = fsapi::ReadBinaryBuffer(filepath, totalSize);
    if (buffer == nullptr) {
        ERRLOG("failed to read checkpoint snapshot file, path: %s", filepath.c_str());
        return nullptr;
    }
    auto checkpointSnapshot = std::make_shared<CheckpointSnapshot>(bitmapBytes);
    uint64_t offset = 0;
    int dummy = memcpy_s(
        checkpointSnapshot->processedBitmapBuffer,
        sizeof(uint8_t) * bitmapBytes,
        buffer + offset,
        sizeof(uint8_t) * bitmapBytes);
    offset += bitmapBytes;
    dummy = memcpy_s(
        checkpointSnapshot->writtenBitmapBuffer,
        sizeof(uint8_t) * bitmapBytes,
        buffer + offset,
        sizeof(uint8_t) * bitmapBytes);
    delete[] buffer;
    (void)(dummy == 0 ? dummy : 0);
    return checkpointSnapshot;
}

bool CheckpointSnapshot::SaveTo(const std::string& filepath) const
{
    uint64_t totalSize = NUM2 * bitmapBufferBytesLength;
    uint8_t* buffer = new uint8_t[totalSize];
    uint64_t offset = 0;
    int dummy = memcpy_s(buffer + offset, bitmapBufferBytesLength, processedBitmapBuffer, bitmapBufferBytesLength);
    offset += bitmapBufferBytesLength;
    dummy = memcpy_s(buffer + offset, bitmapBufferBytesLength, writtenBitmapBuffer, bitmapBufferBytesLength);
    bool ret = fsapi::WriteBinaryBuffer(filepath, buffer, totalSize);
    delete[] buffer;
    buffer = nullptr;
    (void)(dummy == 0 ? dummy : 0);
    return ret;
}


// implement VolumeTaskCheckpointTrait

// any session started (completed/crashed and restarted) is considered "Restarted"
bool VolumeTaskCheckpointTrait::IsSessionRestarted(std::shared_ptr<VolumeTaskSession> session) const
{
    return fsapi::IsFileExists(session->sharedConfig->checkpointFilePath);
}

bool VolumeTaskCheckpointTrait::IsCheckpointEnabled(std::shared_ptr<VolumeTaskSession> session) const
{
    return session->sharedConfig->checkpointEnabled;
}

void VolumeTaskCheckpointTrait::InitSessionBitmap(std::shared_ptr<VolumeTaskSession> session) const
{
    uint64_t numBlocks = session->TotalBlocks();
    session->sharedContext->processedBitmap = std::make_shared<Bitmap>(numBlocks);
    session->sharedContext->writtenBitmap = std::make_shared<Bitmap>(numBlocks);
}

std::shared_ptr<CheckpointSnapshot> VolumeTaskCheckpointTrait::TakeSessionCheckpointSnapshot(
    std::shared_ptr<VolumeTaskSession> session) const
{
    auto sharedContext = session->sharedContext;
    assert(sharedContext->writtenBitmap->Capacity() == sharedContext->processedBitmap->Capacity());
    uint64_t length = sharedContext->writtenBitmap->Capacity();
    auto checkpointSnapshot = std::make_shared<CheckpointSnapshot>(length);
    int dummy = memcpy_s(
        checkpointSnapshot->processedBitmapBuffer, length, sharedContext->processedBitmap->Ptr(), length);
    dummy = memcpy_s(
        checkpointSnapshot->writtenBitmapBuffer, length, sharedContext->writtenBitmap->Ptr(), length);
    (void)(dummy == 0 ? dummy : 0);
    return checkpointSnapshot;
}

// Save sesion...
void VolumeTaskCheckpointTrait::RefreshSessionCheckpoint(std::shared_ptr<VolumeTaskSession> session)
{
    if (!IsCheckpointEnabled(session) || !CheckLastUpdateTimer()) {
        return;
    }
    session->readerTask->Pause();
    std::shared_ptr<void> defer(nullptr, [&](...) {
        session->readerTask->Resume();
    });
    auto checkpointSnapshot = TakeSessionCheckpointSnapshot(session);
    // only should work during backup with hasher enabled,
    // hashing checksum must be saved before writer bitmap
    if (session->sharedConfig->hasherEnabled && !FlushSessionLatestHashingTable(session)) {
        ERRLOG("failed to flush latest hashing table, cannot refresh checkpoint");
        return;
    }
    if (!FlushSessionWriter(session)) {
        ERRLOG("failed to flush writer, cannot refresh checkpoint");
        return;
    }
    if (!FlushSessionBitmap(session)) {
        ERRLOG("failed to flush sessionn bitmap");
        return;
    }
    std::string checkpointFilePath = session->sharedConfig->checkpointFilePath;
    if (!checkpointSnapshot->SaveTo(checkpointFilePath)) {
        ERRLOG("failed to save checkpoint snapshot file to %s", checkpointFilePath.c_str());
        return;
    }
    DBGLOG("checkpoint snapshot saved to %s success", checkpointFilePath.c_str());
}

bool VolumeTaskCheckpointTrait::FlushSessionLatestHashingTable(std::shared_ptr<VolumeTaskSession> session) const
{
    uint8_t* latestChecksumTable = session->sharedContext->hashingContext->lastestTable;
    uint64_t latestChecksumTableSize = session->sharedContext->hashingContext->lastestSize;
    if (session->sharedConfig->hasherEnabled && latestChecksumTable != nullptr) {
        std::string filepath = session->sharedConfig->lastestChecksumBinPath;
        DBGLOG("save latest hash checksum table to %s, size = %llu", filepath.c_str(), latestChecksumTableSize);
        if (!fsapi::WriteBinaryBuffer(filepath, latestChecksumTable, latestChecksumTableSize)) {
            ERRLOG("failed to save session hashing context");
            return false;
        }
    }
    return true;
}

bool VolumeTaskCheckpointTrait::FlushSessionWriter(std::shared_ptr<VolumeTaskSession> session) const
{
    return session->writerTask->Flush();
}

bool VolumeTaskCheckpointTrait::FlushSessionBitmap(SessionPtr session) const
{
    if (!session->sharedConfig->checkpointEnabled) {
        return true;
    }
    auto checkpointSnapshot = TakeSessionCheckpointSnapshot(session);
    std::string checkpointFilePath = session->sharedConfig->checkpointFilePath;
    if (!checkpointSnapshot->SaveTo(checkpointFilePath)) {
        ERRLOG("failed to save bitmap file to %s", checkpointFilePath.c_str());
        return false;
    }
    return true;
}

// Restore section ...

void VolumeTaskCheckpointTrait::RestoreSessionCheckpoint(std::shared_ptr<VolumeTaskSession> session) const
{
    if (!IsSessionRestarted(session) || !IsCheckpointEnabled(session)) {
        return;
    }
    // only should work during backup with hasher enabled, restore both checksum and bitmap
    if (session->sharedConfig->hasherEnabled && !RestoreSessionLatestHashingTable(session)) {
        // if checksum restore failed, session must be restarted from beginning
        ERRLOG("failed to retore latest checksum table from checkpoint, start session from beginning");
        return;
    }
    // restore three bitmap
    if (!RestoreSessionBitmap(session)) {
        ERRLOG("failed to restore session bitmap from checkpoint, start session from beginning");
        return;
    }
    // restore session counter from bitmap
    RestoreSessionCounter(session);
    DBGLOG("restore task from checkpoint success");
}

bool VolumeTaskCheckpointTrait::RestoreSessionLatestHashingTable(std::shared_ptr<VolumeTaskSession> session) const
{
    if (!IsSessionRestarted(session) || !IsCheckpointEnabled(session)) {
        DBGLOG("session didn't restarted, use inited latest checksum table");
        return false;
    }
    std::string lastestChecksumBinPath = session->sharedConfig->lastestChecksumBinPath;
    if (!ReadLatestHashingTable(session)) {
        return false;
    }
    DBGLOG("restore lastest checksum table from %s success", lastestChecksumBinPath.c_str());
    return true;
}

// try to restore session writer bitmap
bool VolumeTaskCheckpointTrait::RestoreSessionBitmap(std::shared_ptr<VolumeTaskSession> session) const
{
    std::string checkpointFilePath = session->sharedConfig->checkpointFilePath;
    std::shared_ptr<CheckpointSnapshot> checkpointSnapshot = ReadCheckpointSnapshot(session);
    if (checkpointSnapshot == nullptr) {
        ERRLOG("failed to read checkpoint snapshot from %s", checkpointFilePath.c_str());
        return false;
    }
    uint64_t bitmapBytesRequired = session->sharedContext->writtenBitmap->Capacity();
    if (checkpointSnapshot->bitmapBufferBytesLength != bitmapBytesRequired) {
        ERRLOG("failed to restore bitmap from checkpoint, require size %llu bytes, actual %llu bytes",
            bitmapBytesRequired, checkpointSnapshot->bitmapBufferBytesLength);
        checkpointSnapshot.reset();
        return false;
    }
    session->sharedContext->processedBitmap = std::make_shared<Bitmap>(
        checkpointSnapshot->processedBitmapBuffer, checkpointSnapshot->bitmapBufferBytesLength);
    session->sharedContext->writtenBitmap = std::make_shared<Bitmap>(
        checkpointSnapshot->writtenBitmapBuffer, checkpointSnapshot->bitmapBufferBytesLength);
    checkpointSnapshot->writtenBitmapBuffer = nullptr;
    checkpointSnapshot->processedBitmapBuffer = nullptr;
    DBGLOG("restore session bitmap from %s success", checkpointFilePath.c_str());
    return true;
}

// restore session counter from writer bitmap
void VolumeTaskCheckpointTrait::RestoreSessionCounter(std::shared_ptr<VolumeTaskSession> session) const
{
    auto counter = session->sharedContext->counter;
    uint64_t sessionSize = session->sharedConfig->sessionSize;
    uint64_t blockSize = session->sharedConfig->blockSize;
    uint64_t sessionBlocksCount = session->TotalBlocks();
    uint64_t processedCount = session->sharedContext->processedBitmap->TotalSetCount();
    uint64_t writtenCount = session->sharedContext->writtenBitmap->TotalSetCount();
    uint64_t bytesProcessed = (processedCount == sessionBlocksCount) ? sessionSize : (processedCount * blockSize);
    uint64_t bytesWritten = (writtenCount == sessionBlocksCount) ? sessionSize : (writtenCount * blockSize);

    counter->bytesToRead = sessionSize;
    counter->bytesRead = bytesProcessed;
    counter->bytesToWrite = bytesWritten;
    counter->bytesWritten = bytesWritten;
    counter->blocksToHash = session->sharedConfig->hasherEnabled ? processedCount : 0;
    counter->blocksHashed = session->sharedConfig->hasherEnabled ? processedCount : 0;

    DBGLOG("restore session counter : bytesToReaded: %llu, bytesRead: %llu, "
        "blocksToHash: %llu, blocksHashed: %llu, "
        "bytesToWrite: %llu, bytesWritten: %llu",
        counter->bytesToRead.load(), counter->bytesRead.load(),
        counter->blocksToHash.load(), counter->blocksHashed.load(),
        counter->bytesToWrite.load(), counter->bytesWritten.load());
}

bool VolumeTaskCheckpointTrait::ReadLatestHashingTable(std::shared_ptr<VolumeTaskSession> session) const
{
    std::string lastestChecksumBinPath = session->sharedConfig->lastestChecksumBinPath;
    uint64_t lastestChecksumTableSize = session->sharedContext->hashingContext->lastestSize;
    uint8_t* buffer = fsapi::ReadBinaryBuffer(lastestChecksumBinPath, lastestChecksumTableSize);
    if (buffer == nullptr) {
        ERRLOG("failed to read latest hashing table from %s", lastestChecksumBinPath.c_str());
        return false;
    }
    int dummy = memcpy_s(
        session->sharedContext->hashingContext->lastestTable,
        sizeof(uint8_t) * lastestChecksumTableSize,
        buffer,
        sizeof(uint8_t) * lastestChecksumTableSize);
    delete[] buffer;
    buffer = nullptr;
    (void)(dummy == 0 ? dummy : 0);
    return true;
}

std::shared_ptr<CheckpointSnapshot> VolumeTaskCheckpointTrait::ReadCheckpointSnapshot(
    std::shared_ptr<VolumeTaskSession> session) const
{
    return CheckpointSnapshot::LoadFrom(session->sharedConfig->checkpointFilePath);
}

bool VolumeTaskCheckpointTrait::CheckLastUpdateTimer()
{
    auto now = std::chrono::steady_clock::now();
    if (now - m_lastUpdate > std::chrono::minutes(1)) {
        DBGLOG("should update checkpoint now");
        m_lastUpdate = now;
        return true;
    }
    return false;
}