import hashlib
import os


def sha256sum(filename):
    """计算文件的SHA-256摘要"""
    sha256 = hashlib.sha256()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b''):
            sha256.update(chunk)
    return sha256.hexdigest()


def opened_file_sha256sum(opened_file):
    sha256 = hashlib.sha256()
    for chunk in iter(lambda: opened_file.read(4096), b''):
        sha256.update(chunk)
    return sha256.hexdigest()


def calculate_folder_sha256(folder):
    """计算文件夹中所有文件的SHA-256摘要总和"""
    total_sha256 = hashlib.sha256()
    for root, dirs, files in os.walk(folder):
        for file in files:
            filepath = os.path.join(root, file)
            file_sha256 = sha256sum(filepath)
            total_sha256.update(file_sha256.encode())
    return total_sha256.hexdigest()


def folder_equal_sha256(folder1, folder2):
    """比较两个文件夹中的文件是否相同"""
    folder1_files = {}
    folder2_files = {}

    # 获取文件夹1中的文件哈希值
    for dirpath, _, filenames in os.walk(folder1):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            relative_path = os.path.relpath(file_path, folder1)
            folder1_files[relative_path] = sha256sum(file_path)

    # 获取文件夹2中的文件哈希值
    for dirpath, _, filenames in os.walk(folder2):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            relative_path = os.path.relpath(file_path, folder2)
            folder2_files[relative_path] = sha256sum(file_path)

    if folder1_files == folder2_files:
        return True
    else:
        return False
