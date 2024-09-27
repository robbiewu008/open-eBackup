import json

EXPORT_PATH = "/usr/local/gaussdb/GaussDB_T_1.9.0-DATASYNC/DataSync/config/"
IMPORT_PATH = "/opt/db_data/GaussDB_V5/GaussDB_T_1.9.0-DATASYNC/DataSync/config/"
EXTRACT_THRESHOLD = 150


def write_export_cfg(ip_address):
    result = extract_file(EXPORT_PATH)

    result["flow_type"] = 1
    result["export_db"]["database_type"] = 4
    result["import_db"]["database_type"] = 7
    result["export_db"]["db"]["ip"] = ip_address
    result["export_db"]["db"]["username"] = "GaussDB"
    result["export_db"]["db"]["port"] = 6432
    result["export_db"]["db"]["db_name"] = "postgres"
    result["option"]["ignore_lost_table"] = 3
    result["option"]["create_sequences"] = "true"
    result["option"]["create_tab_with_default"] = "true"

    file = open(EXPORT_PATH + 'cfg.ini', 'w')
    json.dump(result, file)


def write_import_cfg(ip_address):
    result = extract_file(IMPORT_PATH)

    result["flow_type"] = 2
    result["export_db"]["database_type"] = 4
    result["import_db"]["database_type"] = 7
    result["import_db"]["db"]["ip"] = ip_address
    result["import_db"]["db"]["username"] = "gaussdbremote"
    result["import_db"]["db"]["port"] = 6432
    result["import_db"]["db"]["db_name"] = "postgres"
    result["option"]["ignore_lost_table"] = 3
    result["option"]["create_sequences"] = "true"
    result["option"]["create_tab_with_default"] = "true"
    result["data_path"]["import_local_path"] = "/opt/db_data"

    file = open(IMPORT_PATH + 'cfg.ini', 'w')
    json.dump(result, file)


def extract_file(path):
    # 读取指定内容
    lines = []
    with open(path + 'cfg.ini', 'r') as f:
        for num, line in enumerate(f):
            if num <= EXTRACT_THRESHOLD:
                lines.append(line)
    return json.loads("\n".join(lines))


def get_pod_name(data):
    for i in range(len(data)):
        pod_name = data[i].get('metadata').get('name')
        if "gaussdb" in pod_name:
            return pod_name
