# HostAgent
# 编译方式

# 一、cmake 编译【源代码】
  进入Agent/build目录
  1、修改env.sh文件字段 BUILD_CMAKE=ON
  2、执行export HOME=源代码Agent上层路径
  3、导入环境变量：source env.sh
  4、执行sh agent_pack_backup.sh

# 二、cmake 编译【LLT】
  进入Agent/build目录
  1、修改env.sh文件字段 BUILD_CMAKE=ON
  2、执行export HOME=源代码Agent上层路径
  3、导入环境变量：source env.sh
  4、执行sh agent_make_cmake.sh LLT [可执行程序生成目录Agent/test/stubTest/bin]

# 三、makefile编译【源代码】  ---  ---  后续不使用
  进入Agent/build目录
  1、修改env.sh文件字段 BUILD_CMAKE=OFF
  2、执行export HOME=源代码Agent上层路径
  3、导入环境变量：source env.sh
  4、执行sh agent_pack_backup.sh

# 四、makefile编译【LLT】  ---  后续不使用
  进入Agent/test/stubTest/build目录
  1、导入环境变量：source env.sh
  2、执行sh test_make.sh [可执行程序生成目录Agent/test/stubTest/bin]