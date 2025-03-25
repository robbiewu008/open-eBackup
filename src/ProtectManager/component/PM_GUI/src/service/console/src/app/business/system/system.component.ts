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
import { Component, OnInit } from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  getAccessibleMenu,
  GlobalService,
  RoleType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { includes } from 'lodash';

@Component({
  selector: 'aui-system',
  templateUrl: './system.component.html'
})
export class SystemComponent implements OnInit {
  menus = [];
  collapsed = false;
  isHcsUser = false;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;

  constructor(
    public i18n: I18NService,
    public cookieService: CookieService,
    public globalService: GlobalService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.globalService.getUserInfo().subscribe(res => {
      this.initMenus();
      this.isHcsUser =
        this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
    });
  }

  initMenus() {
    const menus = [
      {
        id: 'infrastructure',
        label: this.i18n.get('common_infrastructure_label'),
        icon: 'aui-icon-infrastruct',
        items: [
          {
            id: 'cluster-management',
            label: this.i18n.get('system_cluster_management_label'),
            routerLink: '/system/infrastructure/cluster-management'
          },
          {
            id: 'local-storage',
            label: this.i18n.get('system_local_storage_label'),
            hidden: !includes(
              [
                DataMap.Deploy_Type.hyperdetect.value,
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value
              ],
              this.i18n.get('deploy_type')
            ),
            routerLink: '/system/infrastructure/local-storage'
          },
          {
            id: 'archive-storage',
            label: this.i18n.get('system_archive_storage_warehouse_label'),
            routerLink: '/system/infrastructure/archive-storage'
          },
          {
            id: 'backup-storage',
            label: this.i18n.get('common_backup_storage_label'),
            routerLink: '/system/infrastructure/backup-storage'
          },
          {
            id: 'nas-backup-storage',
            label: this.i18n.get('system_backup_storage_unit_group_label'),
            routerLink: '/system/infrastructure/nas-backup-storage'
          },
          {
            id: 'hcs-storage',
            label: this.i18n.get('system_hcs_storage_label'),
            routerLink: '/system/infrastructure/hcs-storage',
            hidden: !this.isHcsEnvir
          }
        ]
      },
      {
        id: 'security',
        label: this.i18n.get('common_security_label'),
        icon: 'aui-icon-security',
        items: [
          {
            id: 'rbac',
            label: this.i18n.get('system_rbac_label'),
            hidden: includes(
              [
                DataMap.Deploy_Type.cyberengine.value,
                DataMap.Deploy_Type.hyperdetect.value,
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value
              ],
              this.i18n.get('deploy_type')
            ),
            routerLink: '/system/security/rbac'
          },
          {
            id: 'userrole',
            label: this.i18n.get('system_user_role_label'),
            hidden: !includes(
              [
                DataMap.Deploy_Type.cyberengine.value,
                DataMap.Deploy_Type.hyperdetect.value,
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value
              ],
              this.i18n.get('deploy_type')
            ),
            routerLink: '/system/security/userrole'
          },
          {
            id: 'userQuota',
            label: this.i18n.get('system_user_quota_label'),
            routerLink: '/system/security/user-quota'
          },
          {
            id: 'securitypolicy',
            label: this.i18n.get('system_security_policy_label'),
            routerLink: '/system/security/securitypolicy'
          },
          {
            id: 'certificate',
            label: this.i18n.get('system_certificate_label'),
            routerLink: '/system/security/certificate'
          },
          {
            id: 'kerberos',
            label: this.i18n.get('Kerberos'),
            routerLink: '/system/security/kerberos'
          },
          {
            id: 'dataSecurity',
            label: this.i18n.get('system_data_security_label'),
            routerLink: '/system/security/dataSecurity'
          },
          {
            id: 'hostTrustworthiness',
            label: this.i18n.get('common_host_trustworthiness_op_label'),
            routerLink: '/system/security/hostTrustworthiness'
          },
          {
            id: 'ldapConfig',
            label: this.i18n.get('system_ldap_service_config_label'),
            routerLink: '/system/security/ldapService'
          },
          {
            id: 'samlSsoConfig',
            label: this.i18n.get('system_saml_sso_config_label'),
            routerLink: '/system/security/samlSsoConfig',
            hidden: this.appUtilsService.isDistributed
          },
          {
            id: 'adfsConfig',
            label: this.i18n.get('system_adfs_label'),
            routerLink: '/system/security/adfsConfig'
          }
        ]
      },
      {
        id: 'license',
        label: this.i18n.get('common_license_label'),
        icon: 'aui-icon-license',
        routerLink: '/system/license',
        hidden: !includes(
          [
            DataMap.Deploy_Type.cyberengine.value,
            DataMap.Deploy_Type.decouple.value,
            DataMap.Deploy_Type.openServer.value
          ],
          this.i18n.get('deploy_type')
        )
      },
      {
        id: 'network-config',
        hidden:
          this.i18n.get('deploy_type') !==
          DataMap.Deploy_Type.cyberengine.value,
        label: this.i18n.get('common_network_config_label'),
        icon: 'aui-icon-network-config',
        routerLink: '/system/network-config'
      },
      {
        id: 'tag-management',
        label: this.i18n.get('system_tag_management_label'),
        icon: 'aui-icon-logs',
        routerLink: '/system/settings/tag-management'
      },
      {
        id: 'log-management',
        label: this.i18n.get('common_log_management_label'),
        icon: 'aui-icon-logs',
        routerLink: '/system/log-management'
      },
      {
        id: 'export-query',
        label: this.i18n.get('common_export_query_label'),
        icon: 'aui-icon-export-other',
        routerLink: '/system/export-query'
      },
      {
        id: 'settings',
        label: this.i18n.get('common_setting_label'),
        icon: 'aui-icon-settings',
        items: [
          {
            id: 'system-backup',
            label: this.i18n.get('common_management_data_backup_label'),
            routerLink: '/system/settings/system-backup'
          },
          {
            id: 'alarm-notify',
            label: this.i18n.get('system_alarm_notification_label'),
            routerLink: '/system/settings/alarm-notify'
          },
          {
            id: 'alarm-notify-settings',
            hidden: !includes(
              [
                DataMap.Deploy_Type.cyberengine.value,
                DataMap.Deploy_Type.decouple.value,
                DataMap.Deploy_Type.e6000.value
              ],
              this.i18n.get('deploy_type')
            ),
            label: this.i18n.get('system_alarm_term_notify_label'),
            routerLink: '/system/settings/alarm-notify-settings'
          },
          {
            id: 'alarm-settings',
            hidden: !includes(
              [
                DataMap.Deploy_Type.hyperdetect.value,
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value
              ],
              this.i18n.get('deploy_type')
            ),
            label: this.i18n.get('system_alarm_settings_label'),
            routerLink: '/system/settings/alarm-settings'
          },
          {
            id: 'alarm-dump',
            label: this.i18n.get('system_event_dump_label'),
            routerLink: '/system/settings/alarm-dump'
          },
          {
            id: 'snmp-trap',
            hidden: !includes(
              [
                DataMap.Deploy_Type.x8000.value,
                DataMap.Deploy_Type.a8000.value,
                DataMap.Deploy_Type.x3000.value,
                DataMap.Deploy_Type.x6000.value,
                DataMap.Deploy_Type.x9000.value,
                DataMap.Deploy_Type.cyberengine.value,
                DataMap.Deploy_Type.e6000.value,
                DataMap.Deploy_Type.decouple.value,
                DataMap.Deploy_Type.openServer.value
              ],
              this.i18n.get('deploy_type')
            ),
            label: this.i18n.get('system_snmp_trap_label'),
            routerLink: '/system/settings/snmp-trap'
          },
          {
            id: 'sftp-service',
            label: this.i18n.get('system_sftp_label'),
            routerLink: '/system/settings/sftp-service',
            hidden: includes(
              [
                DataMap.Deploy_Type.x3000.value,
                DataMap.Deploy_Type.e6000.value,
                DataMap.Deploy_Type.decouple.value,
                DataMap.Deploy_Type.openServer.value
              ],
              this.i18n.get('deploy_type')
            )
          },
          {
            id: 'ibmc',
            label: this.i18n.get('iBMC'),
            routerLink: '/system/settings/ibmc',
            hidden: !includes(
              [DataMap.Deploy_Type.cyberengine.value],
              this.i18n.get('deploy_type')
            )
          },
          {
            id: 'system-time',
            label: this.i18n.get('system_device_time_label'),
            hidden:
              this.cookieService.role === RoleType.SysAdmin &&
              !includes(
                [
                  DataMap.Deploy_Type.hyperdetect.value,
                  DataMap.Deploy_Type.cloudbackup.value,
                  DataMap.Deploy_Type.cloudbackup2.value,
                  DataMap.Deploy_Type.cyberengine.value
                ],
                this.i18n.get('deploy_type')
              ),
            routerLink: '/system/settings/system-time'
          },
          {
            id: 'config-network',
            label: this.i18n.get('common_network_config_label'),
            hidden: !(
              this.cookieService.role === RoleType.SysAdmin &&
              includes(
                [
                  DataMap.Deploy_Type.x8000.value,
                  DataMap.Deploy_Type.a8000.value,
                  DataMap.Deploy_Type.x3000.value,
                  DataMap.Deploy_Type.x6000.value,
                  DataMap.Deploy_Type.x9000.value
                ],
                this.i18n.get('deploy_type')
              )
            ),
            routerLink: '/system/settings/config-network'
          },
          {
            id: 'service-oriented-nms',
            label: this.i18n.get('system_dme_access_setting_label'),
            routerLink: '/system/settings/service-oriented-nms'
          }
        ]
      },
      {
        id: 'external-associated-systems',
        label: this.i18n.get('common_external_associated_systems_label'),
        icon: 'aui-icon-external-system',
        hidden: includes(
          [
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value,
            DataMap.Deploy_Type.hyperdetect.value,
            DataMap.Deploy_Type.cyberengine.value
          ],
          this.i18n.get('deploy_type')
        ),
        routerLink: '/system/external-associated-systems'
      }
    ];
    this.menus = getAccessibleMenu(menus, this.cookieService, this.i18n);
  }
}
