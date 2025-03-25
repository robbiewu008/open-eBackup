/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

#include "gtest/gtest.h"
#include "ScanPathMapper.h"

/*
* 用例名称：路径映射
* 前置条件：无映射规则
* check点：路径映射到自身
**/
TEST(ScanPathMapperTest, Empty) {
    ScanPathMapper pathMapper;
    
    EXPECT_EQ(pathMapper.MapDir(""), "");
    EXPECT_EQ(pathMapper.MapDir("/"), "/");
    EXPECT_EQ(pathMapper.MapDir("/home"), "/home");
    EXPECT_EQ(pathMapper.MapDir("/1.png"), "/1.png");
    EXPECT_EQ(pathMapper.MapDir("/home/1.png"), "/home/1.png");

    EXPECT_EQ(pathMapper.RecoverDir(""), "");
    EXPECT_EQ(pathMapper.RecoverDir("/"), "/");
    EXPECT_EQ(pathMapper.RecoverDir("/home"), "/home");
    EXPECT_EQ(pathMapper.RecoverDir("/1.png"), "/1.png");
    EXPECT_EQ(pathMapper.RecoverDir("/home/1.png"), "/home/1.png");
}

/*
* 用例名称：路径映射
* 前置条件：单个映射规则
* check点：匹配到前缀映射目标前缀，不匹配前缀的映射自身
*/
TEST(ScanPathMapperTest, SingleRule) {
    ScanPathMapper pathMapper;
    pathMapper.AddMapRule("/mnt/12345/home", "/mnt/12345");

    EXPECT_EQ(pathMapper.MapDir(""), "");
    EXPECT_EQ(pathMapper.MapDir("/"), "/");
    EXPECT_EQ(pathMapper.MapDir("/root"), "/root");
    EXPECT_EQ(pathMapper.MapDir("/home"), "/mnt/12345/home");
    EXPECT_EQ(pathMapper.MapDir("/home/1.png"), "/mnt/12345/home/1.png");
    EXPECT_EQ(pathMapper.MapDir("/home/user"), "/mnt/12345/home/user");
    EXPECT_EQ(pathMapper.MapDir("/home/user/Download"), "/mnt/12345/home/user/Download");

    EXPECT_EQ(pathMapper.RecoverDir(""), "");
    EXPECT_EQ(pathMapper.RecoverDir("/"), "/");
    EXPECT_EQ(pathMapper.RecoverDir("/root"), "/root");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/12345/home"), "/home");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/12345/home/1.png"), "/home/1.png");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/12345/home/user"), "/home/user");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/12345/home/user/Download"), "/home/user/Download");
}

/*
* 用例名称：路径映射
* 前置条件：多个映射规则
* check点：嵌套的路径匹配最长前缀
*/
TEST(ScanPathMapperTest, MutiRule) {
    ScanPathMapper pathMapper;
    pathMapper.AddMapRule("/mnt/114514/home", "/mnt/114514");
    pathMapper.AddMapRule("/mnt/1919810/home/mount", "/mnt/1919810");

    EXPECT_EQ(pathMapper.MapDir(""), "");
    EXPECT_EQ(pathMapper.MapDir("/"), "/");
    EXPECT_EQ(pathMapper.MapDir("/root"), "/root");
    EXPECT_EQ(pathMapper.MapDir("/home"), "/mnt/114514/home");
    EXPECT_EQ(pathMapper.MapDir("/home/mount"), "/mnt/1919810/home/mount");
    EXPECT_EQ(pathMapper.MapDir("/home/1.png"), "/mnt/114514/home/1.png");
    EXPECT_EQ(pathMapper.MapDir("/home/mount/2.png"), "/mnt/1919810/home/mount/2.png");
    EXPECT_EQ(pathMapper.MapDir("/home/mount/Download"), "/mnt/1919810/home/mount/Download");

    EXPECT_EQ(pathMapper.RecoverDir(""), "");
    EXPECT_EQ(pathMapper.RecoverDir("/"), "/");
    EXPECT_EQ(pathMapper.RecoverDir("/root"), "/root");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/114514/home"), "/home");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/1919810/home/mount"), "/home/mount");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/114514/home/1.png"), "/home/1.png");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/1919810/home/mount/2.png"), "/home/mount/2.png");
    EXPECT_EQ(pathMapper.RecoverDir("/mnt/1919810/home/mount/Download"), "/home/mount/Download");
}

