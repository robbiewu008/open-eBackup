#ifndef HTTP_CLIENT_INTERFACE_H
#define HTTP_CLIENT_INTERFACE_H
#include "curlclient/HttpClientInterface.h"
#include "gtest/gtest.h"
#include "stub.h"

class HttpClientInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void HttpClientInterfaceTest::SetUp()
{}

void HttpClientInterfaceTest::TearDown()
{}

void HttpClientInterfaceTest::SetUpTestCase()
{}

void HttpClientInterfaceTest::TearDownTestCase()
{}

#endif//HTTP_CLIENT_INTERFACE_H
