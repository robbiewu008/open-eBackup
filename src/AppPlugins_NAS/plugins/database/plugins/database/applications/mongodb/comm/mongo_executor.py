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

from time import sleep
import psutil

from bson import CodecOptions, Timestamp
from pymongo import MongoClient
from pymongo.errors import ConfigurationError, ConnectionFailure, OperationFailure, ServerSelectionTimeoutError, \
    AutoReconnect

from common import cleaner
from common.const import AuthType
from mongodb import LOGGER
from mongodb.comm.const import MongoRoles, DefaultValue, ParamField, MongodbErrorCode, MongoTool
from mongodb.comm.exception_err import DBConnectionError, DBOperationError, Error, DBAuthenticationError, \
    DBAuthorizedError


def parse_config_bool(item):
    if isinstance(item, bool):
        return item
    if isinstance(item, str):
        try:
            return item.strip().lower() == "true"
        except AttributeError:
            LOGGER.error("Parse config: %s failed, raise AttributeError.", item)
            return False
    return False


def parse_read_pref_tags(tags_str):
    kv_list = [kv_str.split(":", 1) for kv_str in tags_str.split(",")]
    tags = {kv[0].strip(): kv[1].strip() for kv in kv_list if len(kv) > 1}
    return tags


def get_mongo_bind_ip(port: int, service_ip: str):
    bind_ips = list()
    processes = psutil.process_iter()
    for process in processes:
        if process.name() != MongoTool.MONGOD and process.name() != MongoTool.MONGOS:
            continue
        conns = process.connections()
        for conn in conns:
            if int(conn.laddr.port) != int(port):
                continue
            ips = parse_ip_from_bind_ips(conn.laddr.ip, service_ip)
            bind_ips.extend(ips)
    if bind_ips:
        return bind_ips[0]
    return service_ip


def parse_ip_from_bind_ips(bind_ip, service_ip):
    ips = bind_ip.split(',')
    res_ips = list()
    for ip in ips:
        if DefaultValue.BIND_ALL_IP.value in ip:
            res_ips.append(service_ip)
        else:
            res_ips.append(ip)
    return res_ips


