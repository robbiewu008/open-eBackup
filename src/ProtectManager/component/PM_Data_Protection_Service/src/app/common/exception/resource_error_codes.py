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
from app.common.exception.common_error_codes import BaseErrorCode


class ResourceErrorCodes(BaseErrorCode):
    HOST_NOT_EXISTS = {
        "code": "1677931265",
        "message": "The host is not exists."
    }
    HOST_OFFLINE = {
        "code": "1677931266",
        "message": "The host is offline."
    }
    SINGLE_HOST_FILESET_COUNT_OVER_LIMIT = {
        "code": "1677931267",
        "message": "The fileset count on single host cannot exceed 64."
    }
    RESOURCE_ALREADY_PROTECTED = {
        "code": "1677931268",
        "message": "The resource already protected."
    }
    RESOURCE_PROTECTION_STATUS_ACTIVE = {
        "code": "1677931269",
        "message": "The status of resource is active."
    }
    RESOURCE_PROTECTION_STATUS_INACTIVE = {
        "code": "1677931270",
        "message": "The status of resource is inactive."
    }
    RESOURCE_LINKSTATUS_OFFLINE = {
        "code": "1677931271",
        "message": "The linkstatus of resource is offline."
    }
    ESX_IS_MANAGED_BY_VCENTER = {
        "code": "1677931273",
        "message": "The ESX/ESXi has been managed by the vCenter and cannot be managed independently."
    }
    RESOURCE_IS_REGISTERED = {
        "code": "1677931274",
        "message": "The resource has been registered and cannot be registered again."
    }
    NETWORK_CONNECTION_TIMEDOUT = {
        "code": "1677931275",
        "message": "Network connection timed out."
    }
    PROTECTED_OBJECT_SCRIPT_FORMAT_INCORRECT = {
        "code": "1677931272",
        "message": "protected object script format incorrect."
    }
    PROTECTED_HOST_CONFIRMED_FAILED = {
        "code": "1677931321",
        "message": "host auth failed."
    }
    RESOURCE_AUTHENTICATION_STATUS_IS_FALSE = {
        "code": "1677931277",
        "message": "The resource authentication status is false."
    }
    DATABASE_SERVICE_NOT_EXISTS = {
        "code": "1677931278",
        "message": "The database service not exists."
    }
    CLUSTER_IS_REGISTERED = {
        "code": "1677931279",
        "message": "The cluster is registered."
    }
    RESOURCE_NOT_PROTECTED = {
        "code": "1677931777",
        "message": "The resource is not protected."
    }
    HYPER_V_NAME_DUPLICATE = {
        "code": "1677931284",
        "message": "The hyper v name has existed."
    }
    DATABASE_INSTANCE_NAME_DUPLICATE = {
        "code": "1677931285",
        "message": "The database instance name has existed."
    }
    INPUT_VMWARE_IP_INVALID = {
        "code": "1677931291",
        "message": "The input IP is the information of other VMware."
    }
    VMWARE_CONNECTION_FAILED = {
        "code": "1677931289",
        "message": "The VMware connection failed."
    }
    VSPHERE_HOST_DUPLICATE = {
        "code": "1677931293",
        "message": "The host in the registered vCenter may be the same as the host in the current vCenter."
    }
    BACKUP_RESOURCE_IS_LOCKED = {
        "code": "1677931296",
        "message": "Backups of the same resource are too frequent, resulting in resource locked."
    }
    REPLICATION_PROTECT_CONDITION = {
        "code": "1677931297",
        "message": "Replica resources can only be protected on the disaster recovery side."
    }
    HOST_BELONG_USER_NOT_CORRECT = {
        "code": "1677931299",
        "message": "Hosts in a cluster need to belong to the same user."
    }
    HOST_BOUND_VIRTUAL_PLATFORM = {
        "code": "1677931300",
        "message": "The host has joined the virtualization platform({0})."
    }
    VCENTER_RESOURCE_IS_PROTECTED = {
        "code": "1677931301",
        "message": "VMware registration failed, because the VM/host/cluster in the VMware environment is protected."
    }
    BACKUP_TASK_IS_WORKING = {
        "code": "1677931298",
        "message": "Can not remove protect, because backup task is working."
    }
    DELETE_RESOURCE_HAS_SLD = {
        "code": "1677931320",
        "message": "Can not delete, because resource has protect."
    }
    REGISTER_VMWARE_CERT_FAILURE = {
        "code": "1677931024",
        "message": "No trusted certificate can be used to log in to the VMware environment."
    }
    VMWARE_CERTIFICATE_EXPIRED = {
        "code": "1677931303",
        "message": "Certificate is expired."
    }
    ENVIRONMENT_CERT_IS_REVOKED = {
        "code": "1677931037",
        "message": "The certificate of the environment has been revoked."
    }
    FILE_SIZE_IS_OVER_LIMIT = {
        "code": "1677929231",
        "message": "The revocation list size is over limit."
    }
    CRL_HAS_EXPIRED = {
        "code": "1677931040",
        "message": "The revocation list has expired."
    }
    CRL_IS_INVALID = {
        "code": "1677931017",
        "message": "The revocation list does not match the certification."
    }
    HOST_HAS_RESOURCE = {
        "code": "1677931304",
        "message": "Can not delete, because host has resource"
    }
    THE_HOST_HAS_BEEN_REGISTER_INSTANCE = {
        "code": "1677931305",
        "message": "The host has been register instance."
    }
    THE_INSTANCE_NOT_MATCH_THIS_HOST_INFO = {
        "code": "1677931306",
        "message": "The instance does not match this host information."
    }
    SINGLE_NAMESPACE_DATASET_COUNT_OVER_LIMIT = {
        "code": "1677931337",
        "message": "The dataset count on single namespace cannot exceed 20."
    }
    THE_NAMESPACE_LABEL_IS_INVALID = {
        "code": "1677931338",
        "message": "The label {} of namespace is invalid."
    }
    DATABASE_AUTH_INVALID = {
        "code": "1677931371",
        "message": "Indicates the authentication of the database. This operation is invalid."
    }
    FILESET_TEMPLATES_ASSOCIATE_MAX_FILESETS = {
        "code": "1677931377",
        "message": "The number of filesets associated with the template exceeds the upper limit."
    }
    FILESET_TEMPLATES_COUNT_OVER_LIMIT = {
        "code": "1677931378",
        "message": "The fileset templates count cannot exceed 128."
    }
    CLUSTER_NODES_QUERY_FAILED = {
        "code": "1677929985",
        "message": "The network connection is abnormal or the target cluster is abnormal."
    }
    CONNECT_HOST_FAILED = {
        "code": "1677931344",
        "message": "A user fails to register the agent client when the target host cannot be connected."
    }
    VSPHERE_DELETE_FAILED = {
        "code": "1677931406",
        "message": "Failed to delete a registered vCenter environment because the resource is being scanned."
    }
    MIGRATE_HOST_HAS_PROTECTED_OBJECTS = {
        "code": "1677931411",
        "message": "the host migrate failed because associated with a protected object."
    }
    MIGRATE_HOST_IS_REPLICATION_CLUSTER = {
        "code": "1677931410",
        "message": "the host migrate failed because target cluster is a replication cluster."
    }
    VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY = {
        "code": "1677931451",
        "message": "Failed to get virtual machine disk information."
    }
    CLOUD_HOST_DISK_INFO_IS_EMPTY = {
        "code": "1677747721",
        "message": "Failed to obtain the cloud host disk information."
    }
    VIRTUAL_MACHINE_INFO_FAILED = {
        "code": "1677931357",
        "message": "Failed to get virtual machine information."
    }
    MANUAL_REPLICATION_HAS_NO_COPIES = {
        "code": "1677749504",
        "message": "There are no copies that satisfy the replication rules."
    }
    RESOURCES_IN_RESOURCE_GROUP = {
        "code": "1677931549",
        "message": "Can not remove protect, because resources exist in the resource group."
    }
    NOT_SUPPORT_SHARED_DISK = {
        "code": "1677932051",
        "message": "Fail to protect hyper-v resource, because not to support shared disk."
    }
    NOT_SUPPORT_VHD_SET_DISK = {
        "code": "1677932052",
        "message": "Fail to protect hyper-v resource, because not to support vhdx set disk."
    }
    NOT_SUPPORT_PHYSICAL_HARD_DISK = {
        "code": "1677932058",
        "message": "Fail to protect hyper-v resource, because not to support physical hard disk."
    }
    NOT_SUPPORT_PROTECT_MULTI_WRITER_DISK = {
        "code": "1677931449",
        "message": "Disks of the multi-writer sharing type cannot be protected."
    }
    HAS_MULTI_WRITER_SHARING_DISK = {
        "code": "1677931441",
        "message": "Backup of disks with multi-writers sharing type is not supported."
    }
