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
import { isJson } from '../utils';
import { DataMap } from './data-map.config';
import { RouterUrl } from './permission.const';
import { ApplicationType, ResourceType } from './protection.const';

export const USER_GUIDE_CACHE_DATA = {
  action: 'userGuideState',
  active: false,
  appType: '',
  slaType: '',
  host: [],
  resource: [],
  slas: [],
  activeTab: '',
  showTips: false
};

export function clearUserGuideCache() {
  USER_GUIDE_CACHE_DATA.active = false;
  USER_GUIDE_CACHE_DATA.appType = '';
  USER_GUIDE_CACHE_DATA.slaType = '';
  USER_GUIDE_CACHE_DATA.host = [];
  USER_GUIDE_CACHE_DATA.resource = [];
  USER_GUIDE_CACHE_DATA.slas = [];
  USER_GUIDE_CACHE_DATA.activeTab = '';
  USER_GUIDE_CACHE_DATA.showTips = false;
}

export function cacheGuideResource(res: any) {
  if (isJson(res)) {
    USER_GUIDE_CACHE_DATA.resource.push(JSON.parse(res)?.uuid);
  } else {
    USER_GUIDE_CACHE_DATA.resource.push(res?.uuid);
  }
}

export const USER_GUIDE_PROTECTION_STEPS = {
  beforeBackup: {
    id: 'beforeBackup',
    label: 'protection_guide_before_backup_label',
    steps: []
  },
  installClient: {
    id: 'installClient',
    label: 'protection_guide_client_label',
    isDefault: true,
    steps: [
      {
        desc: 'protection_guide_client_desc_label',
        select: true,
        routerLink: RouterUrl.ProtectionHostAppHost
      },
      {
        desc: 'protection_guide_client_desc_pre_label'
      },
      {
        desc: 'protection_guide_client_desc_post_label'
      }
    ]
  },
  resource: {
    id: 'resource',
    label: 'protection_guide_register_resource_label',
    steps: []
  },
  vmGroup: {
    id: 'vmGroup',
    label: 'protection_create_vm_group_label',
    steps: []
  },
  storageDevice: {
    id: 'storageDevice',
    label: 'protection_add_storage_label',
    steps: []
  },
  configNas: {
    id: 'configNas',
    label: 'protection_config_nasshare_label',
    steps: []
  },
  configPermission: {
    id: 'configPermission',
    label: 'protection_config_permission_label',
    steps: []
  },
  sla: {
    id: 'sla',
    label: 'protection_guide_create_sla_label',
    isDefault: true,
    steps: [
      {
        desc: 'protection_guide_sla_desc_label',
        routerLink: RouterUrl.ProtectionSla
      }
    ]
  },
  backup: {
    id: 'backup',
    label: 'protection_guide_take_backup_label',
    steps: []
  }
};

