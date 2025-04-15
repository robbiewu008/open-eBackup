#!/usr/bin/python3
# -*- coding: utf-8 -*-

import requests

from ansible_collections.oceanprotect.ansible.plugins.module_utils.rest_api.RestApi import RestApi

requests.packages.urllib3.disable_warnings()


class PmManager(object):
    """
    自动化接口关键字基类
    """

    def __init__(self, ip, user='', pwd=''):
        """
        初始化需要完成鉴权
        :return:
        """
        super(PmManager, self).__init__()
        self.base_url = "https://{}:25081".format(ip)
        self.api = RestApi(ip=ip, username=user, password=pwd, scope=0)
        self.api.login()

    def resourceShow(self, params):
        """ 资源查询
            Args:
                'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页页面编码'},
                'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页数据条数'},
                'orders': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'排序字段：display_timestamp'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'条件参数：%location%，%resource_name%，%sla_name%，%resource_location%，!properties，!resource_properties，!sla_properties'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'}
            Returns:
                Response of the API.
                Format: [{'partial': 0, 'parser': xxx, 'rc': xx, 'stderr': xxx, 'stdout': xxx}]
            Examples:
                self.storDev.dispatch('oracleCopyShow', params, conditions, primary_key)
            """
        api = {
            'resource': '%s/v2/resources' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'pageNo': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'页索引'},
                'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'页大小'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'查询条件(json格式)'},
                'orders': {'optional': 'False', 'location': 'query', 'types': 'array',
                           'description': r'排序字段（格式：升序为+field，降序为-field）'}},
        }

        status_code, rsp = self.api.restRequest(api, params)
        return status_code, rsp

    def hostAgentGet(self, params):
        """ 带条件查询agent信息
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'pageNo': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'页索引，默认0'},
            'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'页大小，默认10'},
            'conditions': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'查询条件(json格式)，如：{"type":"Host","subType":["DBBackupAgent","VMBackupAgent","UBackupAgent","SBackupAgent"],"isCluster":false}'}
        """
        api = {
            'resource': '%s/v1/host-agent' % self.base_url,
            'method': 'get',
            'params': {
                'pageNo': {'optional': 'False', 'location': 'query', 'types': 'integer',
                           'description': r'页索引，默认0'},
                'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer',
                             'description': r'页大小，默认10'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'查询条件(json格式)，如：{"type":"Host","subType":["DBBackupAgent","VMBackupAgent","UBackupAgent","SBackupAgent"],"isCluster":false}'}},
        }
        conditions = {"type": "Host", "subType": ["DBBackupAgent", "VMBackupAgent", "UBackupAgent", "SBackupAgent"],
                      "scenario": "0", "isCluster": False, "endpoint": "11"}

        for k, v in params.items():
            if conditions.get(k):
                conditions[k] = v
        params = {'pageSize': 200, 'conditions': conditions}
        status_code, rsp = self.api.restRequest(api, params)
        return status_code, rsp

    def hostAgentRegisterPost(self, params):
        """ 客户端代理自动注册
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'params': {'optional': 'True', 'location': 'body', 'types': '', 'description': r'客户端代理自动注册参数'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/host-agent/register' % self.base_url,
            'method': 'post',
            'params': {
                'params': {'optional': 'True', 'location': 'body', 'types': '',
                           'description': r'客户端代理自动注册参数'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def storagesGet(self, params):
        """ 查询存储库信息
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页数量'},
            'startPage': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页起始页面'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/storages' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'Access token'},
                'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页数量'},
                'startPage': {'optional': 'False', 'location': 'query', 'types': 'integer',
                              'description': r'分页起始页面'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def storagesStorageidDelete(self, params):
        """ 删除存储库
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'storageId': {'optional': 'False', 'location': 'path', 'types': 'string', 'description': r'存储id'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/storages/{storageId}' % self.base_url,
            'method': 'delete',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'Access token'},
                'storageId': {'optional': 'False', 'location': 'path', 'types': 'string', 'description': r'存储id'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def storagesPost(self, params):
        """ 创建存储库
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'request': {'optional': 'False', 'location': 'body', 'types': '', 'description': r'创建存储库请求参数'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/storages' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'Access token'},
                'request': {'optional': 'False', 'location': 'body', 'types': '',
                            'description': r'创建存储库请求参数'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def environmentsPost(self, params):
        """ 注册资源
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'访问令牌'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/environments' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'访问令牌'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def environmentsGet(self, params):
        """ 查询主机资源信息列表
        Args:
            'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页页面编码'},
            'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页数据条数'},
            'orders': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'排序字段：'},
            'conditions': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'条件参数：'},
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/environments' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer',
                            'description': r'分页页面编码'},
                'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer',
                              'description': r'分页数据条数'},
                'orders': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'排序字段：'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'条件参数：'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def slasPost(self, params):
        """ 创建新的SLA
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'请求token'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/slas' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'请求token'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def slasGet(self, params):
        """ 分页查询SLA信息
        Args:
            'name': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'SLA名称，支持模糊查询'},
            'types': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'sla类型，支持筛选过滤'},
            'applications': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'sla应用类型，支持过筛选过滤'},
            'actions': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'备份模式列表'},
            'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页页面编码'},
            'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页数据条数'},
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'请求token'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/slas' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'name': {'optional': 'False', 'location': 'query', 'types': 'string',
                         'description': r'SLA名称，支持模糊查询'},
                'types': {'optional': 'False', 'location': 'query', 'types': 'array',
                          'description': r'sla类型，支持筛选过滤'},
                'applications': {'optional': 'False', 'location': 'query', 'types': 'array',
                                 'description': r'sla应用类型，支持过筛选过滤'},
                'actions': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'备份模式列表'},
                'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer',
                            'description': r'分页页面编码'},
                'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer',
                              'description': r'分页数据条数'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'请求token'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def protectedObjectsBatchPost(self, params):
        """ 批量创建保护对象
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'访问令牌'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/protected-objects/batch' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'访问令牌'},
                'akOperationTips': {'optional': 'True', 'location': 'path', 'types': 'string', 'description': r''},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def resourcesPost(self, params):
        """ 创建资源
        Args:
            'CreateResourceRequestBody': {'optional': 'True', 'location': 'body', 'types': '', 'description': r'创建资源的请求体'}
        Returns:
            Response of the API.
            Format: [{'partial': 0, 'parser': xxx, 'rc': xx, 'stderr': xxx, 'stdout': xxx}]
        Examples:
            self.storDev.dispatch('resourcesPost', params)
        """
        api = {
            'resource': '%s/v2/resources' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'CreateResourceRequestBody': {'optional': 'True', 'location': 'body', 'types': '',
                                              'description': r'创建资源的请求体'}},
        }
        status_code, rsp = self.api.restRequest(api, params)
        return status_code, rsp

    def protectedObjectsDelete(self, params):
        """ 批量移除保护
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/protected-objects' % self.base_url,
            'method': 'delete',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def virtualMachineShow(self, params):
        """ A8000关联的VirtualMachine查询
        Args:
            'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页页面编码'},
            'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页数据条数'},
            'orders': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'排序字段：display_timestamp'},
            'conditions': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'条件参数：%location%，%resource_name%，%sla_name%，%resource_location%，!properties，!resource_properties，!sla_properties'},
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """

        api = {
            'resource': '%s/v1/virtual-resource' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer',
                            'description': r'分页页面编码'},
                'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer',
                              'description': r'分页数据条数'},
                'orders': {'optional': 'False', 'location': 'query', 'types': 'array',
                           'description': r'排序字段：children'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'条件参数：'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'访问令牌'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def protectedObjectsResourceIdActionBackupPost(self, params):
        """ 手动备份资源
        Args:
            'resource_id': {'optional': 'True', 'location': 'path', 'types': 'string', 'description': r'保护对象资源ID'},
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'访问令牌'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/protected-objects/{resource_id}/action/backup' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'resource_id': {'optional': 'True', 'location': 'path', 'types': 'string',
                                'description': r'保护对象资源ID'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'访问令牌'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def jobsGet(self, params):
        """ 查询任务
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'Access token'},
            'copyId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'副本id'},
            'fromCopyTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'副本生成时间(按范围查询的开始时间)'},
            'fromEndTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'任务结束时间(按范围查询的开始时间)'},
            'fromStartTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'任务创建时间(按范围查询的开始时间)'},
            'isSystem': {'optional': 'False', 'location': 'query', 'types': 'boolean', 'description': r'是否为系统任务。是：true，不是：false'},
            'isVisible': {'optional': 'False', 'location': 'query', 'types': 'boolean', 'description': r'是否展示任务'},
            'jobId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务id'},
            'orderBy': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'排序字段'},
            'orderType': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'排序类型'},
            'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页数量'},
            'sourceId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务对象ID'},
            'sourceLocation': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务对象位置'},
            'sourceName': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务对象名称'},
            'sourceTypes': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'资源子类型[取值范围]ABBackupClient:主机 Fileset:文件集 DB2:DB2 SQLServer:SQLServer  MySQL:MySQL GaussDB:GaussDB  Oracle:Oracle数据库 Volume:主机卷 VMWare:VMware虚拟化平台 vim.VirtualMachin:VMware虚拟机 vim.HostSystem:VMware主机系统 vim.ClusterComputeResource:集群 Hyper-V:Hyper-V虚拟化平台 ms.VirtualMachine:Hyper-V虚拟机 ms.HostSystem:Hyper-V主机系统'},
            'startPage': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页起始页面'},
            'statusList': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'任务状态[取值范围]READY:准备中 PENDING排队中 RUNNING:运行中 SUCCESS:成功 PARTIAL_SUCCESS:部分成功 ABORTED: 停止 ABORTING: 停止中 FAIL:失败 ABNORMAL:异常 CANCELLED:取消 ABORT_FAILED:停止失败'},
            'targetLocation': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务目标结果位置'},
            'targetName': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务目标结果对象'},
            'toCopyTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'副本生成时间(按范围查询的结束时间)'},
            'toEndTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'任务结束时间(按范围查询的结束时间)'},
            'toStartTime': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'任务创建时间(按范围查询的结束时间)'},
            'types': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'任务类型[取值范围]BACKUP:备份 RESTORE:恢复 INSTANT_RESTORE:即时恢复 live_mount:即时挂载 copy_replication:副本复制 archive:归档 COPY_DELETE:副本删除 COPY_EXPIRE:副本过期 unmount:卸载 DB_IDENTIFICATION:敏感数据识别 DB_DESESITIZATION:数据脱敏 archive_import:副本导入 migrate:迁移 resource_scan:注册 resource_protection:资源保护 resource_protection_modify:修改保护'}
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/jobs' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'Access token'},
                'copyId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'副本id'},
                'fromCopyTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                                 'description': r'副本生成时间(按范围查询的开始时间)'},
                'fromEndTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                                'description': r'任务结束时间(按范围查询的开始时间)'},
                'fromStartTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                                  'description': r'任务创建时间(按范围查询的开始时间)'},
                'isSystem': {'optional': 'False', 'location': 'query', 'types': 'boolean',
                             'description': r'是否为系统任务。是：true，不是：false'},
                'isVisible': {'optional': 'False', 'location': 'query', 'types': 'boolean',
                              'description': r'是否展示任务'},
                'jobId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务id'},
                'orderBy': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'排序字段'},
                'orderType': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'排序类型'},
                'pageSize': {'optional': 'False', 'location': 'query', 'types': 'integer', 'description': r'分页数量'},
                'sourceId': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'任务对象ID'},
                'sourceLocation': {'optional': 'False', 'location': 'query', 'types': 'string',
                                   'description': r'任务对象位置'},
                'sourceName': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'任务对象名称'},
                'sourceTypes': {'optional': 'False', 'location': 'query', 'types': 'array',
                                'description': r'资源子类型[取值范围]ABBackupClient:主机 Fileset:文件集 DB2:DB2 SQLServer:SQLServer  MySQL:MySQL GaussDB:GaussDB  Oracle:Oracle数据库 Volume:主机卷 VMWare:VMware虚拟化平台 vim.VirtualMachin:VMware虚拟机 vim.HostSystem:VMware主机系统 vim.ClusterComputeResource:集群 Hyper-V:Hyper-V虚拟化平台 ms.VirtualMachine:Hyper-V虚拟机 ms.HostSystem:Hyper-V主机系统'},
                'startPage': {'optional': 'False', 'location': 'query', 'types': 'integer',
                              'description': r'分页起始页面'},
                'statusList': {'optional': 'False', 'location': 'query', 'types': 'array',
                               'description': r'任务状态[取值范围]READY:准备中 PENDING排队中 RUNNING:运行中 SUCCESS:成功 PARTIAL_SUCCESS:部分成功 ABORTED: 停止 ABORTING: 停止中 FAIL:失败 ABNORMAL:异常 CANCELLED:取消 ABORT_FAILED:停止失败'},
                'targetLocation': {'optional': 'False', 'location': 'query', 'types': 'string',
                                   'description': r'任务目标结果位置'},
                'targetName': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'任务目标结果对象'},
                'toCopyTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                               'description': r'副本生成时间(按范围查询的结束时间)'},
                'toEndTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                              'description': r'任务结束时间(按范围查询的结束时间)'},
                'toStartTime': {'optional': 'False', 'location': 'query', 'types': 'integer',
                                'description': r'任务创建时间(按范围查询的结束时间)'},
                'types': {'optional': 'False', 'location': 'query', 'types': 'array',
                          'description': r'任务类型[取值范围]BACKUP:备份 RESTORE:恢复 INSTANT_RESTORE:即时恢复 live_mount:即时挂载 copy_replication:副本复制 archive:归档 COPY_DELETE:副本删除 COPY_EXPIRE:副本过期 unmount:卸载 DB_IDENTIFICATION:敏感数据识别 DB_DESESITIZATION:数据脱敏 archive_import:副本导入 migrate:迁移 resource_scan:注册 resource_protection:资源保护 resource_protection_modify:修改保护'}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def copiesShow(self, params):
        """ 副本列表查询
        Args:
            'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页页面编码'},
            'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer', 'description': r'分页数据条数'},
            'orders': {'optional': 'False', 'location': 'query', 'types': 'array', 'description': r'排序字段：display_timestamp'},
            'conditions': {'optional': 'False', 'location': 'query', 'types': 'string', 'description': r'条件参数：%location%，%resource_name%，%sla_name%，%resource_location%，!properties，!resource_properties，!sla_properties'},
        Returns:
            Response of the API.
            Format:
        Examples:

        """
        api = {
            'resource': '%s/v1/copies' % self.base_url,
            'method': 'get',
            'auth_type': 'pm',
            'params': {
                'page_no': {'optional': 'True', 'location': 'query', 'types': 'integer',
                            'description': r'分页页面编码'},
                'page_size': {'optional': 'True', 'location': 'query', 'types': 'integer',
                              'description': r'分页数据条数'},
                'orders': {'optional': 'False', 'location': 'query', 'types': 'string',
                           'description': r'排序字段：display_timestamp'},
                'conditions': {'optional': 'False', 'location': 'query', 'types': 'string',
                               'description': r'条件参数：%location%，%resource_name%，%sla_name%，%resource_location%，!properties，!resource_properties，!sla_properties'},
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': '', 'description': r'访问令牌'}},
        }

        rsp = self.api.restRequest(api, params)
        return rsp

    def restoresPost(self, params):
        """ 创建恢复（vmware)
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'授权token'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format: [{'partial': 0, 'parser': xxx, 'rc': xx, 'stderr': xxx, 'stdout': xxx}]
        Examples:
            self.storDev.dispatch('restoresPost', params)
        """
        api = {
            'resource': '%s/v1/restores' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'授权token'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp

    def restoresPost_V2(self, params):
        """ 创建恢复(文件集）
        Args:
            'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string', 'description': r'授权token'},
            'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}
        Returns:
            Response of the API.
            Format: [{'partial': 0, 'parser': xxx, 'rc': xx, 'stderr': xxx, 'stdout': xxx}]
        Examples:
            self.storDev.dispatch('restoresPost', params)
        """
        api = {
            'resource': '%s/v2/restore/jobs' % self.base_url,
            'method': 'post',
            'auth_type': 'pm',
            'params': {
                'X-Auth-Token': {'optional': 'True', 'location': 'header', 'types': 'string',
                                 'description': r'授权token'},
                'body': {'optional': 'True', 'location': 'body', 'types': '', 'description': r''}},
        }
        rsp = self.api.restRequest(api, params)
        return rsp
