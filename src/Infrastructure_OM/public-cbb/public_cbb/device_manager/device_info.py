#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import time
from typing import List

from public_cbb.config.global_config import get_settings
from public_cbb.device_manager.constants import (
    MAX_NAME_LENGTH, CompressAlgorithmEnum, DirectoryDistributionModeEnum, RunningStatusEnum, ReadWriteStatusEnum,
    NasShareType, QOS_TWNETY_FOUR_HOUR, QOS_ZERO_CLOCK, QosPolicyType, QosSchedulingPolicy, QOS_MIN_IO_PS_VALUE,
    QOS_MAX_IO_PS_BAND_WITH_VALUE, QOS_MIN_BAND_WITH_VALUE, QOS_LATENCY_500US, QOS_LATENCY_1500US, QOS_MIN_BURST_TIME,
    QOS_MAX_BURST_TIME, SecresAccessEnum
)


class DeviceInfo:
    def __init__(
            self,
            ip: str,
            port: str,
            username: str = "",
            password: str = "",
            device_id: str = "",
            cert: str = '',
            pool_id: int = 0,
            compress_enable: bool = get_settings().FS_COMPRESS_ENABLE,
            compress_algorithm: CompressAlgorithmEnum = CompressAlgorithmEnum.RAPID_COMPRESSION.value,
            dedup_enable: bool = get_settings().FS_DEDUP_ENABLE,
            sector_size: int = 32768,
            show_snap_directory: bool = True,
            alloc_type: int = 1,
            security_style: int = 3,
            directory_distribution_mode:
            DirectoryDistributionModeEnum = DirectoryDistributionModeEnum.CONTROLLER_AFFINITY_MODE.value,
            skip_affinity_mode_count: int = 0
    ):
        self.ip = ip
        self.port = port
        self.username = username
        self.password = password
        self.device_id = device_id
        self.cert = cert
        self.pool_id = pool_id
        self.compress_enable = compress_enable
        self.compress_algorithm = compress_algorithm
        self.dedup_enable = dedup_enable
        self.sector_size = sector_size
        self.show_snap_directory = show_snap_directory
        self.alloc_type = alloc_type
        self.security_style = security_style
        self.directory_distribution_mode = directory_distribution_mode
        self.skip_affinity_mode_count: int = skip_affinity_mode_count


class HostInfo:
    def __init__(self, host_id: str, iscsinitor: str = '', host_ip: str = ''):
        # the max length of the LUN name is 32
        if len(host_id) > MAX_NAME_LENGTH:
            host_id = host_id[0:MAX_NAME_LENGTH]
        self.host_id = host_id
        self.host_ip = host_ip
        self.iscsinitor = iscsinitor


class DeviceDetails:
    def __init__(
            self,
            device_name: str = '',
            device_id: int = 0,
            share_id: int = 0,
            share_client_id: int = 0,
            device_unique_path: str = '',
            share_path: str = '',
            compress_enable: bool = False,
            compress_algorithm: int = 0,
            dedup_enable: bool = False,
            total_capacity: int = 0,
            used_capacity: int = 0,
            total_saved_capacity: int = 0,
            running_status: RunningStatusEnum = RunningStatusEnum.INVALID.value,
            read_write_status: ReadWriteStatusEnum = ReadWriteStatusEnum.NO_ACCESS.value,
            owning_controller: str = '',
            remote_replication_ids=None,
            capacity_threshold: int = 0,
            used_capacity_ratio: int = 0,
            worm_type: int = 0,
            parent_file_system_id: str = '',
            security_style: int = 0,
            alloc_type: int = 0,
            parent_id: str = '',
            is_show_snap_dir: bool = False,
            sector_size: int = 0,
            dist_alg: int = 0,
            skip_affinity_mode_count: int = 0,
            io_class_id: str = '',
            unix_permission: int = 0,
            space_self_adjusting_mode: int = 0,
            max_auto_size: int = 0,
            auto_grow_threshold_percent: int = 0,
            is_migrating: bool = False
    ):
        if not remote_replication_ids:
            remote_replication_ids = []
        self.device_name = device_name
        self.device_id = device_id
        self.share_id = share_id
        self.share_client_id = share_client_id
        self.device_unique_path = device_unique_path
        self.share_path = share_path
        self.compress_enable = compress_enable
        self.compress_algorithm = compress_algorithm
        self.dedup_enable = dedup_enable
        self.total_capacity = total_capacity
        self.used_capacity = used_capacity
        self.total_saved_capacity = total_saved_capacity
        self.running_status = running_status
        self.read_write_status = read_write_status
        self.owning_controller = owning_controller
        self.remote_replication_ids = remote_replication_ids
        self.capacity_threshold = capacity_threshold
        self.used_capacity_ratio = used_capacity_ratio
        self.worm_type = worm_type
        self.parent_file_system_id = parent_file_system_id
        self.security_style = security_style
        self.alloc_type = alloc_type
        self.parent_id = parent_id
        self.is_show_snap_dir = is_show_snap_dir
        self.sector_size = sector_size
        self.dist_alg = dist_alg
        self.skip_affinity_mode_count = skip_affinity_mode_count
        self.io_class_id = io_class_id
        self.unix_permission = unix_permission
        self.space_self_adjusting_mode = space_self_adjusting_mode
        self.max_auto_size = max_auto_size
        self.auto_grow_threshold_percent = auto_grow_threshold_percent
        self.is_migrating = is_migrating


