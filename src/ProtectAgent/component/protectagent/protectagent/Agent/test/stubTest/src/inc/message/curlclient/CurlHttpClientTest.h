#ifndef CURL_HTTP_CLIENT_AINTERFACETEST_H
#define CURL_HTTP_CLIENT_AINTERFACETEST_H
#define private public

#include "gtest/gtest.h"
#include "stub.h"
class CurlHttpClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void CurlHttpClientTest::SetUp()
{}

void CurlHttpClientTest::TearDown()
{}

void CurlHttpClientTest::SetUpTestCase()
{}

void CurlHttpClientTest::TearDownTestCase()
{}
#endif  // CURL_HTTP_CLIENT_AINTERFACETEST_H
