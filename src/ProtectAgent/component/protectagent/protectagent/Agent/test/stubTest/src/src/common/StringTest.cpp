#include "common/StringTest.h"
#include "common/MpString.h"
#include "common/Log.h"

#include "stub.h"

#include <cstdio>
#include <list>
#include <string>
using namespace std;

TEST_F(CMpStringTest, UIntegerToStringTest){
  mp_uint64 IntegerVal;
  mp_string strVal;
  CMpString::UIntegerToString(IntegerVal, strVal);
}

TEST_F(CMpStringTest, StringToUIntegerTest){
  mp_uint64 integerVal;
  mp_string strVal;
  CMpString::StringToUInteger(strVal, integerVal);
}

TEST_F(CMpStringTest, HasSpaceTest){
  mp_char str[] = "111";
  CMpString::HasSpace(str);
  CMpString::HasSpace(nullptr);
}

TEST_F(CMpStringTest, GetChTest){
  //CMpString::GetCh();
}

TEST_F(CMpStringTest, TrimTest){

  mp_char testString[200] = " h l ,l ee w x  x  ";
  mp_char *iRet = CMpString::Trim(testString);
  EXPECT_STREQ(iRet, "h l ,l ee w x  x");
  
  iRet = CMpString::Trim(NULL);
  EXPECT_EQ((mp_char*)NULL, iRet);
  
  strcpy_s(testString, 200, "ni hao!  ");
  iRet = CMpString::Trim(testString);
  EXPECT_EQ(testString, iRet);
  EXPECT_STREQ(iRet, "ni hao!");
  
  strcpy_s(testString, 200, "     ni hao!");
  iRet = CMpString::Trim(testString);
  EXPECT_EQ(testString, iRet);
  EXPECT_STREQ(iRet, "ni hao!");
}

TEST_F(CMpStringTest, TotallyTrimRightTest){
  mp_char testString[200] = " h l ,l ee w x  x  \n\r\t ";
  mp_char *iRet = CMpString::TotallyTrimRight(testString);
  EXPECT_EQ(iRet, testString);
  EXPECT_STREQ(iRet, " h l ,l ee w x  x");
  
  iRet = CMpString::TotallyTrimRight("");
  EXPECT_EQ(iRet, (mp_char*)NULL);
}

TEST_F(CMpStringTest, StringTrimTest){
  mp_string testString = "  h l ,l ee w x  x  ";
  mp_string strRet = CMpString::Trim(testString);
  EXPECT_EQ(strRet, "h l ,l ee w x  x");
  mp_char nullStr[] = ""; //
  strRet = CMpString::Trim(nullStr);
  EXPECT_EQ(strRet, "");
}

TEST_F(CMpStringTest, StringTrimLeftTest)
{
  mp_string testString = "  h l ,l ee w x  x  ";
  mp_string strRet = CMpString::TrimLeft(testString);
  EXPECT_EQ(strRet, "h l ,l ee w x  x  ");
  
  mp_string strTmp;
  strRet = CMpString::Trim(strTmp);
  EXPECT_EQ(strRet, strTmp);
}

TEST_F(CMpStringTest, StringTrimRightTest)
{
  mp_string testString = "  h l ,l ee w x  x  ";
  mp_string strRet = CMpString::TrimRight(testString);
  EXPECT_EQ(strRet, "  h l ,l ee w x  x");
  
  mp_string strTmp;
  strRet = CMpString::Trim(strTmp);
  EXPECT_EQ(strRet, strTmp);
}

TEST_F(CMpStringTest, StringTotallyTrimRightTest){
  mp_string testString = " h l ,l ee w x  x  \n\r\t ";
  mp_string strRet = CMpString::TotallyTrimRight(testString);
  EXPECT_EQ(strRet, " h l ,l ee w x  x");
  
  mp_string strTmp;
  strRet = CMpString::TotallyTrimRight(strTmp);
  EXPECT_EQ(strRet, strTmp);
}

TEST_F(CMpStringTest, FormatLUNIDTest){
  mp_string LunID = "0000";
  mp_string expectOut;
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("0", expectOut);
  
  LunID = "0";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("0", expectOut);
  
  LunID = "103579";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("103579", expectOut);
  
  LunID = "0103579";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("103579", expectOut);
}

