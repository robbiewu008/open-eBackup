#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import json
import os
from typing import List

from public_cbb.communication.rest.https_request import HttpRequest
from public_cbb.exception.custom_exception import CustomException
from public_cbb.exception.error_codes import GeneralErrorCodes
from public_cbb.device_manager.storage_base import StorageBase
from public_cbb.device_manager.constants import (
    ReturnCode, QOS_NAME_MAX_LENGTH, RunningStatusEnum, SMART_QOS_TYPE, DEPLOY_TYPE_GROUP_NO_REPLICATION, QosScale,
    QosReturnCode, CONVERGED_QOS_NAME_MAX_LENGTH, StorageErrorCode
)
from public_cbb.device_manager.device_info import SmartQoSInfo, ReplicationPairInfo
from public_cbb.log.logger import get_logger
from public_cbb.security.anonym_utils.anonymity import Anonymity

log = get_logger()


class StorageBaseCommon(StorageBase):
    def __init__(self, device_info):
        super().__init__(device_info)

    @staticmethod
    def _check_converged_qos_param_valid(qos_info):
        # compound_condition description: [(condition1,error_desc1), (condition2,error_desc1)...]
        qos_scale_check = False
        if qos_info.qos_scale:
            for valid_scale in iter(QosScale):
                if qos_info.qos_scale == valid_scale.value:
                    qos_scale_check = True
                    break
        qos_mode_check = False
        if qos_info.qos_mode:
            for valid_mode in iter(QosScale):
                if qos_info.qos_scale == valid_mode.value:
                    qos_mode_check = True
                    break
        compound_condition = [
            (qos_info.qos_scale and not qos_scale_check, f"Invalid qos scale"),
            (qos_info.qos_mode and not qos_mode_check, f"Invalid qos mode"),
            (qos_info.basic_band_width_converged and qos_info.max_band_width_converged and (
                        qos_info.basic_band_width_converged > qos_info.max_band_width_converged),
             f"Qos check param valid fail: basic_band_width_converged greater than max_band_width_converged."),
        ]
        for item in compound_condition:
            if item[0]:
                log.error(item[1])
                raise CustomException(GeneralErrorCodes.ERR_INVALID_PARAM, item[1])
        return ReturnCode.SUCCESS, ''

    @staticmethod
    def _set_smart_qos_param(qos_info: SmartQoSInfo, req_body: dict):
        if qos_info.max_band_width:
            req_body['MAXBANDWIDTH'] = qos_info.max_band_width
        if qos_info.min_band_width:
            req_body['MINBANDWIDTH'] = qos_info.min_band_width
        if qos_info.burst_band_width:
            req_body['BURSTBANDWIDTH'] = qos_info.burst_band_width
        if qos_info.max_io_ps:
            req_body['MAXIOPS'] = qos_info.max_io_ps
        if qos_info.min_io_ps:
            req_body['MINIOPS'] = qos_info.min_io_ps
        if qos_info.burst_io_ps:
            req_body['BURSTIOPS'] = qos_info.burst_io_ps
        if qos_info.latency:
            req_body['LATENCY'] = qos_info.latency
        if qos_info.burst_time:
            req_body['BURSTTIME'] = qos_info.burst_time
        return ReturnCode.SUCCESS, ''

    @staticmethod
    def _check_smart_qos_param_valid(qos_info: SmartQoSInfo):
        # compound_condition description: [(condition1,error_desc1), (condition2,error_desc1)...]
        compound_condition = [
            (qos_info.burst_time and not qos_info.burst_io_ps and
             not qos_info.burst_band_width, f"Qos check param valid fail: BURSTTIME is setted, At least one "
                                            f"configuaration is required for BURSTBANDWIDTH and BURSTIOPS."),
            (not qos_info.min_band_width and not qos_info.max_band_width and
             not qos_info.max_io_ps and not qos_info.min_io_ps and not qos_info.latency,
             f"Qos check param valid fail: At least one configuration is required for MAXBANDWIDTH, MAXIOPS, "
             f"MINBANDWIDTH, MINIOPS, LATENCY."),
            ((qos_info.burst_io_ps and not qos_info.max_io_ps) or
             (qos_info.burst_band_width and not qos_info.max_band_width),
             f"Qos check param valid fail: Config Burst Value, must config max value."),
            (qos_info.min_band_width and qos_info.max_band_width and (qos_info.min_band_width >
                                                                      qos_info.max_band_width),
             f"Qos check param valid fail: minBandWith greater than maxBandWith."),
            (qos_info.max_io_ps and qos_info.min_io_ps and (qos_info.min_io_ps > qos_info.max_io_ps),
             f"Qos check param valid fail: IOPSMin greater than IOPSMax."),
            (qos_info.burst_band_width and qos_info.max_band_width and (qos_info.max_band_width >=
                                                                        qos_info.burst_band_width),
             f"Qos check param valid fail: maxBand greater than BandwidthBurst."),
            (qos_info.burst_band_width and qos_info.min_band_width and (qos_info.min_band_width >=
                                                                        qos_info.burst_band_width),
             f"Qos check param valid fail: minBand greater than BandwidthBurst."),
            (qos_info.burst_io_ps and qos_info.max_io_ps and (qos_info.max_io_ps >= qos_info.burst_io_ps),
             f"Qos check param valid fail: IOPSMax greater than IOPSBurst."),
            (qos_info.burst_io_ps and qos_info.min_io_ps and (qos_info.min_io_ps >= qos_info.burst_io_ps),
             f"Qos check param valid fail: IOPSMax greater than IOPSBurst."),
        ]
        for item in compound_condition:
            if item[0]:
                log.error(item[1])
                raise CustomException(GeneralErrorCodes.ERR_INVALID_PARAM, item[1])
        return ReturnCode.SUCCESS, ''

    def query_smart_qos(self, qos_name, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/ioclass?NAME={qos_name}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query smart qos fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and isinstance(rsp.get('data'), list) and rsp.get('data')[0].get('NAME') == qos_name:
            qos_info.id = rsp.get('data')[0].get('ID')
            qos_info.name = rsp.get('data')[0].get('NAME')
            qos_info.running_status = rsp.get('data')[0].get('RUNNINGSTATUS')
            qos_info.enable_status = (True if rsp.get('data')[0].get('ENABLESTATUS') == 'true' else False)
            if rsp.get('data')[0].get('FSLIST'):
                qos_info.fs_id_list = json.loads(rsp.get('data')[0].get('FSLIST'))
            if rsp.get('data')[0].get('LUNLIST'):
                qos_info.lun_id_list = json.loads(rsp.get('data')[0].get('LUNLIST'))
            qos_info.schedule_policy = rsp.get('data')[0].get('SCHEDULEPOLICY')
            qos_info.policy_type = rsp.get('data')[0].get('POLICYTYPE')
            qos_info.schedule_start_time = rsp.get('data')[0].get('SCHEDULESTARTTIME')
            qos_info.start_time = rsp.get('data')[0].get('STARTTIME')
            qos_info.duration = rsp.get('data')[0].get('DURATION')
            # 条件返回
            try:
                self.get_qos_info(qos_info, rsp.get('data')[0])
            except ValueError as ex:
                log.error(f"Query qos rsp value exception:{Anonymity.process(str(ex))}.")
                raise CustomException(GeneralErrorCodes.ERR_INVALID_PARAM, ex) from ex
            log.info(f"Query qos success, name:{qos_name}.")
            return ReturnCode.SUCCESS, ''
        else:
            log.info(f"Query qos:{qos_name} not exist.")
            error_code, error_desc = ReturnCode.FAILED, f"Query qos {qos_name} not exist."
        return error_code, error_desc

    def query_smart_qos_by_id(self, qos_id, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/ioclass/{qos_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query smart qos by id fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and isinstance(rsp.get('data'), dict):
            qos_info.id = rsp.get('data', {}).get('ID')
            qos_info.name = rsp.get('data', {}).get('NAME')
            qos_info.running_status = rsp.get('data', {}).get('RUNNINGSTATUS')
            qos_info.enable_status = (True if rsp.get('data', {}).get('ENABLESTATUS') == 'true' else False)
            qos_info.fs_id_list = rsp.get('data', {}).get('FSLIST')
            qos_info.lun_id_list = rsp.get('data', {}).get('LUNLIST')
            qos_info.schedule_policy = rsp.get('data', {}).get('SCHEDULEPOLICY')
            qos_info.policy_type = rsp.get('data', {}).get('POLICYTYPE')
            qos_info.schedule_start_time = rsp.get('data', {}).get('SCHEDULESTARTTIME')
            qos_info.start_time = rsp.get('data', {}).get('STARTTIME')
            qos_info.duration = rsp.get('data', {}).get('DURATION')
            # 条件返回
            try:
                self.get_qos_info(qos_info, rsp.get('data', {}))
            except ValueError as ex:
                log.error(f"Query qos rsp value exception:{Anonymity.process(str(ex))}.")
                raise CustomException(GeneralErrorCodes.ERR_INVALID_PARAM, ex) from ex
            log.info(f"Query qos success, id:{qos_id}, name:{qos_info.name}.")
            return ReturnCode.SUCCESS, ''
        else:
            log.info(f"Query qos id:{qos_id} not exist.")
            error_code, error_desc = ReturnCode.FAILED, f"Query qos id:{qos_id} not exist."
        return error_code, error_desc

    def get_qos_info(self, qos_info: SmartQoSInfo, rsp_data):
        qos_info.max_band_width = rsp_data.get('MAXBANDWIDTH', None)
        qos_info.min_band_width = rsp_data.get('MINBANDWIDTH', None)
        qos_info.burst_band_width = rsp_data.get('BURSTBANDWIDTH', None)
        qos_info.max_io_ps = rsp_data.get('MAXIOPS', None)
        qos_info.min_io_ps = rsp_data.get('MINIOPS', None)
        qos_info.burst_io_ps = rsp_data.get('BURSTIOPS', None)
        qos_info.latency = rsp_data.get('LATENCY', None)
        qos_info.burst_time = rsp_data.get('BURSTTIME', None)

    def update_smart_qos(self, qos_id, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/ioclass/{qos_id}'
        body = {}
        error_code, error_desc = self.construct_qos_param(qos_info, body)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Update qos fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Update qos success, id:{qos_id}.")
        return error_code, error_desc

    def construct_qos_param(self, qos_info: SmartQoSInfo, req_body: dict):
        try:
            self._check_smart_qos_param_valid(qos_info)
            self._set_smart_qos_param(qos_info, req_body)
            return ReturnCode.SUCCESS, ''
        except CustomException as e:
            return e.error_code, e.get_error_rsp()

    def active_smart_qos(self, qos_id, enable_status: bool):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/ioclass/active'
        req.body = json.dumps({
            'ID': qos_id,
            'ENABLESTATUS': 'true' if enable_status else 'false'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Active qos fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Active qos success, id:{qos_id}.")
        return error_code, error_desc

    def delete_smart_qos(self, qos_name):
        if not qos_name:
            error_desc = f'Qos name:{qos_name} is null.'
            return ReturnCode.FAILED, error_desc
        if len(qos_name) > QOS_NAME_MAX_LENGTH:
            qos_name = qos_name[:QOS_NAME_MAX_LENGTH]
        qos_info_tmp = SmartQoSInfo()
        try:
            error_code, error_desc = self.query_smart_qos(qos_name, qos_info_tmp)
        except CustomException as e:
            return e.error_code, e.get_error_rsp()
        if error_code == ReturnCode.FAILED:
            # 查询qos不存在，返回成功
            return ReturnCode.SUCCESS, ''
        if error_code == ReturnCode.SUCCESS:
            error_code, error_desc = self.active_smart_qos(qos_info_tmp.id, False)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
            error_code, error_desc = self._delete_qos_by_id(qos_info_tmp.id)
        return error_code, error_desc

    def delete_converged_qos(self, qos_name):
        if not qos_name:
            error_desc = f'Qos name:{qos_name} is null.'
            return ReturnCode.FAILED, error_desc
        if len(qos_name) > CONVERGED_QOS_NAME_MAX_LENGTH:
            qos_name = qos_name[:CONVERGED_QOS_NAME_MAX_LENGTH]
        qos_info_exist = SmartQoSInfo()
        try:
            error_code, error_desc = self.query_converged_qos_by_name(qos_name, qos_info_exist)
        except CustomException as e:
            return e.error_code, e.get_error_rsp()
        if error_code == ReturnCode.FAILED:
            # 查询qos不存在，返回成功
            return ReturnCode.SUCCESS, ''
        if error_code == ReturnCode.SUCCESS:
            error_code, error_desc = self._delete_converged_qos(qos_name, None, qos_info_exist.qos_scale)
        return error_code, error_desc

    def set_smart_qos_associate(self, qos_id, fs_id_list):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/ioclass/{qos_id}'
        req.body = json.dumps({
            'FSLIST': fs_id_list
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Set qos fs list fail, fs id list:{fs_id_list}, qos id:{qos_id}, "
                      f"error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Set qos fs list success, fs id list:{fs_id_list}, qos id:{qos_id}.")
        return error_code, error_desc

    def query_converged_qos_association_by_fsname(self, filesystem_name, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/converged_qos_association'
        body = {
            'fs_name': filesystem_name,
            'qos_scale': QosScale.NAMESPACE.value
        }
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query converged qos association fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and isinstance(rsp.get('data'), list) and rsp.get('data')[0].get('qos_policy_id'):
            qos_info.id = rsp.get('data').get('qos_policy_id')
            log.info(f"Query converged qos association of filesystem{filesystem_name} result, qos id: {qos_info.id}).")
        log.info(f"Query converged qos association of filesystem {filesystem_name} result: no associated qos.")
        return error_code, error_desc

    def query_converged_qos_by_name(self, qos_name, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/converged_qos_policy'
        body = {
            'name': qos_name,
            'qos_scale': QosScale.NAMESPACE.value
        }
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        req.body = json.dumps(body)
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.CONVERGED_QOS_NOT_EXIST:
            log.info(f"Query qos:{qos_name} not exist.")
            error_code, error_desc = ReturnCode.FAILED, f"Query qos {qos_name} not exist."
            return error_code, error_desc
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query smart qos fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and rsp.get('data').get('name') == qos_name:
            qos_info.id = rsp.get('data').get('id')
            qos_info.name = rsp.get('data').get('name')
            qos_info.basic_band_width_converged = rsp.get('data').get('basic_band_width')
            qos_info.max_band_width_converged = rsp.get('data').get('max_band_width')
            qos_info.max_iops_converged = rsp.get('data').get('max_iops')
            qos_info.package_size = rsp.get('data').get('package_size')
            qos_info.qos_mode = rsp.get('data').get('qos_mode')
            qos_info.qos_scale = rsp.get('data').get('qos_scale')
            log.info(f"Query qos success, name:{qos_name}, id: {qos_info.id}.")
            return ReturnCode.SUCCESS, ''
        else:
            log.info(f"Query converged qos:{qos_name} not exist.")
            error_code, error_desc = ReturnCode.FAILED, f"Query converged qos {qos_name} not exist."
        return error_code, error_desc

    def add_converged_qos_association(self, filesystem_name, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = f'/converged_qos_association'
        body = {
            'qos_scale': QosScale.NAMESPACE.value,
            'fs_name': filesystem_name,
            'id': qos_info.id
        }
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f"Add converged qos association success, fs name:{filesystem_name}, qos id:{qos_info.id}.")
        elif error_code != ReturnCode.SUCCESS:
            log.error(f"Add converged qos association fail, fs name:{filesystem_name}, qos id:{qos_info.id}, "
                      f"error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def delete_converged_qos_association(self, filesystem_name, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/converged_qos_association'
        body = {
            'qos_scale': QosScale.NAMESPACE.value,
            'fs_name': filesystem_name,
        }
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.PACIFIC_OBJECT_NAME_NOT_EXIST:
            # fs不存在关联的qos或者fs不存在报错
            log.error(f"Delete qos association fail, The entered object name does not exist. fs name:{filesystem_name}")
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete qos association fail, fs name:{filesystem_name}"
                      f"error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Delete converged qos association success, fs name:{filesystem_name}")
        return error_code, error_desc

    def update_converged_qos(self, qos_id, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/converged_qos_policy'
        body = {
            'name': qos_info.name
        }
        if qos_info.qos_mode:
            body['qos_mode'] = qos_info.qos_mode
        if qos_info.qos_scale:
            body['qos_scale'] = qos_info.qos_scale
        if qos_info.basic_band_width_converged:
            body['basic_band_width'] = qos_info.basic_band_width_converged
        if qos_info.max_band_width_converged:
            body['max_band_width'] = qos_info.max_band_width_converged
        if qos_info.package_size:
            body['package_size'] = qos_info.package_size
        if qos_info.max_iops_converged:
            body['max_io_ps'] = qos_info.max_iops_converged
        if qos_info.bps_density:
            body['bps_density'] = qos_info.bps_density
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        error_code, error_desc = self._check_converged_qos_param_valid(qos_info)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Update converged qos fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Update converged qos success, id:{qos_id}.")
        return error_code, error_desc

    def associate_query_replication_pair(self, obj_type, obj_id: str, pair_infos: List):
        """
        obj_type: 11=LUN, 40=file system
        obj_id: LUN id or file system id
        """
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/replicationpair/associate?ASSOCIATEOBJTYPE={obj_type}&ASSOCIATEOBJID={obj_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query replication pair fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and isinstance(rsp.get('data'), list) and rsp.get('data')[0].get('LOCALRESID') == obj_id:
            for data in rsp.get('data'):
                pair_info = ReplicationPairInfo()
                pair_info.pair_id = data.get('ID')
                pair_info.status = int(data.get('RUNNINGSTATUS', 0))
                pair_info.band_width = int(data.get('bandwidth', 0))
                pair_info.progress = int(data.get('REPLICATIONPROGRESS', 0))
                pair_info.secres_data_status = int(data.get('SECRESDATASTATUS', 0))
                pair_info.is_primary = (True if data.get('ISPRIMARY') == 'true' else False)
                pair_info.secres_access = int(data.get('SECRESACCESS', 0))
                pair_infos.append(pair_info)
            return ReturnCode.SUCCESS, ''
        else:
            log.info(f"Query replication pair not exist, obj_type:{obj_type}, obj_id:{obj_id}.")
            error_code, error_desc = ReturnCode.FAILED, f"Query replication pair not exist, " \
                                                        f"obj_type:{obj_type}, obj_id:{obj_id}."
            return error_code, error_desc

    def split_replication_pair_by_id(self, pair_id):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/REPLICATIONPAIR/split'
        req.body = json.dumps({
            'ID': pair_id
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Split replication pair fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Split replication pair success, pair id:{pair_id}.")
        return error_code, error_desc

    def delete_replication_pair_by_id(self, pair_id, is_local_delete=False):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/REPLICATIONPAIR'
        req.body = json.dumps({
            'ID': pair_id,
            'ISLOCALDELETE': 'true' if is_local_delete else 'false'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete replication pair fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Delete replication pair success, pair id:{pair_id}.")
        return error_code, error_desc

    def delete_obj_replication_pair(self, obj_type, obj_id: str):
        """
        obj_type: 11：LUN, 40: file system
        obj_id: LUN id or file system id
        """
        if os.getenv('DEPLOY_TYPE') in DEPLOY_TYPE_GROUP_NO_REPLICATION:
            return ReturnCode.SUCCESS, ''
        pair_infos = []
        error_code, error_desc = self.associate_query_replication_pair(obj_type, obj_id, pair_infos)
        if error_code == ReturnCode.FAILED:
            # replication pair not exist, return SUCCESS
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            # query replication pair error, return error_code
            return error_code, error_desc
        # replication pair exist
        for pair_info in pair_infos:
            if pair_info.status != RunningStatusEnum.INVALID:
                error_code, error_desc = self.split_replication_pair_by_id(pair_info.pair_id)
                if error_code != ReturnCode.SUCCESS:
                    return error_code, error_desc
            error_code, error_desc = self.delete_replication_pair_by_id(pair_info.pair_id)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
        return ReturnCode.SUCCESS, ''

    def _create_qos(self, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = f'/ioclass'
        body = {
            'NAME': qos_info.name,
            'FSLIST': qos_info.fs_id_list,
            'LUNLIST': qos_info.lun_id_list,
            'SCHEDULEPOLICY': qos_info.schedule_policy,
            'POLICYTYPE': qos_info.policy_type,
            'SCHEDULESTARTTIME': qos_info.schedule_start_time,
            'STARTTIME': qos_info.start_time,
            'DURATION': qos_info.duration
        }
        error_code, error_desc = self.construct_qos_param(qos_info, body)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Create qos fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and rsp.get('data').get('NAME') == qos_info.name:
            qos_info.id = rsp.get('data').get('ID')
            qos_info.running_status = rsp.get('data').get('RUNNINGSTATUS')
            qos_info.enable_status = (True if rsp.get('data').get('ENABLESTATUS') == 'true' else False)
            log.info(f"Create qos success, name:{qos_info.name}.")
        else:
            error_code, error_desc = ReturnCode.FAILED, f"Query qos response data not exist."
        return error_code, error_desc

    def _delete_qos_by_id(self, qos_id):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/ioclass/{qos_id}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete qos fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Delete qos success, id:{qos_id}.")
        return error_code, error_desc

    def _delete_converged_qos(self, qos_name, vstore_id, qos_scale=QosScale.NAMESPACE.value):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/converged_qos_policy'
        body = {
            'name': qos_name,
            'qos_scale': qos_scale
        }
        if vstore_id:
            body['vstoreId'] = vstore_id
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete qos fail, name:{qos_name},  error code: {error_code}, error desc: {error_desc}")
        else:
            log.info(f"Delete qos success, name:{qos_name}.")
        return error_code, error_desc

    def _create_converged_qos(self, qos_info: SmartQoSInfo):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = f'/converged_qos_policy'
        body = {
            'name': qos_info.name
        }
        if qos_info.qos_mode:
            body['qos_mode'] = qos_info.qos_mode
        if qos_info.qos_scale:
            body['qos_scale'] = qos_info.qos_scale
        if qos_info.basic_band_width_converged:
            body['basic_band_width'] = qos_info.basic_band_width_converged
        if qos_info.max_band_width_converged:
            body['max_band_width'] = qos_info.max_band_width_converged
        if qos_info.package_size:
            body['package_size'] = qos_info.package_size
        if qos_info.max_iops_converged:
            body['max_io_ps'] = qos_info.max_iops_converged
        if qos_info.bps_density:
            body['bps_density'] = qos_info.bps_density
        if qos_info.vstore_id:
            body['vstoreId'] = qos_info.vstore_id
        error_code, error_desc = self._check_converged_qos_param_valid(qos_info)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Create converged qos fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if rsp.get('data') and rsp.get('data').get('id'):
            qos_info.id = rsp.get('data').get('id')
            log.info(f"Create qos success, name:{qos_info.name}.")
        else:
            error_code, error_desc = ReturnCode.FAILED, f"Query qos response data not exist."
        return error_code, error_desc
