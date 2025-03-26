#include "CityHash.h"
#include "log/Log.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

#define C(x) 0x ## x ## ULL
static char cityData[2];

static const uint64_t testdata[2][16] = {
        {C(9ae16a3b2f90404f), C(75106db890237a4a), C(3feac5f636039766), C(3df09dfc64c09a2b),
        C(3cb540c392e51e29), C(6b56343feac0663), C(5b7bc50fd8e8ad92),
        C(3df09dfc64c09a2b), C(3cb540c392e51e29), C(6b56343feac0663), C(5b7bc50fd8e8ad92),
        C(95162f24e6a5f930), C(6808bdf4f1eb06e0), C(b3b1f3a67b624d82), C(c9a62f12bd4cd80b),
        C(dc56d17a)},
        {C(541150e87f415e96), C(1aef0d24b3148a1a), C(bacc300e1e82345a), C(c3cdc41e1df33513),
        C(2c138ff2596d42f6), C(f58e9082aed3055f), C(162e192b2957163d),
        C(c3cdc41e1df33513), C(2c138ff2596d42f6), C(f58e9082aed3055f), C(162e192b2957163d),
        C(fb99e85e0d16f90c), C(608462c15bdf27e8), C(e7d2c5c943572b62), C(1baaa9327642798c),
        C(99929334)}
    };

class CityHashTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<CityHash> m_ins {nullptr};
};

void CityHashTest::SetUp() {
    m_ins = std::make_unique<CityHash>();
}
void CityHashTest::TearDown() {}
void CityHashTest::SetUpTestCase() {}
void CityHashTest::TearDownTestCase() {}

static int Check(uint64_t expected, uint64_t actual) {
    int errors = 0;
    if (expected != actual) {
        ++errors;
    }
    return errors;
}

static void SetupData() {
    uint64_t k0 = 0xc3a5c85c97cb3127ULL;
    uint64_t a = 9;
    uint64_t b = 777;
    for (int i = 0; i < 2; i++) {
        a += b;
        b += a;
        a = (a ^ (a >> 41)) * k0;
        b = (b ^ (b >> 41)) * k0 + i;
        uint8_t u = b >> 37;
        memcpy(cityData + i, &u, 1);  // uint8 -> char
  }
}
static int TestCity(const uint64_t* expected, int offset, int len) {
    int ret = 0;
    ret = Check(expected[0], CityHash::CityHash64(cityData + offset, len));
    return ret;
}

/*
 * 用例名称：CityHash64
 * 前置条件：无
 * check点：获取对象key值的Hash值
 **/
TEST_F(CityHashTest, CityHash64) 
{
    SetupData();
    int i = 0;
    int errors = 0;
    for ( ; i <2; i++) {
        if( 0 != TestCity(testdata[i], i * i, i)) {
            errors++;
        }
    }
    EXPECT_EQ(errors, 0);
}
