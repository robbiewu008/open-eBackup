/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
export const UserRoleType = {
  // tslint:disable-next-line: object-literal-key-quotes
  '1': 'admin',
  // tslint:disable-next-line: object-literal-key-quotes
  '2': 'subadmin',
  // tslint:disable-next-line: object-literal-key-quotes
  '3': 'useadmin',
  // tslint:disable-next-line: object-literal-key-quotes
  '4': 'devicemanager',
  // tslint:disable-next-line: object-literal-key-quotes
  '5': 'auditor',
  // tslint:disable-next-line: object-literal-key-quotes
  '6': 'remoteDeviceAdministrator',
  // tslint:disable-next-line: object-literal-key-quotes
  '7': 'dmeAdmin'
};

export const UserRoleI18nMap = {
  admin: 'common_sys_admin_label',
  subadmin: 'common_user_label',
  auditor: 'common_auditor_label',
  useadmin: 'common_useadmin_label',
  devicemanager: 'common_devicemanager_label',
  remoteDeviceAdministrator: 'common_remote_device_administrator_label',
  dmeAdmin: 'common_dme_admin_label'
};

export const UserRoleDescI18nMap = {
  admin: 'common_sys_admin_desc_label',
  subadmin: 'common_user_desc_label',
  auditor: 'common_auditor_desc_label',
  useadmin: 'common_useadmin_desc_label',
  devicemanager: 'common_devicemanager_desc_label',
  remoteDeviceAdministrator: 'common_remote_device_administrator_desc_label',
  dmeAdmin: 'common_dme_admin_desc_label'
};

// sftp用户名敏感字符名单
export const SFTP_USERNAME_BLACKLIST = [
  'root',
  'bin',
  'daemon',
  'adm',
  'lp',
  'sync',
  'shutdown',
  'halt',
  'mail',
  'operator',
  'games',
  'ftp',
  'nobody',
  'dbus',
  'tss',
  'polkitd',
  'libstoragemgmt',
  'unbound',
  'setroubleshoot',
  'chrony',
  'ntp',
  'sshd',
  'rpc',
  'cloudmonitor',
  'rpcuser',
  'nfsnobody',
  'radvd',
  'uvpkmc',
  'dnsmasq',
  'saslauth',
  'qemu',
  'gluster',
  'dhcpd',
  'hwcdm'
];

// sftp用户密码敏感字符名单
export const SFTP_PASSWORD_BLACKLIST = [
  'admin',
  'root',
  'bin',
  'daemon',
  'rpcuser',
  'nfsnobody',
  'radvd'
];

// 开关状态
export enum SWITCH_TYPE {
  disable = 0,
  enable = 1
}

// 加密方式
export enum ENCRYPTION_METHOD {
  notencrypted = 0,
  ssl,
  tls
}

// 文件格式
export enum FILE_FORMAT {
  CSV = 1,
  Excel
}

// 语言类型
export enum LANGUAGR_TYPE {
  chinese = 1,
  english
}

export enum LANGUAGE {
  EN = 'en-us',
  CN = 'zh-cn'
}

// 结果类型
export enum RESULT_TYPE {
  unknown = 0,
  success,
  fail,
  running
}

// tape类型
export enum TapeType {
  Rw = 'RW',
  Worm = 'WORM'
}

// tape位置
export enum TapeLocation {
  Driver = 'Driver',
  Slot = 'Slot'
}

// 用户角色
export enum USER_ROLE_TYPE {
  USER,
  ROLE
}

export enum ALARMTHRESHOLD_TYPE {
  Percentage = 1,
  Absolute
}

export enum StandardStatus {
  Installed = 'installed',
  Uninstall = 'uninstall',
  Installing = 'installing',
  Installed_Half = 'installed_half',
  Installed_Loading = 'installed_loading'
}

export enum KerberosConfigMode {
  pwd = 'password_model',
  file = 'keytab_model'
}
