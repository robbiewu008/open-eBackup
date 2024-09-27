import os
import subprocess
import shutil
import yaml
import sys


def build_client_package():
    version = sys.argv[1]
    current_path = os.getcwd()
    manifest_file = os.path.join(current_path, "manifest.yml")

    with open(manifest_file, 'r') as file:
        yml_content = yaml.safe_load(file)

    yml_content["Version"] = version
    with open(manifest_file, 'w') as file:
        yaml.safe_dump(yml_content, file)

    p = subprocess.Popen(['pip3', 'install', '-r', '../requirements.txt'])
    p.communicate()

    p = subprocess.Popen(['pyinstaller', '--clean', '-F', 'dpclient.py'])
    p.communicate()

    os.makedirs("dpclient", exist_ok=True)
    destination_path = os.path.join(current_path, "dpclient", "manifest.yml")
    os.replace(manifest_file, destination_path)

    source_exe = os.path.join(current_path, "dist", "dpclient.exe")
    target_exe = os.path.join(current_path, "dpclient", "dpclient.exe")
    os.replace(source_exe, target_exe)

    source_temp_dir = os.path.join(current_path, "template")
    target_temp_dir = os.path.join(current_path, "dpclient/template")
    shutil.copytree(source_temp_dir, target_temp_dir, dirs_exist_ok=True)
    # p = subprocess.Popen(["7z.exe", "a -tzip", "dpclient_windows.zip", "dpclient"])
    p.communicate()


if __name__ == "__main__":
    build_client_package()
