#ifndef _FUZZ_ARCHIVESTREAM_H
#define _FUZZ_ARCHIVESTREAM_H

#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzArchiveStream : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzArchiveStream::SetUp() {}

void FuzzArchiveStream::TearDown() {}

void FuzzArchiveStream::SetUpTestCase() {}

void FuzzArchiveStream::TearDownTestCase() {}
#endif