class NasShareInfo:
    def __init__(
            self,
            share_name: str = '',
            share_id: str = '',
            share_path: str = '',
            fs_id: str = '',
            smb3: bool = False,
            file_handle_byte_alignment: bool = False,
            name: str = ''

    ):
        self.share_name = share_name
        self.share_id = share_id
        self.share_path = share_path
        self.fs_id = fs_id
        self.smb3 = smb3
        self.file_handle_byte_alignment = file_handle_byte_alignment
        self.name = name


class NasSharedClientInfo:
    def __init__(
            self,
            name: str = '',
            client_id: str = '',
            parent_id: str = '',
            permission: str = '',
            cifs_domain_type: str = ''
    ):
        self.name = name
        self.id = client_id
        self.parent_id = parent_id
        self.permission = permission
        self.cifs_domain_type = cifs_domain_type


class NasSnapshotInfo:
    def __init__(
            self,
            snapshot_name: str = '',
            snapshot_id: str = '',
            file_system_name: str = '',
            file_system_id: str = '',
            dtree_id: str = "",
            dtree_path: str = ""
    ):
        self.snapshot_name = snapshot_name
        self.snapshot_id = snapshot_id
        self.file_system_name = file_system_name
        self.file_system_id = file_system_id
        self.dtree_id = dtree_id
        self.dtree_path = dtree_path


class RevertInfo:
    def __init__(
            self,
            rollback_rate: int = 0,
            rollback_status: int = 0,
            rollback_speed: int = 0,
            rollback_start_time: int = 0,
            rollback_end_time: int = 0
    ):
        self.rollback_rate = rollback_rate
        self.rollback_status = rollback_status
        self.rollback_speed = rollback_speed
        self.rollback_start_time = rollback_start_time
        self.rollback_end_time = rollback_end_time


class NasMountInfo:
    def __init__(
            self,
            mount_type: NasShareType = NasShareType.NFS,
            mount_ip: str = '',
            mount_path: str = '',
            local_path: str = '',
            cifs_user_name: str = '',
            cifs_password: str = '',
            source_id: str = '',
            osad_ip_list: str = '',
            osad_auth_port: str = '',
            osad_server_port: str = '',
            mount_point: str = ''
    ):
        self.mount_type = mount_type
        self.mount_ip = mount_ip
        self.mount_path = mount_path
        self.local_path = local_path
        self.cifs_user_name = cifs_user_name
        self.cifs_password = cifs_password
        self.source_id = source_id
        self.osad_ip_list = osad_ip_list
        self.osad_auth_port = osad_auth_port
        self.osad_server_port = osad_server_port
        self.mount_point = mount_point


class DtreeInfo:
    def __init__(
            self,
            dtree_name: str = '',
            dtree_id: str = '',
            file_system_name: str = '',
            file_system_id: str = '',
            path: str = '',
            quota_switch: str = '',
            quota_switch_status: str = '',
            security_style: str = ''
    ):
        self.dtree_name = dtree_name
        self.dtree_id = dtree_id
        self.file_system_name = file_system_name
        self.file_system_id = file_system_id
        self.path = path
        self.quota_switch = quota_switch
        self.quota_switch_status = quota_switch_status
        self.security_style = security_style