export const USER_GUIDE_APPLICATION_CONFIG = [
  {
    id: 'database',
    label: 'common_database_label',
    apps: [
      {
        id: ApplicationType.DB2,
        subType: DataMap.Resource_Type.DB2.value,
        label: 'DB2',
        prefix: 'D',
        color: '#019A2C',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_before_backup_label',
              link: 'DB2-0012.html',
              enLink: 'en-us_topic_0000002200056433.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_db2_cluster_label',
              desc: 'protection_guide_register_db2_cluster_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppDB2,
              activeTab: DataMap.Resource_Type.dbTwoCluster.value
            },
            {
              title: 'protection_guide_register_db2_instance_label',
              routerLink: RouterUrl.ProtectionHostAppDB2,
              activeTab: DataMap.Resource_Type.dbTwoInstance.value
            },
            {
              title: 'protection_guide_create_db2_tableset_label',
              desc: 'protection_guide_create_db2_tableset_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppDB2,
              activeTab: DataMap.Resource_Type.dbTwoTableSet.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'resource_sub_type_db2_database_label',
              routerLink: RouterUrl.ProtectionHostAppDB2,
              activeTab: DataMap.Resource_Type.dbTwoDatabase.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'resource_sub_type_db2_tablespace_label',
              routerLink: RouterUrl.ProtectionHostAppDB2,
              activeTab: DataMap.Resource_Type.dbTwoTableSet.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.Dameng,
        subType: DataMap.Resource_Type.Dameng_cluster.value,
        label: 'Dameng',
        prefix: 'D',
        color: '#0300D3',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'dameng-00008.html',
              enLink: 'en-us_topic_0000002164659722.html'
            },
            {
              title: 'protection_guide_dameng_dma_label',
              link: 'dameng-00010.html',
              enLink: 'en-us_topic_0000002200146045.html'
            },
            {
              title: 'protection_guide_dameng_archive_label',
              link: 'dameng-00011.html',
              enLink: 'en-us_topic_0000002164659694.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_dameng_register_label',
              routerLink: RouterUrl.ProtectionHostAppDameng
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_dameng_database_label',
              routerLink: RouterUrl.ProtectionHostAppDameng,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.LightCloudGaussDB,
        subType: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
        label: 'protection_light_cloud_gaussdb_label',
        prefix: 'G',
        color: '#C8000C',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'TPOPS_GaussDB_00009.html',
              enLink: 'en-us_topic_0000002200058469.html'
            },
            {
              title: 'protection_guide_gaussdb_xbsa_label',
              link: 'TPOPS_GaussDB_00013.html',
              enLink: 'en-us_topic_0000002164817506.html'
            },
            {
              title: 'protection_guide_gaussdb_root_path_label',
              link: 'TPOPS_GaussDB_00011.html'
            },
            {
              title: 'protection_guide_gaussdb_monitor_label',
              link: 'TPOPS_GaussDB_00012.html'
            },
            {
              title: 'protection_guide_gaussdb_ip_port_label',
              link: 'TPOPS_GaussDB_00014.html',
              enLink: 'en-us_topic_0000002200058477.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_gaussdb_register_label',
              routerLink: RouterUrl.ProtectionHostApLightCloudGaussDB
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_light_cloud_gaussdb_instance_label',
              routerLink: RouterUrl.ProtectionHostApLightCloudGaussDB,
              activeTab: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.standaloneHost
      },
      {
        id: ApplicationType.GaussDBT,
        subType: DataMap.Resource_Type.GaussDB_T.value,
        label: 'resource_sub_type_gauss_dbt_label',
        prefix: 'G',
        color: '#C8000C',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'gaussdbT_00010.html',
              enLink: 'en-us_topic_0000002164760738.html'
            },
            {
              title: 'protection_guide_oracle_environment_ready_label',
              link: 'gaussdbT_00012.html',
              enLink: 'en-us_topic_0000002199967385.html'
            },
            {
              title: 'protection_guide_gaussdbt_redo_label',
              link: 'gaussdbT_00013.html',
              enLink: 'en-us_topic_0000002164760762.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_gaussdbt_register_label',
              routerLink: RouterUrl.ProtectionHostAppGaussDBT
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_gaussdbt_database_label',
              routerLink: RouterUrl.ProtectionHostAppGaussDBT,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.GoldenDB,
        subType: DataMap.Resource_Type.goldendbInstance.value,
        label: 'protection_goldendb_label',
        prefix: 'G',
        color: '#0086D1',
        steps: {
          resource: [
            {
              title: 'protection_goldendb_cluster_register_label',
              routerLink: RouterUrl.ProtectionHostAppGoldendb,
              activeTab: DataMap.Resource_Type.goldendbCluter.value
            },
            {
              title: 'protection_goldendb_instance_register_label',
              routerLink: RouterUrl.ProtectionHostAppGoldendb,
              activeTab: DataMap.Resource_Type.goldendbInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_goldendb_instance_label',
              routerLink: RouterUrl.ProtectionHostAppGoldendb,
              activeTab: DataMap.Resource_Type.goldendbInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.Informix,
        subType: DataMap.Resource_Type.informixService.value,
        label: 'Informix/GBase 8s',
        prefix: 'I',
        color: '#000000',
        steps: {
          beforeBackup: [
            {
              title: 'protection_config_xbsa_path_label',
              link: 'informix-0012.html',
              enLink: 'en-us_topic_0000002200098125.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_informix_cluster_label',
              routerLink: RouterUrl.ProtectionHostAppInformix,
              activeTab: DataMap.Resource_Type.informixService.value
            },
            {
              title: 'protection_register_informix_instance_label',
              routerLink: RouterUrl.ProtectionHostAppInformix,
              activeTab: DataMap.Resource_Type.informixInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_informix_instance_label',
              routerLink: RouterUrl.ProtectionHostAppInformix,
              activeTab: DataMap.Resource_Type.informixInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.KingBase,
        subType: DataMap.Resource_Type.KingBaseInstance.value,
        label: 'Kingbase',
        prefix: 'K',
        color: '#CF142D',
        steps: {
          resource: [
            {
              title: 'protection_register_kingbase_cluster_label',
              desc: 'protection_guide_register_db2_cluster_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppKingBase,
              activeTab: DataMap.Resource_Type.KingBaseCluster.value
            },
            {
              title: 'protection_register_kingbase_instance_label',
              routerLink: RouterUrl.ProtectionHostAppKingBase,
              activeTab: DataMap.Resource_Type.KingBaseInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'resource_sub_type_king_base_instance_label',
              routerLink: RouterUrl.ProtectionHostAppKingBase,
              activeTab: DataMap.Resource_Type.KingBaseInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.MySQL,
        subType: DataMap.Resource_Type.MySQLCluster.value,
        label: 'protection_mysql_label',
        prefix: 'M',
        color: '#01618B',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'mysql-0011.html',
              enLink: 'en-us_topic_0000002199969721.html'
            },
            {
              title: 'protection_enable_database_permission_label',
              link: 'mysql-0013.html',
              enLink: 'en-us_topic_0000002164603378.html'
            },
            {
              title: 'protection_install_backup_tool_label',
              link: 'mysql-0015.html',
              enLink: 'en-us_topic_0000002164603366.html'
            },
            {
              title: 'protection_config_environment_path_label',
              link: 'mysql-0014.html',
              enLink: 'en-us_topic_0000002200004141.html'
            },
            {
              title: 'protection_enable_log_mode_label',
              link: 'mysql-0018.html',
              enLink: 'en-us_topic_0000002164603354.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_cluster_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppMySQL,
              activeTab: 'cluster'
            },
            {
              title: 'protection_register_instance_label',
              routerLink: RouterUrl.ProtectionHostAppMySQL,
              activeTab: 'instance'
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_database_instance_label',
              routerLink: RouterUrl.ProtectionHostAppMySQL,
              activeTab: 'instance',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_database_label',
              routerLink: RouterUrl.ProtectionHostAppMySQL,
              activeTab: 'database',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.OceanBase,
        subType: DataMap.Resource_Type.OceanBaseCluster.value,
        label: 'protection_oceanbase_label',
        prefix: 'O',
        color: '#066FFF',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'oceanbase_00008.html',
              enLink: 'en-us_topic_0000002200131701.html'
            },
            {
              title: 'protection_enable_nfs_label',
              link: 'zh-cn_topic_0000002164645372.html',
              enLink: 'en-us_topic_0000002164645372.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_cluster_label',
              routerLink: RouterUrl.ProtectionHostAppOceanBase,
              activeTab: DataMap.Resource_Type.OceanBaseCluster.value
            },
            {
              title: 'protection_register_tenant_set_label',
              desc: 'protection_register_tenant_set_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppOceanBase,
              activeTab: DataMap.Resource_Type.OceanBaseTenant.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_oceanbase_cluster_label',
              routerLink: RouterUrl.ProtectionHostAppOceanBase,
              activeTab: DataMap.Resource_Type.OceanBaseCluster.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_oceanbase_tenant_label',
              routerLink: RouterUrl.ProtectionHostAppOceanBase,
              activeTab: DataMap.Resource_Type.OceanBaseTenant.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.Oracle,
        subType: DataMap.Resource_Type.oracle.value,
        label: 'common_oracle_label',
        prefix: 'O',
        color: '#C74634',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'oracle_gud_0012.html',
              enLink: 'en-us_topic_0000002199998061.html'
            },
            {
              title: 'protection_guide_oracle_environment_ready_label',
              link: 'oracle_gud_0015.html',
              enLink: 'en-us_topic_0000002199998065.html'
            },
            {
              title: 'protection_guide_oracle_ca_ready_label',
              link: 'oracle_gud_0023.html',
              enLink: 'en-us_topic_0000002199963629.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_cluster_label',
              desc: 'protection_guide_register_cluster_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppOracle,
              activeTab: ResourceType.HOST
            },
            {
              title: 'protection_guide_register_database_label',
              routerLink: RouterUrl.ProtectionHostAppOracle,
              activeTab: ResourceType.DATABASE
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_oracle_database_label',
              routerLink: RouterUrl.ProtectionHostAppOracle,
              activeTab: ResourceType.DATABASE,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.oracle,
        options: [
          {
            key: 'RMAN',
            value: 'RMAN',
            label: 'common_oracle_rman_label',
            isLeaf: true
          },
          {
            key: 'snapshotBackup',
            value: 'snapshotBackup',
            label: 'common_oracle_storage_label',
            isLeaf: true,
            children: [
              {
                key: 'Windows',
                value: 'Windows',
                label: 'Windows OS',
                isLeaf: true
              },
              {
                key: 'Linux',
                value: 'Linux',
                label: 'Linux OS',
                isLeaf: true
              }
            ]
          }
        ]
      },
      {
        id: ApplicationType.PostgreSQL,
        subType: DataMap.Resource_Type.PostgreSQLInstance.value,
        label: 'PostgreSQL',
        prefix: 'P',
        color: '#32648D',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'postgresql-0008.html',
              enLink: 'en-us_topic_0000002164763638.html'
            },
            {
              title: 'protection_install_user_sudo_label',
              link: 'postgresql-0080.html',
              enLink: 'en-us_topic_0000002164603910.html'
            },
            {
              title: 'protection_open_archive_mode_label',
              link: 'postgresql-0010_0.html',
              enLink: 'en-us_topic_0000002200004653.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_postgresql_cluster_label',
              desc: 'protection_guide_register_db2_cluster_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppPostgreSQL,
              activeTab: DataMap.Resource_Type.PostgreSQLCluster.value
            },
            {
              title: 'protection_register_postgresql_instance_label',
              routerLink: RouterUrl.ProtectionHostAppPostgreSQL,
              activeTab: DataMap.Resource_Type.PostgreSQLInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'resource_sub_type_postgre_instance_label',
              routerLink: RouterUrl.ProtectionHostAppPostgreSQL,
              activeTab: DataMap.Resource_Type.PostgreSQLInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.SQLServer,
        subType: DataMap.Resource_Type.SQLServerCluster.value,
        label: 'protection_sql_server_label',
        prefix: 'S',
        color: '#0179D4',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'sql-0011.html',
              enLink: 'en-us_topic_0000002164605414.html'
            },
            {
              title: 'protection_guide_oracle_environment_ready_label',
              link: 'sql-0013.html',
              enLink: 'en-us_topic_0000002164605422.html'
            },
            {
              title: 'protection_set_powershell_permission_label',
              link: 'sql-0014.html',
              enLink: 'en-us_topic_0000002200006217.html'
            },
            {
              title: 'protection_enable_sysadmin_label',
              link: 'sql-0015.html',
              enLink: 'en-us_topic_0000002199971733.html'
            },
            {
              title: 'protection_log_recovery_mode_label',
              link: 'sql-0016.html',
              enLink: 'en-us_topic_0000002199971761.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_sqlser_cluster_label',
              desc: 'protection_guide_register_db2_cluster_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppSQLServer,
              activeTab: DataMap.Resource_Type.SQLServerCluster.value
            },
            {
              title: 'protection_register_sqlser_instance_label',
              routerLink: RouterUrl.ProtectionHostAppSQLServer,
              activeTab: DataMap.Resource_Type.SQLServerInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_sqlser_instance_label',
              routerLink: RouterUrl.ProtectionHostAppSQLServer,
              activeTab: DataMap.Resource_Type.SQLServerInstance.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'explore_sqlserver_group_label',
              routerLink: RouterUrl.ProtectionHostAppSQLServer,
              activeTab: DataMap.Resource_Type.SQLServerGroup.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'explore_sqlserver_database_label',
              routerLink: RouterUrl.ProtectionHostAppSQLServer,
              activeTab: DataMap.Resource_Type.SQLServerDatabase.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.TDSQL,
        subType: DataMap.Resource_Type.tdsqlCluster.value,
        label: 'TDSQL',
        prefix: 'T',
        color: '#006EFF',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'tdsql_gud_012.html',
              enLink: 'en-us_topic_0000002200059921.html'
            },
            {
              title: 'protection_enable_tdsql_permission_label',
              link: 'tdsql_gud_014.html',
              enLink: 'en-us_topic_0000002200059925.html'
            },
            {
              title: 'protection_enable_zkmeta_label',
              link: 'tdsql_gud_015.html',
              enLink: 'en-us_topic_0000002164818970.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_tdsql_cluster_label',
              routerLink: RouterUrl.ProtectionHostAppTdsql,
              activeTab: DataMap.Resource_Type.tdsqlCluster.value
            },
            {
              title: 'protection_register_tdsql_non_distributed_label',
              routerLink: RouterUrl.ProtectionHostAppTdsql,
              activeTab: DataMap.Resource_Type.tdsqlInstance.value
            },
            {
              title: 'protection_register_tdsql_distributed_label',
              routerLink: RouterUrl.ProtectionHostAppTdsql,
              activeTab: DataMap.Resource_Type.tdsqlDistributedInstance.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_tdsql_non_distributed_instance_label',
              routerLink: RouterUrl.ProtectionHostAppTdsql,
              activeTab: DataMap.Resource_Type.tdsqlInstance.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_tdsql_distributed_instance_label',
              routerLink: RouterUrl.ProtectionHostAppTdsql,
              activeTab: DataMap.Resource_Type.tdsqlDistributedInstance.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.TiDB,
        subType: DataMap.Resource_Type.tidbCluster.value,
        label: 'TiDB',
        prefix: 'T',
        color: '#E6012E',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'TiDB_00007.html',
              enLink: 'en-us_topic_0000002200065069.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_tidb_cluster_label',
              routerLink: RouterUrl.ProtectionHostAppTidb,
              activeTab: DataMap.Resource_Type.tidbCluster.value
            },
            {
              title: 'protection_register_tidb_database_label',
              desc: 'protection_register_tidb_database_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppTidb,
              activeTab: DataMap.Resource_Type.tidbDatabase.value
            },
            {
              title: 'protection_register_tidb_tableset_label',
              desc: 'protection_register_tidb_tableset_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppTidb,
              activeTab: DataMap.Resource_Type.tidbTable.value
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_tidb_database_label',
              routerLink: RouterUrl.ProtectionHostAppTidb,
              activeTab: DataMap.Resource_Type.tidbDatabase.value,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_tidb_table_label',
              routerLink: RouterUrl.ProtectionHostAppTidb,
              activeTab: DataMap.Resource_Type.tidbTable.value,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.OpenGauss,
        subType: DataMap.Resource_Type.OpenGauss.value,
        label: 'common_opengauss_label',
        prefix: 'O',
        color: '#2081C4',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'opengauss-0009.html',
              enLink: 'en-us_topic_0000002200039525.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_openguass_cluster_label',
              routerLink: RouterUrl.ProtectionOpenGauss,
              activeTab: 'cluster'
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_opengauss_instance_label',
              routerLink: RouterUrl.ProtectionOpenGauss,
              activeTab: 'instance',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_opengauss_database_label',
              routerLink: RouterUrl.ProtectionOpenGauss,
              activeTab: 'database',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.GeneralDatabase,
        subType: DataMap.Resource_Type.generalDatabase.value,
        label: 'protection_general_database_label',
        prefix: 'T',
        color: '#282B33',
        steps: {
          resource: [
            {
              title: 'protection_register_general_database_label',
              routerLink: RouterUrl.ProtectionHostGeneralDatabase
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_general_database_label',
              routerLink: RouterUrl.ProtectionHostGeneralDatabase,
              hideTips: true
            }
          ]
        }
      }
    ]
  },
  {
    id: 'virtualization',
    label: 'common_virtualization_label',
    apps: [
      {
        id: ApplicationType.Vmware,
        subType: DataMap.Resource_Type.vmware.value,
        label: 'common_vmware_label',
        prefix: 'V',
        color: '#717074',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'vmware_gud_0017.html',
              enLink: 'en-us_topic_0000002164654338.html'
            },
            {
              title: 'protection_install_vmware_tools_label',
              desc: 'protection_install_vmware_tools_desc_label',
              link: 'vmware_gud_0019.html',
              enLink: 'en-us_topic_0000002200055057.html'
            },
            {
              title: 'protection_check_vapi_endpoint_label',
              desc: 'protection_check_vapi_endpoint_desc_label',
              link: 'vmware_gud_0020.html',
              enLink: 'en-us_topic_0000002200140641.html'
            },
            {
              title: 'protection_config_script_label',
              desc: 'protection_config_script_desc_label',
              link: 'vmware_gud_0021.html',
              enLink: 'en-us_topic_0000002164813950.html'
            },
            {
              title: 'protection_get_vmware_certificates_label',
              desc: 'protection_get_vmware_certificates_desc_label',
              link: 'vmware_gud_0026.html',
              enLink: 'en-us_topic_0000002164654274.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_vcenter_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware
            }
          ],
          vmGroup: [
            {
              desc: 'protection_create_vm_group_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware,
              isOptional: true,
              activeTab: ResourceType.VM_GROUP,
              hideTips: true
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_vm_virtual_cluster_label',
              desc: 'protection_vmware_cluster_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware,
              activeTab: ResourceType.CLUSTER,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_vmware_host_label',
              desc: 'protection_vmware_host_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware,
              activeTab: ResourceType.HOST,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_vm_virtual_machine_label',
              desc: 'protection_vmware_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware,
              activeTab: ResourceType.VM,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_vmware_group_label',
              routerLink: RouterUrl.ProtectionVirtualizationVmware,
              activeTab: ResourceType.VM_GROUP,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.vmWare,
        options: [
          {
            key: 'SAN',
            value: 'SAN',
            label: 'common_vm_san_label',
            isLeaf: true
          },
          {
            key: 'HotADD',
            value: 'HotADD',
            label: 'common_vm_hot_add_label',
            isLeaf: true
          },
          {
            key: 'NBD',
            value: 'NBD',
            label: 'common_vm_ndb_label',
            isLeaf: true
          },
          {
            key: 'Storage',
            value: 'Storage',
            label: 'common_vm_storage_label',
            isLeaf: true
          }
        ]
      },
      {
        id: ApplicationType.FusionCompute,
        subType: DataMap.Resource_Type.FusionCompute.value,
        label: 'common_fusion_compute_label',
        prefix: 'F',
        color: '#C80A2B',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'fc_gud_0012.html',
              enLink: 'en-us_topic_0000002164765970.html'
            },
            {
              title: 'protection_create_fc_user_label',
              desc: 'protection_create_fc_user_desc_label',
              link: 'fc_gud_0014.html',
              enLink: 'en-us_topic_0000002164606210.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_fc_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute
            }
          ],
          vmGroup: [
            {
              desc: 'protection_create_vm_group_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute,
              isOptional: true,
              activeTab: ResourceType.VM_GROUP,
              hideTips: true
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_fc_cluster_label',
              desc: 'protection_vmware_cluster_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute,
              activeTab: ResourceType.CLUSTER,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_fc_host_label',
              desc: 'protection_vmware_host_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute,
              activeTab: ResourceType.HOST,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_fc_vm_label',
              desc: 'protection_vmware_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute,
              activeTab: ResourceType.VM,
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_fc_group_label',
              routerLink: RouterUrl.ProtectionVirtualizationFusionCompute,
              activeTab: ResourceType.VM_GROUP,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.standaloneHost
      },
      {
        id: ApplicationType.CNware,
        subType: DataMap.Resource_Type.cNware.value,
        label: 'common_cnware_label',
        prefix: 'C',
        color: '#EA1E28',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'cnware_00011.html',
              enLink: 'en-us_topic_0000002200060089.html'
            },
            {
              title: 'protection_get_cnware_certificates_label',
              desc: 'protection_get_cnware_certificates_desc_label',
              link: 'cnware_00014.html',
              enLink: 'en-us_topic_0000002200060097.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_resource_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware
            }
          ],
          vmGroup: [
            {
              desc: 'protection_create_vm_group_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware,
              isOptional: true,
              activeTab: 'group',
              hideTips: true
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_cnware_cluster_label',
              desc: 'protection_vmware_cluster_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware,
              activeTab: 'cluster',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_cnware_host_label',
              desc: 'protection_vmware_host_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware,
              activeTab: 'host',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_cnware_vm_label',
              desc: 'protection_vmware_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware,
              activeTab: 'vm',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_cnware_group_label',
              routerLink: RouterUrl.ProtectionVirtualizationCnware,
              activeTab: 'group',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyVmHost
      },
      {
        id: ApplicationType.Nutanix,
        subType: DataMap.Resource_Type.nutanix.value,
        label: 'common_nutanix_label',
        prefix: 'N',
        color: '#316CE6',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'Nutanix_0011.html',
              enLink: 'en-us_topic_0000002200026161.html'
            },
            {
              title: 'protection_get_nutanix_certificates_label',
              desc: 'protection_get_nutanix_certificates_desc_label',
              link: 'Nutanix_0014.html',
              enLink: 'en-us_topic_0000002164785140.html'
            }
          ],
          resource: [
            {
              title: 'protection_guide_register_resource_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix
            }
          ],
          vmGroup: [
            {
              desc: 'protection_create_vm_group_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix,
              isOptional: true,
              activeTab: 'group',
              hideTips: true
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_nutanix_cluster_label',
              desc: 'protection_vmware_cluster_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix,
              activeTab: 'cluster',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_nutanix_host_label',
              desc: 'protection_vmware_host_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix,
              activeTab: 'host',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_nutanix_vm_label',
              desc: 'protection_vmware_desc_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix,
              activeTab: 'vm',
              hideTips: true
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_nutanix_group_label',
              routerLink: RouterUrl.ProtectionVirtualizationNutanix,
              activeTab: 'group',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyVmHost
      }
    ]
  },
  {
    id: 'fileSystem',
    label: 'common_file_system_label',
    apps: [
      {
        id: ApplicationType.NASFileSystem,
        subType: DataMap.Resource_Type.NASFileSystem.value,
        label: 'common_nas_file_system_label',
        prefix: 'N',
        color: '#EBAA44',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'nas_s_0010.html',
              enLink: 'en-us_topic_0000002164645320.html'
            },
            {
              title: 'protection_get_ca_certificates_label',
              desc: 'protection_get_ca_certificates_desc_label',
              link: 'nas_s_0012.html',
              enLink: 'en-us_topic_0000002164804992.html'
            },
            {
              title: 'protection_create_port_label',
              desc: 'protection_create_port_desc_label',
              link: 'nas_s_0014.html',
              enLink: 'en-us_topic_0000002200045973.html'
            }
          ],
          storageDevice: [
            {
              desc: 'protection_add_storage_desc_label',
              routerLink: RouterUrl.ProtectionStorageDeviceInfo
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_nas_file_system_label',
              routerLink: RouterUrl.ProtectionDoradoFileSystem,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.nasFileSystem
      },
      {
        id: DataMap.Resource_Type.ndmp.value,
        subType: DataMap.Resource_Type.ndmp.value,
        label: 'protection_ndmp_protocol_label',
        prefix: 'N',
        color: '#EBAA44',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'ndmp_0010.html',
              enLink: 'en-us_topic_0000002200007789.html'
            }
          ],
          storageDevice: [
            {
              desc: 'protection_add_storage_desc_label',
              routerLink: RouterUrl.ProtectionStorageDeviceInfo
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_ndmp_protocol_label',
              routerLink: RouterUrl.ProtectionNdmp,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.object
      },
      {
        id: ApplicationType.NASShare,
        subType: DataMap.Resource_Type.NASShare.value,
        label: 'common_nas_shares_label',
        prefix: 'N',
        color: '#EBAA44',
        steps: {
          beforeBackup: [
            {
              title: 'protection_guide_oracle_info_ready_label',
              link: 'nas_s_0010.html',
              enLink: 'en-us_topic_0000002164645320.html'
            },
            {
              title: 'protection_get_ca_certificates_label',
              desc: 'protection_get_ca_certificates_desc_label',
              link: 'nas_s_0021.html',
              enLink: 'en-us_topic_0000002200046001.html'
            },
            {
              title: 'protection_enable_sv_service_label',
              desc: 'protection_enable_sv_service_desc_label',
              link: 'nas_s_0028.html',
              enLink: 'en-us_topic_0000002164645276.html'
            }
          ],
          storageDevice: [
            {
              desc: 'protection_add_storage_desc_label',
              routerLink: RouterUrl.ProtectionStorageDeviceInfo
            }
          ],
          configNas: [
            {
              desc: 'protection_config_nasshare_desc_label',
              routerLink: RouterUrl.ProtectionNasShared,
              hideTips: true
            }
          ],
          resource: [
            {
              desc: 'protection_register_nasshare_label',
              routerLink: RouterUrl.ProtectionNasShared
            }
          ],
          configPermission: [
            {
              desc: 'protection_config_permission_desc_label',
              link: 'nas_s_0025.html',
              enLink: 'en-us_topic_0000002164645244.html'
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_nas_shared_label',
              routerLink: RouterUrl.ProtectionNasShared,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.nasShare
      },
      {
        id: ApplicationType.Volume,
        subType: DataMap.Resource_Type.volume.value,
        label: 'protection_volume_label',
        prefix: 'V',
        color: '#000000',
        steps: {
          resource: [
            {
              title: 'protection_create_volume_label',
              routerLink: RouterUrl.ProtectionHostAppVolume
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_kubernetes_volume_name_label',
              routerLink: RouterUrl.ProtectionHostAppVolume,
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.ObjectStorage,
        subType: DataMap.Resource_Type.ObjectStorage.value,
        label: 'common_object_storage_label',
        prefix: 'O',
        color: '#C8000C',
        steps: {
          beforeBackup: [
            {
              title: 'protection_get_endpoint_label',
              link: 'object-0011.html',
              enLink: 'en-us_topic_0000002200056357.html'
            },
            {
              title: 'protection_get_ak_sk_label',
              link: 'object-0012.html',
              enLink: 'en-us_topic_0000002200056353.html'
            },
            {
              title: 'protection_enable_sv_service_label',
              link: 'object-0019.html',
              enLink: 'en-us_topic_0000002164815354.html'
            }
          ],
          resource: [
            {
              title: 'protection_register_object_storage_label',
              routerLink: RouterUrl.ProtectionObject,
              activeTab: 'object-storage'
            },
            {
              title: 'protection_register_object_set_label',
              desc: 'protection_register_object_set_desc_label',
              routerLink: RouterUrl.ProtectionObject,
              activeTab: 'object'
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_object_set_label',
              routerLink: RouterUrl.ProtectionObject,
              activeTab: 'object',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.object
      },
      {
        id: ApplicationType.Fileset,
        subType: DataMap.Resource_Type.fileset.value,
        label: 'common_fileset_label',
        prefix: 'F',
        color: '#EBAA44',
        steps: {
          resource: [
            {
              title: 'protection_create_fileset_template_label',
              desc: 'protection_create_fileset_template_desc_label',
              isOptional: true,
              routerLink: RouterUrl.ProtectionHostAppFilesetTemplate,
              activeTab: 'template'
            },
            {
              title: 'protection_create_fileset_label',
              routerLink: RouterUrl.ProtectionHostAppFilesetTemplate,
              activeTab: 'fileset'
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'common_fileset_label',
              routerLink: RouterUrl.ProtectionHostAppFilesetTemplate,
              activeTab: 'fileset',
              hideTips: true
            }
          ]
        },
        clientDeployType: DataMap.clientDeployType.onlyHost
      },
      {
        id: ApplicationType.CommonShare,
        subType: DataMap.Resource_Type.commonShare.value,
        label: 'protection_commonshare_label',
        prefix: 'C',
        color: '#F86603',
        steps: {
          resource: [
            {
              title: 'protection_create_common_share_label',
              routerLink: RouterUrl.ProtectionCommonShare
            }
          ],
          backup: [
            {
              desc: 'protection_guide_backup_guide_tip_label'
            },
            {
              title: 'protection_guide_backup_app_tip_label',
              titleParams: 'protection_commonshare_label',
              routerLink: RouterUrl.ProtectionCommonShare,
              hideTips: true
            }
          ]
        }
      }
    ]
  }
];
