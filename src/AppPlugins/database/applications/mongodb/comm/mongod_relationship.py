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

class MongodCommandRelationship:
    """
    mongod 启动命令映射键值对, key为parsed data中的字段, value为yaml启动文件中的key, 为了避免启动文件中key字段不对应导致启动实例失效,
    做了一个key映射,将yaml文件字段替换为有效字段
    """
    data = {
        'auditLog.auditCompressionMode': 'auditLog.compressionMode',
        'auditLog.auditDestination': 'auditLog.destination',
        'auditLog.auditEncryptionKeyUID': 'auditLog.auditEncryptionKeyIdentifier',
        'auditLog.auditFilter': 'auditLog.filter',
        'auditLog.auditFormat': 'auditLog.format',
        'auditLog.auditLocalKeyFile': 'auditLog.localAuditKeyFile',
        'auditLog.auditPath': 'auditLog.path',
        'cloud.enableFreeMonitoring': 'cloud.monitoring.free.state',
        'cloud.freeMonitoringTag': 'cloud.monitoring.free.tags',
        'net.bind_ip': 'net.bindIp',
        'net.bind_ip_all': 'net.bindIpAll',
        'net.filePermissions': 'net.unixDomainSocket.filePermissions',
        'net.maxConns': 'net.maxIncomingConnections',
        'net.networkMessageCompressors': 'net.compression.compressors',
        'net.nounixsocket': 'net.unixDomainSocket.enabled',
        'net.sslOnNormalPorts': 'net.ssl.sslOnNormalPorts',
        'net.tlsAllowConnectionsWithoutCertificates': 'net.tls.allowConnectionsWithoutCertificates',
        'net.tlsAllowInvalidCertificates': 'net.tls.allowInvalidCertificates',
        'net.tlsAllowInvalidHostnames': 'net.tls.allowInvalidHostnames',
        'net.tlsCAFile': 'net.tls.CAFile',
        'net.tlsCRLFile': 'net.tls.CRLFile',
        'net.tlsCertificateKeyFile': 'net.tls.certificateKeyFile',
        'net.tlsCertificateKeyFilePassword': 'net.tls.certificateKeyFilePassword',
        'net.tlsCertificateSelector': 'net.tls.certificateSelector',
        'net.tlsClusterCAFile': 'net.tls.clusterCAFile',
        'net.tlsClusterCertificateSelector': 'net.tls.clusterCertificateSelector',
        'net.tlsClusterFile': 'net.tls.clusterFile',
        'net.tlsClusterPassword': 'net.tls.clusterPassword',
        'net.tlsDisabledProtocols': 'net.tls.disabledProtocols',
        'net.tlsFIPSMode': 'net.tls.FIPSMode',
        'net.tlsMode': 'net.tls.mode',
        'net.unixSocketPrefix': 'net.unixDomainSocket.pathPrefix',
        'operationProfiling.profile': 'operationProfiling.mode',
        'operationProfiling.slowms': 'operationProfiling.slowOpThresholdMs',
        'processManagement.pidfilepath': 'processManagement.pidFilePath',
        'processManagement.serviceDescription': 'processManagement.windowsService.description',
        'processManagement.serviceDisplayName': 'processManagement.windowsService.displayName',
        'processManagement.serviceName': 'processManagement.windowsService.serviceName',
        'processManagement.servicePassword': 'processManagement.windowsService.servicePassword',
        'processManagement.serviceUser': 'processManagement.windowsService.serviceUser',
        'replication.oplogSize': 'replication.oplogSizeMB',
        'replication.replSet': 'replication.replSetName',
        'security.auth': 'security.authorization',
        'security.kmipActivateKeys': 'security.kmip.activateKeys',
        'security.kmipClientCertificateFile': 'security.kmip.clientCertificateFile',
        'security.kmipClientCertificatePassword': 'security.kmip.clientCertificatePassword',
        'security.kmipClientCertificateSelector': 'security.kmip.clientCertificateSelector',
        'security.kmipConnectRetries': 'security.kmip.connectRetries',
        'security.kmipConnectTimeoutMS': 'security.kmip.connectTimeoutMS',
        'security.kmipKeyIdentifier': 'security.kmip.keyIdentifier',
        'security.kmipKeyStatePollingSeconds': 'security.kmip.keyStatePollingSeconds',
        'security.kmipPort': 'security.kmip.port',
        'security.kmipRotateMasterKey': 'security.kmip.rotateMasterKey',
        'security.kmipServerCAFile': 'security.kmip.serverCAFile',
        'security.kmipServerName': 'security.kmip.serverName',
        'security.ldapAuthzQueryTemplate': 'security.ldap.authz.queryTemplate',
        'security.ldapBindMethod': 'security.ldap.bind.method',
        'security.ldapBindSaslMechanisms': 'security.ldap.bind.saslMechanisms',
        'security.ldapBindWithOSDefaults': 'security.ldap.bind.useOSDefaults',
        'security.ldapQueryPassword': 'security.ldap.bind.queryPassword',
        'security.ldapQueryUser': 'security.ldap.bind.queryUser',
        'security.ldapServers': 'security.ldap.servers',
        'security.ldapTimeoutMS': 'security.ldap.timeoutMS',
        'security.ldapTransportSecurity': 'security.ldap.transportSecurity',
        'security.ldapUserToDNMapping': 'security.ldap.userToDNMapping',
        'security.noscripting': 'security.javascriptEnabled',
        'security.setParameter saslHostName=...': 'security.sasl.hostName',
        'security.setParameter saslServiceName=...': 'security.sasl.serviceName',
        'security.setParameter saslauthdPath=...': 'security.sasl.saslauthdSocketPath',
        'setParameter.setParameter': 'setParameter',
        'sharding.configsvr': 'sharding.clusterRole',
        'sharding.moveParanoia': 'sharding.archiveMovedChunks',
        'sharding.noMoveParanoia': 'sharding.archiveMovedChunks',
        'sharding.shardsvr': 'sharding.clusterRole',
        'snmp.snmp-disabled': 'snmp.disabled',
        'snmp.snmp-master': 'snmp.master',
        'snmp.snmp-subagent': 'snmp.subagent',
        'storage.dbpath': 'storage.dbPath',
        'storage.directoryperdb': 'storage.directoryPerDB',
        'storage.inMemorySizeGB': 'storage.inMemory.engineConfig.inMemorySizeGB',
        'storage.journal': 'storage.journal.enabled',
        'storage.journalCommitInterval': 'storage.journal.commitIntervalMs',
        'storage.nojournal': 'storage.journal.enabled',
        'storage.storageEngine': 'storage.engine',
        'storage.syncdelay': 'storage.syncPeriodSecs',
        'storage.wiredTigerCacheSizeGB': 'storage.wiredTiger.engineConfig.cacheSizeGB',
        'storage.wiredTigerCollectionBlockCompressor': 'storage.wiredTiger.collectionConfig.blockCompressor',
        'storage.wiredTigerDirectoryForIndexes': 'storage.wiredTiger.engineConfig.directoryForIndexes',
        'storage.wiredTigerIndexPrefixCompression': 'storage.wiredTiger.indexConfig.prefixCompression',
        'storage.wiredTigerJournalCompressor': 'storage.wiredTiger.engineConfig.journalCompressor',
        'systemLog.logappend': 'systemLog.logAppend',
        'systemLog.logpath': 'systemLog.path',
        'systemLog.setParameter "logComponentVerbosity={accessControl: ... }"':
            'systemLog.component.accessControl.verbosity',
        'systemLog.setParameter "logComponentVerbosity={command: ... }"':
            'systemLog.component.command.verbosity',
        'systemLog.setParameter "logComponentVerbosity={control: ... }"':
            'systemLog.component.control.verbosity',
        'systemLog.setParameter "logComponentVerbosity={ftdc: ... }"':
            'systemLog.component.ftdc.verbosity',
        'systemLog.setParameter "logComponentVerbosity={geo: ... }"':
            'systemLog.component.geo.verbosity',
        'systemLog.setParameter "logComponentVerbosity={index: ... }"':
            'systemLog.component.index.verbosity',
        'systemLog.setParameter "logComponentVerbosity={network: ... }"':
            'systemLog.component.network.verbosity',
        'systemLog.setParameter "logComponentVerbosity={query: ... }"':
            'systemLog.component.query.verbosity',
        'systemLog.setParameter "logComponentVerbosity={replication: ... }"':
            'systemLog.component.replication.verbosity',
        'systemLog.setParameter "logComponentVerbosity={replication: {heartbeats: ... }}"':
            'systemLog.component.replication.heartbeats.verbosity',
        'systemLog.setParameter "logComponentVerbosity={replication: {rollback: ... }}"':
            'systemLog.component.replication.rollback.verbosity',
        'systemLog.setParameter "logComponentVerbosity={sharding: ... }"':
            'systemLog.component.sharding.verbosity',
        'systemLog.setParameter "logComponentVerbosity={storage: ... }"':
            'systemLog.component.storage.verbosity',
        'systemLog.setParameter "logComponentVerbosity={storage: {journal: ... }}"':
            'systemLog.component.storage.journal.verbosity',
        'systemLog.setParameter "logComponentVerbosity={storage: {recovery: ... }}"':
            'systemLog.component.storage.recovery.verbosity',
        'systemLog.setParameter "logComponentVerbosity={write: ... }"': 'systemLog.component.write.verbosity',
        'systemLog.traceExceptions': 'systemLog.traceAllExceptions',
        'systemLog.verbose': 'systemLog.verbosity'
    }