TEST_F(CMpStringTest, ToUpperTest_ToLowerTest){
  
  mp_char testString[] = "Hello, WoRLD!  !";
  mp_char *iRetUpper = CMpString::ToUpper(testString);
  EXPECT_EQ(testString, iRetUpper);
  EXPECT_STREQ(iRetUpper, "HELLO, WORLD!  !");
  
  mp_char *iRetLower = CMpString::ToLower(testString);
  EXPECT_EQ(testString, iRetLower);
  EXPECT_STREQ(iRetLower, "hello, world!  !");
}

TEST_F(CMpStringTest, StringToUpperTest_ToLowerTest){
  
  mp_string testString = "Hello, WoRLD!  !";
  mp_string iRetUpper = CMpString::ToUpper(testString);
  EXPECT_EQ(iRetUpper, "HELLO, WORLD!  !");
  EXPECT_EQ(testString, "HELLO, WORLD!  !");
  
  testString = "Hello, WoRLD!  !";
  mp_string iRetLower = CMpString::ToLower(testString);
  EXPECT_EQ(iRetLower, "hello, world!  !");
  EXPECT_EQ(testString, "hello, world!  !");
}

TEST_F(CMpStringTest, StrTokenTest){
  mp_string token = "AB:CD:EF:08:x5";
  mp_string separator = ":";
  list<mp_string> lStr;
  CMpString::StrToken(token, separator, lStr);
  ASSERT_EQ(lStr.size(), 5);
  list<mp_string>::iterator it = lStr.begin();
  EXPECT_EQ(*(it++), "AB");
  EXPECT_EQ(*(it++), "CD");
  EXPECT_EQ(*(it++), "EF");
  EXPECT_EQ(*(it++), "08");
  EXPECT_EQ(*(it++), "x5");
  EXPECT_EQ(it, lStr.end());
}

TEST_F(CMpStringTest, StrSplitTest)
{
    {
        mp_string token = "AB:CD:EF:08:x5";
      mp_char separator = ':';
      vector<mp_string> lStr;
      CMpString::StrSplit(lStr, token, separator);
      ASSERT_EQ(lStr.size(), 5);
      EXPECT_EQ(lStr[0], "AB");
      EXPECT_EQ(lStr[1], "CD");
      EXPECT_EQ(lStr[2], "EF");
      EXPECT_EQ(lStr[3], "08");
      EXPECT_EQ(lStr[4], "x5");
    }
    {
        mp_string token = "A  B    C";
        vector<mp_string> lStr;
        CMpString::StrSplitEx(lStr, token, " ");
        EXPECT_EQ(3, lStr.size());
        EXPECT_EQ(lStr[0], "A");
        EXPECT_EQ(lStr[1], "B");
        EXPECT_EQ(lStr[2], "C");
    }
}

TEST_F(CMpStringTest, BlankCommaTest){
  mp_string path = "test123";
  mp_string sRet = CMpString::BlankComma(path);
  EXPECT_EQ(path, sRet);
  
  path = "test 123";
  sRet = CMpString::BlankComma(path);
  EXPECT_EQ("\""+path+"\"", sRet);
}

TEST_F(CMpStringTest, ToString)
{
    mp_int32 intVal = 10086;
    EXPECT_EQ("10086", CMpString::to_string(intVal));

    mp_uint32 uintVal = 10086;
    EXPECT_EQ("10086", CMpString::to_string(uintVal));
    
    mp_long longVal = 10086;
    EXPECT_EQ("10086", CMpString::to_string(longVal));

    mp_ulong ulongVal = 10086;
    EXPECT_EQ("10086", CMpString::to_string(ulongVal));

    mp_uint64 ullongVal = 10086;
    EXPECT_EQ("10086", CMpString::to_string(ullongVal));
}

/*
* 测试用例：EndsWith功能测试
* 前置条件：无
* CHECK点：1.都为空时返回true.2.end with时返回true，其他返回false
*/
TEST_F(CMpStringTest, EndsWithTest)
{
    EXPECT_EQ(CMpString::EndsWith("", ""), true);
    EXPECT_EQ(CMpString::EndsWith("", "12"), false);
    EXPECT_EQ(CMpString::EndsWith("12", ""), false);
    EXPECT_EQ(CMpString::EndsWith("123", "23"), true);
    EXPECT_EQ(CMpString::EndsWith("1234", "23"), false);
}