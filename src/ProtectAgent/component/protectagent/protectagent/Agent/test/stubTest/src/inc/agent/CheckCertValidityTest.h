#ifndef __CHECK_CERT_VALIDITY_TEST_H__
#define __CHECK_CERT_VALIDITY_TEST_H__

#define private public
#define protected public
#include "agent/CheckCertValidity.h"
#include "gtest/gtest.h"
#include "stub.h"

class CheckCertValidityTest : public testing::Test
{
public:
    Stub stub;
    CheckCertValidity worker;
};

#endif
