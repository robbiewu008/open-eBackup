import os
import json
import sys


def update_json_object_data(src_object, desc_object):
    for key, vaule in src_object.items():
        if key in desc_object.keys():
            if not isinstance(vaule, dict):
                desc_object[key] = vaule
            else:
                update_json_object_data(vaule, desc_object[key])


def update_json_data(src_file_path, desc_file_path):
    with open(src_file_path, "r") as file:
        src_file_data = json.loads(file.read())
    with open(desc_file_path, "r") as file:
        dest_file_data = json.loads(file.read())

    update_json_object_data(src_file_data, dest_file_data)

    with open(desc_file_path, "w") as file:
        file.write(json.dumps(dest_file_data, indent=4, ensure_ascii=False))


if __name__ == '__main__':
    print("Start update json file")
    src_file_path = sys.argv[1]
    desc_file_path = sys.argv[2]
    if not os.path.isfile(src_file_path):
        print("Error source file is not exit")
        sys.exit(1)
    if not os.path.isfile(desc_file_path):
        print("Error destination file is not exit")
        sys.exit(1)
    try:
        ret = update_json_data(src_file_path, desc_file_path)
    except Exception as ex:
        print("Error update json data file")
        sys.exit(1)
    print("End update json file")
    sys.exit(ret)