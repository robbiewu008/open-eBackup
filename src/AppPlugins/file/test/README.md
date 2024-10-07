# LLT 运行方法
1.执行LLT准备工作
   步骤1， 下载Module、FS_Scanner、FS_Backup仓库代码。在plugins/file目录下
       sh build/download_code_for_developer.sh
          
2. 执行LLT的编译 
   方法一，使用gtest运行方式,在test当前目录下执行
       cd test
       sh run_gmock_test.sh
   
   方法二， 使用hdt运行方式,在test当前目录下执行
       cd test
       sh run_hdt_test.sh

       hdt运行单个用例的方法
       sh run_hdt_test.sh --gtest_filter=ApplicationManagerTest*
       或sh run_hdt_test.sh --gtest_filter=ApplicationManagerTest.ListApplicationResource_native
# DT-FUZZ 运行方法
1.执行DT-FUZZ准备工作
   步骤1， 下载Module、FS_Scanner、FS_Backup仓库代码。在plugins/file目录下
       sh build/download_code_for_developer.sh
   步骤2， 下载DT-FUZZ代码。在Module/build目录下
       sh build/download_dtfuzz.sh  
2. 执行DT-FUZZ的编译 
   方法一，使用gtest运行方式,在test当前目录下执行
       cd test
       sh run_gmock_test.sh DTFUZZ
   

