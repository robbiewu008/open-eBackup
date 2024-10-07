# LLT 运行方法
0. 编译gmock
   执行 Module/dt_utils/build_gmock.sh

1. 使用gtest运行方式,在test当前目录下执行
   sh run_gmock_test.sh
   
2. 使用hdt运行方式,在test当前目录下执行
   sh run_hdt_test.sh
   
3. 编写LLT位置
   对应源码文件目录，在src目录下创建目录，写LLT用例。并且编写CMakieLists.txt。
   比如，Module/test/src/config_reader对应的是源码目录Module/src/config_reader。
        CMakelists.txt可以仿照Module/test/src/config_reader/CMakelists.txt。
   
   LLT代码文件命名，源码文件名称加后缀TEST。
   比如Module/test/src/config_reader/ConfigReaderTest.cpp对用的源码文件是Module/src/config_reader/ConfigReader.cpp。
   
