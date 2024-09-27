# Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
# Description: toolchain.cmake
# Author: hejianfan h30009301
# Create: 2022-05-20

# 添加编译选项
option(USE32BIT "Use 32-Bit" OFF)
if(USE32BIT)
    add_compile_options(-m32)
    add_link_options(-m32)
endif()

add_compile_options(-Wall)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

