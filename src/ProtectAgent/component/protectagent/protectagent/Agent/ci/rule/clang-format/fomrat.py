#!/usr/bin/env python
# -*-coding:utf-8-*-

import os
import shutil

# clang-format工具目录
CLANG_FORMAT='clang-format.exe'
CLANG_FORMAT_CONFIG='.clang-format'
CODE_PATH=r'D:\04.Code\02.Agent\eReplication_Agent\Agent\src'

def FormatFile(dirName):
    print dirName, 111
    dirs = os.listdir(dirName)
    for path in dirs:
        sub_path = os.path.join(dirName, path)
        if os.path.isdir(sub_path):
            FormatFile(sub_path)

        if path.endswith('.cpp') or path.endswith('.h'):
            file_with_path = os.path.join(dirName,path)
            print('====format file [', file_with_path, ']====')
            os.system(CLANG_FORMAT + ' -i ' + file_with_path)

def main():
    if not os.path.exists(os.path.join(CODE_PATH, CLANG_FORMAT_CONFIG)):
        shutil.copyfile(CLANG_FORMAT_CONFIG, os.path.join(CODE_PATH, CLANG_FORMAT_CONFIG))

    FormatFile(CODE_PATH)

if __name__ == "__main__":
    main()
