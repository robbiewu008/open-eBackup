import os
import shutil


def sync_file(src, des):
    shutil.copyfile(src, des)
