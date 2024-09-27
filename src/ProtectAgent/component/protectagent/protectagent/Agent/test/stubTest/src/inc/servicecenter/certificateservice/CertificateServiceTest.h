#ifndef CERTIFICATE_SERVICE_TEST_H_
#define CERTIFICATE_SERVICE_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class CertificateServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void CertificateServiceTest::SetUp() {}

void CertificateServiceTest::TearDown() {}

void CertificateServiceTest::SetUpTestCase() {}

void CertificateServiceTest::TearDownTestCase() {}

#endif