class LogicPortInfo:
    def __init__(
            self,
            ipv4_addr: str = '',
            ipv6_addr: str = '',
            running_status: str = '',
            role: str = '',
            support_protocol: str = '',
            home_controller_id: str = '',
            port_type: str = ''
    ):
        self.ipv4_addr = ipv4_addr
        self.ipv6_addr = ipv6_addr
        self.running_status = running_status
        self.role = role
        self.support_protocol = support_protocol
        self.home_controller_id = home_controller_id
        self.port_type = port_type


class SmartQoSInfo:
    def __init__(
            self,
            qos_id: str = '',
            name: str = '',
            running_status: str = '',
            enable_status: bool = False,
            schedule_policy: int = QosSchedulingPolicy.DAILY.value,
            policy_type: str = f'{QosPolicyType.NORMAL}',
            schedule_start_time: int = round(time.time()),
            start_time: str = QOS_ZERO_CLOCK,
            duration: int = QOS_TWNETY_FOUR_HOUR,
            max_band_width: int = None,
            min_band_width: int = None,
            burst_band_width: int = None,
            max_io_ps: int = None,
            min_io_ps: int = None,
            burst_io_ps: int = None,
            latency: int = None,
            burst_time: int = None,
            # pacific converged qos 参数
            max_band_width_converged: int = None,
            basic_band_width_converged: int = None,
            max_iops_converged: int = None,
            package_size: int = None,
            bps_density: int = None,
            qos_scale: int = None,
            qos_mode: int = None,
            vstore_id: str = None
    ):
        self.id = qos_id
        self.name = name
        self.running_status = running_status
        self.enable_status = enable_status
        self.fs_id_list = []
        self.lun_id_list = []
        self.schedule_policy = schedule_policy
        self.policy_type = policy_type
        self.schedule_start_time = schedule_start_time
        self.start_time = start_time
        self.duration = duration
        self.max_band_width = max_band_width
        self.min_band_width = min_band_width
        self.burst_band_width = burst_band_width
        self.max_io_ps = max_io_ps
        self.min_io_ps = min_io_ps
        self.burst_io_ps = burst_io_ps
        self.latency = latency
        self.burst_time = burst_time
        # pacific converged qos 参数
        self.basic_band_width_converged = basic_band_width_converged
        self.max_band_width_converged = max_band_width_converged
        self.max_iops_converged = max_iops_converged
        self.bps_density = bps_density
        self.package_size = package_size
        self.qos_scale = qos_scale
        self.qos_mode = qos_mode
        self.vstore_id = vstore_id

    @property
    def max_band_width(self):
        return self._max_band_width

    @max_band_width.setter
    def max_band_width(self, value):
        if value and not (QOS_MIN_BAND_WITH_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:max_band_width, value:{value} is illegal")
        self._max_band_width = int(value) if value else None

    @property
    def min_band_width(self):
        return self._min_band_width

    @min_band_width.setter
    def min_band_width(self, value):
        if value and not (QOS_MIN_BAND_WITH_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:min_band_width, value:{value} is illegal")
        self._min_band_width = int(value) if value else None

    @property
    def burst_band_width(self):
        return self._burst_band_width

    @burst_band_width.setter
    def burst_band_width(self, value):
        if value and not (QOS_MIN_BAND_WITH_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:burst_band_width, value:{value} is illegal")
        self._burst_band_width = int(value) if value else None

    @property
    def max_io_ps(self):
        return self._max_io_ps

    @max_io_ps.setter
    def max_io_ps(self, value):
        if value and not (QOS_MIN_IO_PS_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:max_io_ps, value:{value} is illegal")
        self._max_io_ps = int(value) if value else None

    @property
    def min_io_ps(self):
        return self._min_io_ps

    @min_io_ps.setter
    def min_io_ps(self, value):
        if value and not (QOS_MIN_IO_PS_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:min_io_ps, value:{value} is illegal")
        self._min_io_ps = int(value) if value else None

    @property
    def burst_io_ps(self):
        return self._burst_io_ps

    @burst_io_ps.setter
    def burst_io_ps(self, value):
        if value and not (QOS_MIN_IO_PS_VALUE <= int(value) <= QOS_MAX_IO_PS_BAND_WITH_VALUE):
            raise ValueError(f"Qos check value range fail, key:burst_io_ps, value:{value} is illegal")
        self._burst_io_ps = int(value) if value else None

    @property
    def latency(self):
        return self._latency

    @latency.setter
    def latency(self, value):
        if value and not (int(value) == QOS_LATENCY_500US or int(value) == QOS_LATENCY_1500US):
            raise ValueError(f"Qos check value range fail, key:latency, value:{value} is illegal")
        self._latency = int(value) if value else None

    @property
    def burst_time(self):
        return self._burst_time

    @burst_time.setter
    def burst_time(self, value):
        if value and not (QOS_MIN_BURST_TIME <= int(value) <= QOS_MAX_BURST_TIME):
            raise ValueError(f"Qos check value range fail, key:burst_time, value:{value} is illegal")
        self._burst_time = int(value) if value else None


class ReplicationPairInfo:
    def __init__(
            self,
            pair_id: str = '',
            status: int = 0,
            band_width: int = 0,
            progress: int = 0,
            secres_data_status: int = 0,
            is_primary: bool = False,
            secres_access: SecresAccessEnum = SecresAccessEnum.READ_ONLY
    ):
        self.pair_id = pair_id
        self.status = status
        self.band_width = band_width
        self.progress = progress
        self.secres_data_status = secres_data_status
        self.is_primary = is_primary
        self.secres_access = secres_access


class WindowsUserInfo:
    def __init__(
            self,
            name: str = '',
            rid: str = '',
            password_validity_period: str = '',
            status: bool = False,
            privileges: str = ''
    ):
        self.name = name
        self.rid = rid
        self.password_validity_period = password_validity_period
        self.status = status
        self.privileges = privileges
        self.group_name_list = []


class ReductionInfo:
    def __init__(
            self,
            total_write_capacity: int = 0,
            reduction_ratio: str = '',
            capacity_befor_reduction: int = 0,
            capacity_after_reduction: int = 0,
            excl_cap_befor_red: int = 0,
            excl_cap_after_red: int = 0,
            incompressible_cap: int = 0,
            shared_cap_ratio: str = '',
            excl_cap_ratio: str = '',
            incompressible_ratio: str = ''
    ):
        self.total_write_capacity = total_write_capacity
        self.reduction_ratio = reduction_ratio
        self.capacity_befor_reduction = capacity_befor_reduction
        self.capacity_after_reduction = capacity_after_reduction
        self.excl_cap_befor_red = excl_cap_befor_red
        self.excl_cap_after_red = excl_cap_after_red
        self.incompressible_cap = incompressible_cap
        self.shared_cap_ratio = shared_cap_ratio
        self.excl_cap_ratio = excl_cap_ratio
        self.incompressible_ratio = incompressible_ratio


class SystemInfo:
    def __init__(
            self,
            sys_id: str = '',
            product_mode: str = '',
            running_status: str = ''
    ):
        self.sys_id = sys_id
        self.product_mode = product_mode
        self.running_status = running_status


class StorageTaskInfo:
    def __init__(
            self,
            task_id: str = '',
            task_status: str = ''
    ):
        self.task_id = task_id
        self.task_status = task_status


class ModifyFileSystemInfo:
    def __init__(
            self,
            file_system_id: int,
            size_in_mega_bytes: int = 0,
            capacity_threshold: int = 0,
            auto_grow_capacity: bool = False,
            dist_alg: int = 0,
            security_style: int = 0,
            compress_enable: bool = False
    ):
        self.file_system_id = file_system_id
        self.size_in_mega_bytes = size_in_mega_bytes
        self.capacity_threshold = capacity_threshold
        self.auto_grow_capacity = auto_grow_capacity
        self.dist_alg = dist_alg
        self.security_style = security_style
        self.compress_enable = compress_enable


class CifsIpControllerRule:
    def __init__(
            self,
            vstore_id: str = "",
            id: str = "",
            cifs_share_id: str = "",
            rule: str = ""
    ):
        self.vstore_id = vstore_id
        self.id = id
        self.cifs_share_id = cifs_share_id
        self.rule = rule


class OsaDeviceInfo:
    def __init__(
            self,
            device_id: str = "",
            username: str = ""
    ):
        self.device_id = device_id
        self.username = username


class OsaLivemountInfo:
    def __init__(self,
                 clone_file_system_name: str = " ",
                 source_file_system_name: str = " ",
                 snapshot_name: str = " ",
                 task_id: str = " ",
                 repository: List[dict] = None,
                 character_set: int = 0,
                 vstore_id: str = ''):
        self.clone_file_system_name = clone_file_system_name
        self.source_file_system_name = source_file_system_name
        self.snapshot_name = snapshot_name
        self.task_id = task_id
        self.repository = repository
        self.character_set = character_set
        self.vstore_id = vstore_id
