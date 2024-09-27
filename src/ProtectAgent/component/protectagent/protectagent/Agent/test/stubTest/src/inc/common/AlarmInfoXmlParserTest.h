
#ifndef _ALAEM_INFO_XML_PARSER_H_
#define _ALAEM_INFO_XML_PARSER_H_
#define private public
#include "common/AlarmInfoXmlParser.h"
#include "gtest/gtest.h"
#include "common/Pipe.h"
#include "stub.h"

class AlarmInfoXmlParserTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void AlarmInfoXmlParserTest::SetUp() {}

void AlarmInfoXmlParserTest::TearDown() {}

void AlarmInfoXmlParserTest::SetUpTestCase() {}

void AlarmInfoXmlParserTest::TearDownTestCase() {}

#endif