#include "dataprocess/DataContextTest.h"

TEST_F(DataContextTest, GetSockFd) {
    mp_socket sockFd = 1234;
    DataContext om;
    om.SetSockFd(sockFd);
    mp_int32 iRet = sockFd == om.GetSockFd() ? 1 : 0;
    EXPECT_EQ(1, iRet);
}

TEST_F(DataContextTest, GetDiskFdByName) {
    DataContext om;
    om.SetDiskFdByName("test", 1234);
    mp_int32 iRet = om.GetDiskFdByName("test");
    EXPECT_NE(MP_FAILED, iRet);
}