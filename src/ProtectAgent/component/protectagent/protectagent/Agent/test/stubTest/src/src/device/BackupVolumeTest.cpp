#include "device/BackupVolumeTest.h"

TEST_F(BackupVolumeTest,UnitTest){
    mp_int32 ret;
    mp_string disk;
    mp_string returnStr;

    BackupVolume work("test","test","test","test");

    returnStr = work.GetVolWwn();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetIsExtend();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetIsMetaData();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetVolumeType();
    EXPECT_EQ(returnStr, "test");
}
