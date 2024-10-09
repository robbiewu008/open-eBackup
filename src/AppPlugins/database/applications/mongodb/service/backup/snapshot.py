#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

import json
import re

from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, MongoDBCode, DefaultValue


class Snap:
    """
        快照备份基础功能实现类
    """

    def __init__(self, cmd):
        self.lvs = []
        self.vgs = []
        self.cmd = cmd

    @staticmethod
    def parse_lvm_name(lvm_mapper):
        vg_lvm_name = lvm_mapper.rsplit("/", 1)[-1]
        lvm_name = vg_lvm_name.rsplit("-", 1)[-1]
        return lvm_name

    def show_lvs(self):
        """
        查询节点中的所有逻辑卷
        :return:
        """
        ret, lvs_cont = self.cmd.show_lv_info()
        if not ret:
            return []
        lvs_json = json.loads(lvs_cont)
        report = lvs_json.get("report", [])
        if not report:
            return []
        lvs = report[0].get("lv", [])
        return lvs

    def show_vgs(self):
        """
        查询节点中的所有卷组
        :return:
        """
        ret, vgs_cont = self.cmd.show_vg_info()
        if not ret:
            return []
        vgs_json = json.loads(vgs_cont)
        report = vgs_json.get("report", [])
        for vg in report:
            if "vg" in vg:
                vgs_info = vg.get("vg", [])
                return vgs_info
        return []

    def parse_df_info(self, data_path):
        """
        获取数据目录所在的逻辑卷信息：逻辑卷名(mapper形式)、文件系统类型、大小、可用空间、使用率、挂载路径
        :return:
        """
        ret, lvm_info = self.cmd.get_lvm_info(data_path)
        lvm_info = "" if not ret else lvm_info
        pattern = r"on\s+(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\s+(\S*)\n"
        tmp = re.search(pattern, lvm_info)
        if not tmp:
            result = []
            output_line = lvm_info.split("\n")[1]
            for output in output_line.split():
                result.append(output)
            return result
        return tmp.groups()

    def get_lvm_info(self, lvm_name):
        lvs = self.show_lvs()
        for lvm_info in lvs:
            if lvm_info.get("lv_name") == lvm_name:
                return lvm_info
        return {}

    def get_vg_mount_style(self, lvm_path, mount_path):
        """
        检查逻辑卷挂载方式
        :return:
        """
        ret, mount_cont = self.cmd.get_mount_info()
        if not ret:
            return "Default"
        pattern = re.compile(fr"{lvm_path}\s+{mount_path}")
        if pattern.search(mount_cont):
            return "Name"
        pattern = re.compile(fr"UUID=(\S*)\s+{mount_path}\s")
        if pattern.search(mount_cont):
            return "UUID"
        pattern = re.compile(fr"Label(\S*)\s+{mount_path}\s")
        if pattern.search(mount_cont):
            return "Label"
        return "Default"

    def check_lvm_name(self, lvm_name):
        if not self.lvs:
            self.lvs = self.show_lvs()
        lvs_names = [lvm.get("lv_name", "") for lvm in self.lvs]
        return lvm_name in lvs_names

    def check_lvm_free_size(self, lvm_info, snap_per):
        """
        检查逻辑卷可用空间
        :return:
        """
        if not self.vgs:
            self.vgs = self.show_vgs()
        vg_name = lvm_info.get("vg_name", "")
        lv_size = lvm_info.get("lv_size", "")
        vg_free = ""
        for vg in self.vgs:
            if vg.get("vg_name") == vg_name:
                vg_free = vg.get("vg_free")
                break
        if not vg_free:
            return False
        vg_size = vg_free.strip().strip(DefaultValue.SNAP_UNIT.value)
        lv_size = lv_size.strip().strip(DefaultValue.SNAP_UNIT.value)
        try:
            lv, vg = map(float, (lv_size, vg_size))
        except ValueError:
            LOGGER.error("Trans size failed with vg: %s, lv: %s", vg_size, lv_size)
            return False
        # 卷组剩余空间大于逻辑卷10%
        LOGGER.info("Check lvm free size vg: %s, lv: %s", vg_size, lv_size)
        return lv * snap_per / 100 < vg

    def create_snap_shot(self, size, name, lvm_path):
        """
        创建快照
        :return:vg格式的快照路径:  /dev/vg/snap_name
        """
        ret, info = self.cmd.create_snap_shot(size, name, lvm_path)
        if not ret:
            LOGGER.info("Create snap failed, err_info:%s", info)
            return ErrorCode.FAILED_CREATE_SNAP.value, info
        return MongoDBCode.SUCCESS.value, name

    def mount_lvm(self, snap_full_path, tmp_mount, fs_type=''):
        """
        挂载逻辑卷，xfs文件系统使用不同指令
        :return:
        """
        if fs_type == "xfs":
            ret, info = self.cmd.mount_xfs(snap_full_path, tmp_mount)
        else:
            ret, info = self.cmd.mount(snap_full_path, tmp_mount)
        if not ret:
            LOGGER.info("mount lvm failed,err_info:%s", info)
        return ret

    def umount_lvm(self, snap_full_path):
        """
        挂载逻辑卷
        :return:
        """
        ret, info = self.cmd.umount(snap_full_path)
        if not ret:
            LOGGER.error("Umount lvm failed, ret:%s, info:%s, snap full path:%s.", ret, info, snap_full_path)
            return ErrorCode.UMOUNT_SNAPSHOT_FILE_ERROR.value, info, snap_full_path
        return MongoDBCode.SUCCESS.value, info, snap_full_path

    def release_lvm(self, snap_full_path):
        """
        释放逻辑卷
        :return:
        """
        ret, info = self.cmd.lvremove(snap_full_path)
        LOGGER.debug("Release snap lv: %s with ret:%s, info:%s", snap_full_path, ret, info)
        if not ret:
            LOGGER.error("Release snap lv failed, ret:%s, info:%s, snap full path:%s.", ret, info, snap_full_path)
            return ErrorCode.UMOUNT_SNAPSHOT_FILE_ERROR.value, info, snap_full_path
        return MongoDBCode.SUCCESS.value, info, snap_full_path