class DB:
    def __init__(self, uri, config, do_replset=False, read_pref='primaryPreferred', do_rp_tags=False,
                 do_connect=True, conn_timeout=5000, retries=5, direct_connection=False):
        """
        :param uri: mongo实例的ip和port
        :param config: 认证配置信息
        :param do_replset: 启用副本集群配置
        :param read_pref: 读优先
        :param do_rp_tags: 是否读取标签
        :param do_connect: 建立连接
        :param conn_timeout: 连接超时时间
        :param retries: 连接重试次数
        :param direct_connection: 直连本机实例
        """
        self.uri = uri
        self.config = config
        self.do_replset = do_replset
        self.read_pref = read_pref
        self.do_rp_tags = do_rp_tags
        self.do_connect = do_connect
        self.conn_timeout = conn_timeout
        self.retries = retries
        self.direct_connection = direct_connection
        self.username = config.get("username")
        self.password = config.get("password")
        self.authdb = "admin"
        self.auth_type = config.get("auth_type")
        self.replset = None
        self._conn = None
        self._is_master = None
        self.wait_secs = 30
        self.balancer_sleep = 3

    def client_opts(self):
        opts = {
            "connect": self.do_connect,
            "host": self.uri.split(":")[0],
            "port": int(self.uri.split(":")[1]),
            "connectTimeoutMS": self.conn_timeout,
            "serverSelectionTimeoutMS": self.conn_timeout,
            "maxPoolSize": 1,
            "directConnection": self.direct_connection
        }
        if not self.auth_type:
            return opts
        if self.auth_type == str(AuthType.APP_PASSWORD.value):
            opts.update({
                "username": self.username,
                "password": self.password
            })
        return opts

    def connect(self):
        try:
            LOGGER.debug(
                "Getting MongoDB connection to %s (replicaSet=%s, readPreference=%s, readPreferenceTags=%s)",
                self.uri,
                self.replset,
                self.read_pref,
                self.do_rp_tags,
            )
            conn = MongoClient(**self.client_opts())
            if self.do_connect:
                conn['admin'].command({"ping": 1})
        except (ConfigurationError, ConnectionFailure, ServerSelectionTimeoutError) as ex:
            LOGGER.error("Ip, port, network or server shut down when connect to %s! Error: %s", self.uri, ex)
            raise DBConnectionError(ex) from ex
        except OperationFailure as ex:
            if ex.code == MongodbErrorCode.AUTH_ERROR.value:
                LOGGER.error("Authentication error when connect to %s, Error: %s", self.uri, ex)
                raise DBAuthenticationError(ex) from ex
            raise DBConnectionError(ex) from ex
        if conn is not None:
            self._conn = conn
        return self._conn

    def admin_command(self, admin_command, quiet=False):
        tries = 0
        status = None
        if not self._conn:
            self.connect()
        while not status and tries < self.retries:
            try:
                status = self._conn['admin'].command(admin_command)
            except OperationFailure as e:
                if not quiet:
                    LOGGER.error("Error running admin command '%s': %s", admin_command, e)
                if e.code == MongodbErrorCode.NOT_AUTHORIZED_ERROR.value:
                    raise DBAuthorizedError(
                        "Not authorized on admin to execute command '%s': %s" % (admin_command, e)) from e
                tries += 1
                sleep(1)
        if not status:
            raise DBOperationError(
                "Could not get output from command: '%s' after %i retries!" % (admin_command, self.retries))
        return status

    def is_primary(self):
        if not self._conn:
            self.connect()
        return self._conn.is_primary

    def server_version(self):
        status = self.admin_command('serverStatus')
        try:
            if 'version' in status:
                version = status['version'].split('-')[0]
                return ".".join(version.split(".")[0:2])
        except Exception as e:
            raise Error("Unable to determine version from serverStatus! Error: %s.", e) from e
        return ""

    def shutdown_server(self):
        try:
            self.admin_command({'shutdown': 1, 'force': 'true'})
        except AutoReconnect as ex:
            LOGGER.debug("Shut down mongo server success! msg: %s.", ex)

    def initiate_cluster(self, param):
        try:
            self.admin_command({'replSetInitiate': param})
        except Exception as e:
            LOGGER.error("ReplSetInitiate server failed! msg: (%s), type : %s", e, type(e))
            raise Error("Unable to init cluster server! Error: %s" % e) from e

    def is_mongos(self):
        return self._conn.is_mongos

    def get_inst_role(self):
        """
        查询分片集群的服务类型 实例为single和复制集集群replication
        """
        config_svr = MongoRoles.CONFIG_SVR
        shards_vr = MongoRoles.SHARDS_VR
        line_opts = self.admin_command("getCmdLineOpts")
        if not line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING):
            if line_opts.get(MongoRoles.PARSED).get(MongoRoles.REPLICATION):
                return MongoRoles.REPLICATION
            return MongoRoles.SINGLE
        if line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CONFIG_DB):
            return MongoRoles.MONGOS
        if line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
            if config_svr == line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
                return MongoRoles.CONFIG
            if shards_vr == line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
                return MongoRoles.SHARD
        return ""

    def close(self):
        if self._conn:
            LOGGER.debug("Closing connection to: %s", self.uri)
            self._conn.close()

    def clean(self):
        cleaner.clear(self.password)

    def get_data_path(self):
        """
        查询备份数据信息
        """
        line_opts = self.admin_command("getCmdLineOpts")
        if line_opts.get(MongoRoles.PARSED).get("storage").get("dbPath"):
            return line_opts.get(MongoRoles.PARSED).get("storage").get("dbPath")
        return ""

    def set_balancer(self, flag):
        status = "balancerStart" if flag else "balancerStop"
        status = self.admin_command(status)
        return status is None

    def check_balance(self):
        balance_state = self.admin_command("balancerStatus")
        return balance_state.get("inBalancerRound")

    def stop_balance(self):
        wait_cnt = 0
        self.set_balancer(False)
        while wait_cnt < self.wait_secs:
            if self.check_balance():
                wait_cnt += self.balancer_sleep
                sleep(self.balancer_sleep)
            else:
                LOGGER.info("Balancer stopped success.")
                return True
        err_msg = "Could not stop balancer %s!" % self.uri
        LOGGER.error(err_msg)
        return False

    def drop_local_database(self):
        if not self._conn:
            self.connect()
        try:
            self._conn.drop_database('local')
        except Exception as e:
            LOGGER.error("Unable to drop local database! Error: %s, type : %s", e, type(e))
            raise Error("Unable to drop local database! Error: %s" % e) from e

    def get_oplog(self):
        if not self._conn:
            self.connect()
        local_db = self._conn.get_database(DefaultValue.LOCAL_DB.value)
        collections = local_db.list_collection_names()
        oplog = None
        if DefaultValue.OPLOG_COLLECTION.value in collections:
            oplog = local_db.get_collection(DefaultValue.OPLOG_COLLECTION.value)
        return oplog

    def get_oplog_rs(self):
        oplog = self.get_oplog()
        if oplog is None:
            raise DBOperationError("Not support oplog.")
        return oplog.with_options(codec_options=CodecOptions(unicode_decode_error_handler="ignore"))

    def get_oplog_tail_ts(self):
        return self.get_oplog_rs().find_one(sort=[('$natural', -1)])['ts']

    def query_oplog_ts(self, lsn):
        oplog = self.get_oplog_rs()
        if isinstance(lsn, str):
            t, ind = map(int, lsn.split(",", 1))
            ts = Timestamp(t, ind)
        elif isinstance(lsn, tuple) or isinstance(lsn, list):
            if len(lsn) != 2:
                raise ValueError("Lsn value err,must have two member as time and inc.")
            if not all(map(lambda x: isinstance(x, int), lsn)):
                raise ValueError("Lsn value err, all member must be int.")
            ts = Timestamp(lsn[0], lsn[1])
        else:
            raise ValueError("Not support lsn type with query oplog ts.")
        query = {"ts": ts}
        return oplog.find_one(query)

    def get_oplog_with_query(self, query=None):
        cour = self.get_oplog_rs()
        return cour.find_one(query, sort=[('$natural', -1)])

    def watch_stream(self):
        if not self._conn:
            self.connect()
        self._conn.watch()

    def update_config_info(self, shard_info: dict):
        if not self._conn:
            self.connect()
        try:
            for shard_id, host in shard_info.items():
                # update_one返回结果形式：{"acknowledged": true, "matchedCount": 0, "modifiedCount": 0}
                result = self._conn['config'].shards.update_one({"_id": shard_id}, {"$set": {"host": host}})
                if result.matched_count == 0:
                    LOGGER.error("Unable to update config node info, because matched count is 0.")
                    raise Error("Unable to update config node info, because matched count is 0.")
                if result.modified_count == 0:
                    LOGGER.warn("Unable to update config node info, because modified count is 0. ")
        except Exception as e:
            LOGGER.error("Unable to update config node info!")
            raise Error("Unable to update config node info! Error: %s" % e) from e

    def update_shard_info(self, config_info: str):
        if not self._conn:
            self.connect()
        try:
            result = self._conn['admin'].system.version.update_one({"_id": "shardIdentity"},
                                                                   {"$set": {"configsvrConnectionString": config_info}})
            if result.matched_count == 0:
                LOGGER.error("Unable to update shard node info, because matched count is 0.")
                raise Error("Unable to update shard node info, because matched count is 0.")
            if result.modified_count == 0:
                LOGGER.warn("Unable to update shard node info, because modified count is 0. ")

            delete_result = self._conn['admin'].system.version.delete_one({"_id": "minOpTimeRecovery"})
            if delete_result.deleted_count == 0:
                LOGGER.warn("Unable to delete minOpTimeRecovery, because deleted count is 0.")

        except Exception as e:
            raise Error("Unable to drop local database! Error: %s" % e) from e

    def get_replset_status(self):
        status = self.admin_command(ParamField.REPLSET_GET_STATUS)
        LOGGER.info("Start get replication server status info.")
        return status

    def get_cmd_line_opts(self):
        line_opts = self.admin_command(ParamField.GET_CMDLINE_OPTS)
        LOGGER.info("Start get cmd line opts info.")
        return line_opts
