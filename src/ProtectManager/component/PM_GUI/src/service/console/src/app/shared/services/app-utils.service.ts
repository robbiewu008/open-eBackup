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
import { Injectable } from '@angular/core';
import { AbstractControl, ValidatorFn } from '@angular/forms';
import { Router } from '@angular/router';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isFunction,
  isNumber,
  map,
  reduce,
  set,
  size,
  values
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map as _map } from 'rxjs/operators';
import {
  AppService,
  ProtectedResourceApiService,
  SystemApiService
} from '../api/services';
import {
  ApplicationType,
  ClusterEnvironment,
  CommonConsts,
  DataMap,
  HelpUrlCode,
  ResourceSetType,
  ResourceType,
  RESOURCE_CATALOGS,
  RetentionType,
  RouterUrl,
  SoftwareType,
  SYSTEM_TIME,
  TRIGGER_TYPE
} from '../consts';
import { CookieService } from './cookie.service';
import { DataMapService } from './data-map.service';
import { I18NService } from './i18n.service';
import { ResourceCatalogsService } from './resource-catalogs.service';
import { WhiteboxService } from './whitebox.service';

@Injectable({
  providedIn: 'root'
})
export class AppUtilsService {
  // 临时缓存数据
  cacheObject: { [key: string]: any } = {};
  isDmeUser = this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isDistributed =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value;
  isDecouple = includes(
    [DataMap.Deploy_Type.decouple.value, DataMap.Deploy_Type.openServer.value],
    this.i18n.get('deploy_type')
  );
  isOpenOem =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.openOem.value;
  isOpenServer =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.openServer.value;
  isOpenVersion = this.isOpenOem || this.isOpenServer;
  isJumpToStorageUnits = false;
  isWhitebox = this.whitebox.isWhitebox;

  // 联机帮助包路径
  helpPkg = this.isWhitebox || this.isOpenVersion ? 'oem' : 'a8000';

  // 备份一体机
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  // 巴石油修改页面版本号
  isModifyVersion =
    this.isDataBackup ||
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private appService: AppService,
    private whitebox: WhiteboxService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService,
    private resourceCatalogsService: ResourceCatalogsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  setCacheValue(key: string, value: any) {
    set(this.cacheObject, key, value);
  }

  getCacheValue(key: string, clear = true): any {
    const cacheValue = this.cacheObject[key];
    if (clear) {
      delete this.cacheObject[key];
    }
    return cacheValue;
  }

  clearCacheValue(key: string) {
    delete this.cacheObject[key];
  }

  getApplicationConfig() {
    const items = this.resourceCatalogsService.parseCatalogs(RESOURCE_CATALOGS);
    const resource = {
      database: [
        {
          id: 'antdb',
          slaId: ApplicationType.AntDB,
          key: [
            DataMap.Resource_Type.AntDBInstance.value,
            DataMap.Resource_Type.AntDBClusterInstance.value
          ],
          hide: !includes(items, ApplicationType.AntDB),
          label: this.i18n.get('protection_antdb_label'),
          prefix: 'A',
          color: '#01618B',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionDatabaseAntDB,
          copyUrl: RouterUrl.ExploreCopyDataAntDB,
          resType: ResourceSetType.AntDB,
          resourceSetType: ResourceSetType.AntDB,
          jobTargetType: [
            DataMap.Job_Target_Type.AntDBInstance.value,
            DataMap.Job_Target_Type.AntDBClusterInstance.value
          ]
        },
        {
          id: 'oralce',
          slaId: ApplicationType.Oracle,
          key: [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value,
            DataMap.Resource_Type.oraclePDB.value
          ],
          hide: !includes(items, ApplicationType.Oracle),
          label: this.i18n.get('common_oracle_label'),
          prefix: 'O',
          color: '#C74634',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppOracle,
          copyUrl: RouterUrl.ExploreCopyDataOracle,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationOracle,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: ClusterEnvironment.oralceClusterEnv
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.oracle.value
            },
            {
              label: this.i18n.get('protection_pdb_set_label'),
              resType: DataMap.Resource_Type.oraclePDB.value
            }
          ],
          resourceSetType: ResourceSetType.Oracle,
          jobTargetType: [
            DataMap.Job_Target_Type.oracle.value,
            DataMap.Job_Target_Type.oracleCluster.value,
            DataMap.Job_Target_Type.oraclePdbSet.value
          ]
        },
        {
          id: 'mysql',
          slaId: ApplicationType.MySQL,
          key: [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLClusterInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value
          ],
          hide: !includes(items, ApplicationType.MySQL),
          label: this.i18n.get('protection_mysql_label'),
          prefix: 'M',
          color: '#01618B',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppMySQL,
          copyUrl: RouterUrl.ExploreCopyDataMySQL,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationMysql,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.MySQLCluster.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.MySQLInstance.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.MySQLDatabase.value
            }
          ],
          resourceSetType: ResourceSetType.MySQL,
          jobTargetType: [
            DataMap.Job_Target_Type.MySQLCluster.value,
            DataMap.Job_Target_Type.MySQLInstance.value,
            DataMap.Job_Target_Type.MySQLDatabase.value,
            DataMap.Job_Target_Type.MySQLClusterInstance.value
          ]
        },
        {
          id: 'sqlserver',
          slaId: ApplicationType.SQLServer,
          key: [
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.SQLServerDatabase.value
          ],
          hide: !includes(items, ApplicationType.SQLServer),
          label: this.i18n.get('SQL Server'),
          prefix: 'S',
          color: '#0179D4',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppSQLServer,
          copyUrl: RouterUrl.ExploreCopyDataSQLServer,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.SQLServerCluster.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.SQLServerInstance.value
            },
            {
              label: this.i18n.get('protection_availability_group_label'),
              resType: DataMap.Resource_Type.SQLServerGroup.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.SQLServerDatabase.value
            }
          ],
          resourceSetType: ResourceSetType.SQLServer,
          jobTargetType: [
            DataMap.Job_Target_Type.SQLServerCluster.value,
            DataMap.Job_Target_Type.SQLServerInstance.value,
            DataMap.Job_Target_Type.sqlServerClusterInstance.value,
            DataMap.Job_Target_Type.SQLServerGroup.value,
            DataMap.Job_Target_Type.SQLServerDatabase.value
          ]
        },
        {
          id: 'postgresql',
          slaId: ApplicationType.PostgreSQL,
          key: [
            DataMap.Resource_Type.PostgreSQLInstance.value,
            DataMap.Resource_Type.PostgreSQLClusterInstance.value
          ],
          hide: !includes(items, ApplicationType.PostgreSQL),
          label: this.i18n.get('PostgreSQL'),
          prefix: 'P',
          color: '#32648D',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppPostgreSQL,
          copyUrl: RouterUrl.ExploreCopyDataPostgreSQL,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.PostgreSQLCluster.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.PostgreSQLInstance.value
            }
          ],
          resourceSetType: ResourceSetType.PostgreSQL,
          jobTargetType: [
            DataMap.Job_Target_Type.PostgreInstance.value,
            DataMap.Job_Target_Type.PostgreClusterInstance.value
          ]
        },
        {
          id: 'db2',
          slaId: ApplicationType.DB2,
          key: [
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.dbTwoTableSet.value
          ],
          hide: !includes(items, ApplicationType.DB2),
          label: this.i18n.get('DB2'),
          prefix: 'D',
          color: '#019A2C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppDB2,
          copyUrl: RouterUrl.ExploreCopyDataDB2,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.dbTwoCluster.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.dbTwoInstance.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.dbTwoDatabase.value
            },
            {
              label: this.i18n.get('protection_table_space_set_label'),
              resType: DataMap.Resource_Type.dbTwoTableSet.value
            }
          ],
          resourceSetType: ResourceSetType.DB2,
          jobTargetType: [
            DataMap.Job_Target_Type.dbTwoCluster.value,
            DataMap.Job_Target_Type.dbTwoInstance.value,
            DataMap.Job_Target_Type.dbTwoClusterInstance.value,
            DataMap.Job_Target_Type.dbTwoDatabase.value,
            DataMap.Job_Target_Type.dbTwoTableSet.value
          ]
        },
        {
          id: 'informix',
          slaId: ApplicationType.Informix,
          key: [
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value
          ],
          hide: !includes(items, ApplicationType.Informix),
          label: this.i18n.get('Informix/GBase 8s'),
          prefix: 'I',
          color: '#000000',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppInformix,
          copyUrl: RouterUrl.ExploreCopyDataInformix,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.informixService.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.informixInstance.value
            }
          ],
          resourceSetType: ResourceSetType.Informix,
          jobTargetType: [
            DataMap.Job_Target_Type.informixService.value,
            DataMap.Job_Target_Type.informixInstance.value,
            DataMap.Job_Target_Type.informixClusterInstance.value
          ]
        },
        {
          id: 'opengauss',
          slaId: ApplicationType.OpenGauss,
          key: [
            DataMap.Resource_Type.OpenGauss_database.value,
            DataMap.Resource_Type.OpenGauss_instance.value
          ],
          hide: !includes(items, ApplicationType.OpenGauss),
          label: this.i18n.get('common_opengauss_label'),
          prefix: 'O',
          color: '#2081C4',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionOpenGauss,
          copyUrl: RouterUrl.ExploreCopyDataOpenGauss,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.OpenGauss.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.OpenGauss_instance.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.OpenGauss_database.value
            }
          ],
          resourceSetType: ResourceSetType.OpenGauss,
          jobTargetType: [
            DataMap.Job_Target_Type.OpenGauss.value,
            DataMap.Job_Target_Type.openGaussInstance.value,
            DataMap.Job_Target_Type.openGaussDatabase.value
          ]
        },
        {
          id: 'gaussdbt',
          slaId: ApplicationType.GaussDBT,
          key: [
            DataMap.Resource_Type.GaussDB_T.value,
            DataMap.Resource_Type.gaussdbTSingle.value
          ],
          hide: !includes(items, ApplicationType.GaussDBT),
          label: this.i18n.get('GaussDB T'),
          prefix: 'G',
          color: '#C8000C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppGaussDBT,
          copyUrl: RouterUrl.ExploreCopyDataGaussDBT,
          resType: DataMap.Resource_Type.GaussDB_T.value,
          resourceSetType: ResourceSetType.GaussDB_T,
          jobTargetType: [
            DataMap.Job_Target_Type.GaussDB_T.value,
            DataMap.Job_Target_Type.gaussdbTSingle.value
          ]
        },
        {
          id: 'tidb',
          slaId: ApplicationType.TiDB,
          key: [
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.tidbTable.value
          ],
          hide: !includes(items, ApplicationType.TiDB),
          label: this.i18n.get('TiDB'),
          prefix: 'T',
          color: '#E6012E',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppTidb,
          copyUrl: RouterUrl.ExploreCopyDataTiDB,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.tidbCluster.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.tidbDatabase.value
            },
            {
              label: this.i18n.get('protection_table_set_label'),
              resType: DataMap.Resource_Type.tidbTable.value
            }
          ],
          resourceSetType: ResourceSetType.TiDB,
          jobTargetType: [
            DataMap.Job_Target_Type.tidbCluster.value,
            DataMap.Job_Target_Type.tidbDatabase.value,
            DataMap.Job_Target_Type.tidbTable.value
          ]
        },
        {
          id: 'oceanbase',
          slaId: ApplicationType.OceanBase,
          key: [
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.OceanBaseTenant.value
          ],
          hide: !includes(items, ApplicationType.OceanBase),
          label: this.i18n.get('OceanBase'),
          prefix: 'O',
          color: '#066FFF',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppOceanBase,
          copyUrl: RouterUrl.ExploreCopyDataOceanBase,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.OceanBaseCluster.value
            },
            {
              label: this.i18n.get('protection_tenant_set_label'),
              resType: DataMap.Resource_Type.OceanBaseTenant.value
            }
          ],
          resourceSetType: ResourceSetType.OceanBase,
          jobTargetType: [
            DataMap.Job_Target_Type.oceanBaseCluster.value,
            DataMap.Job_Target_Type.oceanBaseTenant.value
          ]
        },
        {
          id: 'tdsql',
          slaId: ApplicationType.TDSQL,
          key: [
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.tdsqlDistributedInstance.value
          ],
          hide: !includes(items, ApplicationType.TDSQL),
          label: this.i18n.get('TDSQL'),
          prefix: 'T',
          color: '#006EFF',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppTdsql,
          copyUrl: RouterUrl.ExploreCopyDataTDSQL,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationTdsql,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.tdsqlCluster.value
            },
            {
              label: this.i18n.get('protection_non_distributed_instance_label'),
              resType: DataMap.Resource_Type.tdsqlInstance.value
            },
            {
              label: this.i18n.get('protection_distributed_instance_label'),
              resType: DataMap.Resource_Type.tdsqlDistributedInstance.value
            }
          ],
          resourceSetType: ResourceSetType.TDSQL,
          jobTargetType: [
            DataMap.Job_Target_Type.tdsqlCluster.value,
            DataMap.Job_Target_Type.tdsqlInstance.value,
            DataMap.Job_Target_Type.tdsqlDistributedInstance.value
          ]
        },
        {
          id: 'kingbase',
          slaId: ApplicationType.KingBase,
          key: [
            DataMap.Resource_Type.KingBaseInstance.value,
            DataMap.Resource_Type.KingBaseClusterInstance.value
          ],
          hide: !includes(items, ApplicationType.KingBase),
          label: this.i18n.get('Kingbase'),
          prefix: 'K',
          color: '#CF142D',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppKingBase,
          copyUrl: RouterUrl.ExploreCopyDataKingBase,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.KingBaseCluster.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.KingBaseInstance.value
            }
          ],
          resourceSetType: ResourceSetType.KingBase,
          jobTargetType: [
            DataMap.Job_Target_Type.KingBaseInstance.value,
            DataMap.Job_Target_Type.KingBaseClusterInstance.value
          ]
        },
        {
          id: 'dameng',
          slaId: ApplicationType.Dameng,
          key: [
            DataMap.Resource_Type.Dameng_cluster.value,
            DataMap.Resource_Type.Dameng_singleNode.value
          ],
          hide: !includes(items, ApplicationType.Dameng),
          label: this.i18n.get('Dameng'),
          prefix: 'D',
          color: '#0300D3',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppDameng,
          copyUrl: RouterUrl.ExportCopyDataDameng,
          resType: DataMap.Resource_Type.Dameng.value,
          resourceSetType: ResourceSetType.Dameng,
          jobTargetType: [
            DataMap.Job_Target_Type.damengCluster.value,
            DataMap.Job_Target_Type.damengSingleNode.value
          ]
        },
        {
          id: 'goldendb',
          slaId: ApplicationType.GoldenDB,
          key: DataMap.Resource_Type.goldendbInstance.value,
          hide: !includes(items, ApplicationType.GoldenDB),
          label: this.i18n.get('protection_goldendb_label'),
          prefix: 'G',
          color: '#0086D1',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppGoldendb,
          copyUrl: RouterUrl.ExploreCopyDataGoldendb,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.goldendbCluter.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.goldendbInstance.value
            }
          ],
          resourceSetType: ResourceSetType.GoldenDB,
          jobTargetType: [
            DataMap.Job_Target_Type.goldendbCluter.value,
            DataMap.Job_Target_Type.goldendbInstance.value
          ]
        },
        {
          id: 'generaldatabase',
          slaId: ApplicationType.GeneralDatabase,
          key: [DataMap.Resource_Type.generalDatabase.value],
          hide: !includes(items, ApplicationType.GeneralDatabase),
          label: this.i18n.get('protection_general_database_label'),
          prefix: 'T',
          color: '#282B33',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostGeneralDatabase,
          copyUrl: RouterUrl.ExploreCopyDataGeneralDatabase,
          resType: DataMap.Resource_Type.generalDatabase.value,
          resourceSetType: ResourceSetType.GeneralDb,
          jobTargetType: [DataMap.Job_Target_Type.GeneralDatabase.value]
        },
        {
          id: 'gbase',
          slaId: ApplicationType.GBase,
          key: [
            DataMap.Resource_Type.gbaseClusterInstance.value,
            DataMap.Resource_Type.gbaseInstance.value
          ],
          hide: true,
          label: this.i18n.get('protection_gbase_label'),
          prefix: 'G',
          color: '#E60213',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionGbase,
          copyUrl: RouterUrl.ExploreCopyDataDatabaseGbase,
          resType: DataMap.Resource_Type.gbaseCluster.value,
          jobTargetType: []
        },
        {
          id: 'lightcloudgaussdb',
          slaId: ApplicationType.LightCloudGaussDB,
          key: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
          hide: !includes(items, ApplicationType.LightCloudGaussDB),
          label: this.i18n.get('protection_light_cloud_gaussdb_label'),
          prefix: 'G',
          color: '#C8000C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostApLightCloudGaussDB,
          copyUrl: RouterUrl.ExploreCopyDataLightCloudGaussdb,
          tabs: [
            {
              label: this.i18n.get('common_project_label'),
              resType: DataMap.Resource_Type.lightCloudGaussdbProject.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.lightCloudGaussdbInstance.value
            }
          ],
          resourceSetType: ResourceSetType.GaussDB,
          jobTargetType: [
            DataMap.Job_Target_Type.lightCloudGaussdbProject.value,
            DataMap.Job_Target_Type.lightCloudGaussdbInstance.value
          ]
        }
      ],
      bigData: [
        {
          id: 'mongodb',
          slaId: ApplicationType.MongoDB,
          key: [
            DataMap.Resource_Type.MongodbClusterInstance.value,
            DataMap.Resource_Type.MongodbSingleInstance.value
          ],
          hide: !includes(items, ApplicationType.MongoDB),
          label: this.i18n.get('MongoDB'),
          prefix: 'M',
          color: '#126149',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppMongoDB,
          copyUrl: RouterUrl.ExploreCopyDataMongoDB,
          resType: DataMap.Resource_Type.MongoDB.value,
          resourceSetType: ResourceSetType.MongoDB,
          jobTargetType: [
            DataMap.Job_Target_Type.mongodbSingleInstance.value,
            DataMap.Job_Target_Type.mongodbClusterInstance.value
          ]
        },
        {
          id: 'redis',
          slaId: ApplicationType.Redis,
          key: DataMap.Resource_Type.Redis.value,
          hide: !includes(items, ApplicationType.Redis),
          label: this.i18n.get('Redis'),
          prefix: 'R',
          color: '#DC382C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppRedis,
          copyUrl: RouterUrl.ExploreCopyDataRedis,
          resType: DataMap.Resource_Type.Redis.value,
          resourceSetType: ResourceSetType.Redis,
          jobTargetType: [DataMap.Job_Target_Type.Redis.value]
        },
        {
          id: 'gaussdbdws',
          slaId: ApplicationType.GaussDBDWS,
          key: [
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value
          ],
          hide: !includes(items, ApplicationType.GaussDBDWS),
          label: this.i18n.get('common_dws_label'),
          prefix: 'D',
          color: '#B12023',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppGaussDBDWS,
          copyUrl: RouterUrl.ExploreCopyDataGaussDBDWS,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.DWS_Cluster.value
            },
            {
              label: this.i18n.get('protection_schema_set_label'),
              resType: DataMap.Resource_Type.DWS_Schema.value
            },
            {
              label: this.i18n.get('protection_table_set_label'),
              resType: DataMap.Resource_Type.DWS_Table.value
            }
          ],
          resourceSetType: ResourceSetType.GaussDB_DWS,
          jobTargetType: [
            DataMap.Job_Target_Type.dwsCluster.value,
            DataMap.Job_Target_Type.dwsSchema.value,
            DataMap.Job_Target_Type.dwsTable.value
          ]
        },
        {
          id: 'clickhouse',
          slaId: ApplicationType.ClickHouse,
          key: DataMap.Resource_Type.ClickHouse.value,
          hide: !includes(items, ApplicationType.ClickHouse),
          label: this.i18n.get('ClickHouse'),
          prefix: 'C',
          color: '#131312',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppClickHouse,
          copyUrl: RouterUrl.ExploreCopyDataClickHouse,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.ClickHouseCluster.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.ClickHouseDatabase.value
            },
            {
              label: this.i18n.get('protection_table_set_label'),
              resType: DataMap.Resource_Type.ClickHouseTableset.value
            }
          ],
          resourceSetType: ResourceSetType.ClickHouse,
          jobTargetType: [DataMap.Job_Target_Type.ClickHouse.value]
        },
        {
          id: 'hdfs',
          slaId: ApplicationType.HDFS,
          key: DataMap.Resource_Type.HDFSFileset.value,
          hide: !includes(items, ApplicationType.HDFS),
          label: this.i18n.get('HDFS'),
          prefix: 'H',
          color: '#6CD0F7',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionBigDataHdfs,
          copyUrl: RouterUrl.ExploreCopyDataHdfs,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.HDFS.value
            },
            {
              label: this.i18n.get('common_fileset_label'),
              resType: DataMap.Resource_Type.HDFSFileset.value
            }
          ],
          resourceSetType: ResourceSetType.HDFS,
          jobTargetType: [DataMap.Job_Target_Type.HDFSFileset.value]
        },
        {
          id: 'hbase',
          slaId: ApplicationType.HBase,
          key: DataMap.Resource_Type.HBaseBackupSet.value,
          hide: !includes(items, ApplicationType.HBase),
          label: this.i18n.get('HBase'),
          prefix: 'H',
          color: '#BA170C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionBigDataHbase,
          copyUrl: RouterUrl.ExploreCopyDataHbase,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.HBase.value
            },
            {
              label: this.i18n.get('protection_backup_set_label'),
              resType: DataMap.Resource_Type.HBaseBackupSet.value
            }
          ],
          resourceSetType: ResourceSetType.HBase,
          jobTargetType: [DataMap.Job_Target_Type.HBaseBackupSet.value]
        },
        {
          id: 'hive',
          slaId: ApplicationType.Hive,
          key: DataMap.Resource_Type.HiveBackupSet.value,
          hide: !includes(items, ApplicationType.Hive),
          label: this.i18n.get('Hive'),
          prefix: 'H',
          color: '#FDEF21',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionBigDataHive,
          copyUrl: RouterUrl.ExploreCopyDataHive,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.Hive.value
            },
            {
              label: this.i18n.get('protection_backup_set_label'),
              resType: DataMap.Resource_Type.HiveBackupSet.value
            }
          ],
          resourceSetType: ResourceSetType.Hive,
          jobTargetType: [DataMap.Job_Target_Type.HiveBackupSet.value]
        },
        {
          id: 'elasticsearch',
          slaId: ApplicationType.Elasticsearch,
          key: DataMap.Resource_Type.ElasticsearchBackupSet.value,
          hide: !includes(items, ApplicationType.Elasticsearch),
          label: this.i18n.get('Elasticsearch'),
          prefix: 'E',
          color: '#07BFB4',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionBigDataElasticsearch,
          copyUrl: RouterUrl.ExploreCopyDataElasticsearch,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.Elasticsearch.value
            },
            {
              label: this.i18n.get('protection_backup_set_label'),
              resType: DataMap.Resource_Type.ElasticsearchBackupSet.value
            }
          ],
          resourceSetType: ResourceSetType.Elasticsearch,
          jobTargetType: [DataMap.Job_Target_Type.ElasticSearch.value]
        }
      ],
      virtualization: [
        {
          id: 'vmware',
          slaId: ApplicationType.Vmware,
          key: [
            DataMap.Resource_Type.virtualMachine.value,
            DataMap.Resource_Type.hostSystem.value,
            DataMap.Resource_Type.clusterComputeResource.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.virtualMachine.value,
            DataMap.Resource_Type.hostSystem.value,
            DataMap.Resource_Type.clusterComputeResource.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.Vmware),
          label: this.i18n.get('VMware'),
          prefix: 'V',
          color: '#717074',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationVmware,
          copyUrl: RouterUrl.ExploreCopyDataVMware,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationVmware,
          antiUrl: RouterUrl.ExploreAntiApplicationVmware,
          resType: ApplicationType.Vmware,
          resourceSetType: ResourceSetType.VMware,
          jobTargetType: [
            DataMap.Job_Target_Type.VMware.value,
            DataMap.Job_Target_Type.VMwarevCenterServer.value,
            DataMap.Job_Target_Type.clusterComputeResource.value,
            DataMap.Job_Target_Type.vmwareHostSystem.value,
            DataMap.Job_Target_Type.vmwareEsx.value,
            DataMap.Job_Target_Type.vmwareEsxi.value,
            DataMap.Job_Target_Type.vmware.value
          ]
        },
        {
          id: 'cnware',
          slaId: ApplicationType.CNware,
          key: [
            DataMap.Resource_Type.cNwareCluster.value,
            DataMap.Resource_Type.cNwareHost.value,
            DataMap.Resource_Type.cNwareVm.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.cNwareCluster.value,
            DataMap.Resource_Type.cNwareHost.value,
            DataMap.Resource_Type.cNwareVm.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.CNware),
          label: this.i18n.get('common_cnware_label'),
          prefix: 'C',
          color: '#EA1E28',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationCnware,
          copyUrl: RouterUrl.ExploreCopyDataCNware,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationCnware,
          antiUrl: RouterUrl.ExploreAntiApplicationCnware,
          resType: ResourceType.CNWARE,
          resourceSetType: ResourceSetType.CNware,
          jobTargetType: [
            DataMap.Job_Target_Type.cNware.value,
            DataMap.Job_Target_Type.cNwareHostPool.value,
            DataMap.Job_Target_Type.cNwareCluster.value,
            DataMap.Job_Target_Type.cNwareHost.value,
            DataMap.Job_Target_Type.cNwareVm.value
          ]
        },
        {
          id: 'fusioncompute',
          slaId: ApplicationType.FusionCompute,
          key: DataMap.Resource_Type.FusionCompute.value,
          hide: !includes(items, ApplicationType.FusionCompute),
          label: this.i18n.get('FusionCompute'),
          prefix: 'F',
          color: '#C80A2B',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationFusionCompute,
          copyUrl: RouterUrl.ExploreCopyDataFusionCompute,
          resType: ApplicationType.FusionCompute,
          resourceSetType: ResourceSetType.FusionCompute,
          jobTargetType: [
            DataMap.Job_Target_Type.FusionComputePlatform.value,
            DataMap.Job_Target_Type.FusionComputeCluster.value,
            DataMap.Job_Target_Type.FusionComputeHost.value,
            DataMap.Job_Target_Type.FusionCompute.value
          ]
        },
        {
          id: 'hyper-v',
          slaId: ApplicationType.HyperV,
          key: [
            DataMap.Resource_Type.hyperVHost.value,
            DataMap.Resource_Type.hyperVCluster.value,
            DataMap.Resource_Type.hyperVVm.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.hyperVHost.value,
            DataMap.Resource_Type.hyperVCluster.value,
            DataMap.Resource_Type.hyperVVm.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.HyperV),
          label: this.i18n.get('common_hyperv_label'),
          prefix: 'H',
          color: '#C80A2B',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationHyperV,
          copyUrl: RouterUrl.ExploreCopyDataHyperv,
          resType: ResourceType.HYPERV,
          resourceSetType: ResourceSetType.HyperV,
          jobTargetType: [
            DataMap.Job_Target_Type.hypervScvmm.value,
            DataMap.Job_Target_Type.hypervCluster.value,
            DataMap.Job_Target_Type.hypervHost.value,
            DataMap.Job_Target_Type.hypervVM.value
          ]
        },
        {
          id: 'fusionone',
          slaId: ApplicationType.FusionOne,
          key: DataMap.Resource_Type.fusionOne.value,
          resourceSetKey: [DataMap.Resource_Type.fusionOne.value],
          hide: !includes(items, ApplicationType.FusionOne),
          label: this.i18n.get('protection_fusionone_label'),
          prefix: 'F',
          color: '#C80A2B',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationFusionOne,
          copyUrl: RouterUrl.ExploreCopyDataFusionOne,
          resType: ApplicationType.FusionOne,
          resourceSetType: ResourceSetType.FusionOne,
          jobTargetType: [
            DataMap.Job_Target_Type.FusionOneComputePlatform.value,
            DataMap.Job_Target_Type.FusionOneComputeCluster.value,
            DataMap.Job_Target_Type.FusionOneComputeHost.value,
            DataMap.Job_Target_Type.FusionOneCompute.value
          ]
        },
        {
          id: 'nutanix',
          slaId: ApplicationType.Nutanix,
          key: [
            DataMap.Resource_Type.nutanixCluster.value,
            DataMap.Resource_Type.nutanixHost.value,
            DataMap.Resource_Type.nutanixVm.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.nutanixCluster.value,
            DataMap.Resource_Type.nutanixHost.value,
            DataMap.Resource_Type.nutanixVm.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.Nutanix),
          label: this.i18n.get('common_nutanix_label'),
          prefix: 'N',
          color: '#316CE6',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationNutanix,
          copyUrl: RouterUrl.ExploreCopyDataNutanix,
          antiUrl: RouterUrl.ExploreAntiApplicationNutanix,
          resType: ResourceType.NUTANIX,
          resourceSetType: ResourceSetType.Nutanix,
          jobTargetType: [
            DataMap.Job_Target_Type.nutanix.value,
            DataMap.Job_Target_Type.nutanixCluster.value,
            DataMap.Job_Target_Type.nutanixHost.value,
            DataMap.Job_Target_Type.nutanixVm.value
          ]
        }
      ],
      container: [
        {
          id: 'kubernetes',
          slaId: ApplicationType.KubernetesStatefulSet,
          key: [
            DataMap.Resource_Type.KubernetesNamespace.value,
            DataMap.Resource_Type.KubernetesStatefulset.value
          ],
          hide: !includes(items, ApplicationType.KubernetesStatefulSet),
          label: this.i18n.get('protection_kubernetes_flexvolume_label'),
          tooltip: this.i18n.get('protection_kubernetes_flexvolume_tip_label'),
          prefix: 'K',
          color: '#316CE6',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationKubernetes,
          copyUrl: RouterUrl.ExploreCopyDataKubernetes,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.Kubernetes.value
            },
            {
              label: this.i18n.get('protection_name_space_label'),
              resType: DataMap.Resource_Type.KubernetesNamespace.value
            },
            {
              label: this.i18n.get('protection_statefulset_label'),
              resType: DataMap.Resource_Type.KubernetesStatefulset.value
            }
          ],
          resourceSetType: ResourceSetType.Kubernetes_FlexVolume,
          jobTargetType: [
            DataMap.Job_Target_Type.kubernetes.value,
            DataMap.Job_Target_Type.kubernetesNamespace.value,
            DataMap.Job_Target_Type.kubernetesStatefulSet.value
          ]
        },
        {
          id: 'kubernetes-container',
          slaId: ApplicationType.KubernetesDatasetCommon,
          key: [
            DataMap.Resource_Type.kubernetesNamespaceCommon.value,
            DataMap.Resource_Type.kubernetesDatasetCommon.value
          ],
          hide: !includes(items, ApplicationType.KubernetesDatasetCommon),
          label: this.i18n.get('protection_kubernetes_container_label'),
          tooltip: this.i18n.get('protection_kubernetes_container_tip_label'),
          prefix: 'K',
          color: '#316CE6',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionVirtualizationKubernetesContainer,
          copyUrl: RouterUrl.ExploreCopyDataKubernetesContainer,
          tabs: [
            {
              label: this.i18n.get('common_cluster_label'),
              resType: DataMap.Resource_Type.kubernetesClusterCommon.value
            },
            {
              label: this.i18n.get('protection_name_space_label'),
              resType: DataMap.Resource_Type.kubernetesNamespaceCommon.value
            },
            {
              label: this.i18n.get('protection_kubernetes_dataset_label'),
              resType: DataMap.Resource_Type.kubernetesDatasetCommon.value
            }
          ],
          resourceSetType: ResourceSetType.Kubernetes_CSI,
          jobTargetType: [
            DataMap.Job_Target_Type.kubernetesClusterCommon.value,
            DataMap.Job_Target_Type.kubernetesNamespaceCommon.value,
            DataMap.Job_Target_Type.kubernetesDatasetCommon.value
          ]
        }
      ],
      cloud: [
        {
          id: 'hcscloudhost',
          slaId: ApplicationType.HCSCloudHost,
          key: [
            DataMap.Resource_Type.HCSProject.value,
            DataMap.Resource_Type.HCSCloudHost.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.HCSTenant.value,
            DataMap.Resource_Type.HCSProject.value,
            DataMap.Resource_Type.HCSCloudHost.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.HCSCloudHost),
          label: this.i18n.get('common_cloud_label'),
          prefix: 'H',
          color: '#C8000C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionCloudHuaweiStack,
          copyUrl: RouterUrl.ExploreCopyDataHuaweiStack,
          resType: ApplicationType.HCSCloudHost,
          resourceSetType: ResourceSetType.HCSStack,
          jobTargetType: [
            DataMap.Job_Target_Type.HCSContainer.value,
            DataMap.Job_Target_Type.hcsEnvOp.value,
            DataMap.Job_Target_Type.HCSTenant.value,
            DataMap.Job_Target_Type.HCSProject.value,
            DataMap.Job_Target_Type.HCSCloudHost.value
          ]
        },
        {
          id: 'openstack',
          slaId: ApplicationType.OpenStack,
          key: [
            DataMap.Resource_Type.openStackProject.value,
            DataMap.Resource_Type.openStackCloudServer.value
          ],
          resourceSetKey: [
            ResourceType.OpenStackDomain,
            DataMap.Resource_Type.openStackProject.value,
            DataMap.Resource_Type.openStackCloudServer.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.OpenStack),
          label: this.i18n.get('common_open_stack_label'),
          prefix: 'O',
          color: '#BD3725',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionCloudOpenstack,
          copyUrl: RouterUrl.ExploreCopyDataOpenStack,
          resType: ApplicationType.OpenStack,
          resourceSetType: ResourceSetType.OpenStack,
          jobTargetType: [
            DataMap.Job_Target_Type.Openstack.value,
            DataMap.Job_Target_Type.OpenStackProject.value,
            DataMap.Job_Target_Type.OpenStackCloudServer.value
          ]
        },
        {
          id: 'gaussdbforopengauss',
          slaId: ApplicationType.GaussDBForOpenGauss,
          key: DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
          hide: !includes(items, ApplicationType.GaussDBForOpenGauss),
          label: this.i18n.get('protection_gaussdb_for_opengauss_label'),
          prefix: 'H',
          color: '#C8000C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppGaussDBForOpengauss,
          copyUrl: RouterUrl.ExploreCopyDataGaussdbForOpengauss,
          tabs: [
            {
              label: this.i18n.get('common_project_label'),
              resType: DataMap.Resource_Type.gaussdbForOpengaussProject.value
            },
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.gaussdbForOpengaussInstance.value
            }
          ],
          resourceSetType: ResourceSetType.HCSStack_GaussDB,
          jobTargetType: [
            DataMap.Job_Target_Type.gaussdbForOpengaussInstance.value,
            DataMap.Job_Target_Type.gaussdbForOpengaussProject.value
          ]
        },
        {
          id: 'apsarastack',
          slaId: ApplicationType.ApsaraStack,
          key: [
            DataMap.Resource_Type.APSCloudServer.value,
            DataMap.Resource_Type.APSZone.value,
            DataMap.Resource_Type.APSResourceSet.value
          ],
          resourceSetKey: [
            DataMap.Resource_Type.APSCloudServer.value,
            DataMap.Resource_Type.APSZone.value,
            DataMap.Resource_Type.APSResourceSet.value,
            ResourceSetType.RESOURCE_GROUP
          ],
          hide: !includes(items, ApplicationType.ApsaraStack),
          label: this.i18n.get('protection_ali_cloud_label'),
          prefix: 'A',
          color: '#F86603',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionApsaraStack,
          copyUrl: RouterUrl.ExploreCopyDataApsaraStack,
          resourceSetType: ResourceSetType.ApsaraStack,
          resType: ApplicationType.ApsaraStack,
          jobTargetType: [
            DataMap.Job_Target_Type.ApsaraStack.value,
            DataMap.Job_Target_Type.APSResourceSet.value,
            DataMap.Job_Target_Type.APSCloudServer.value,
            DataMap.Job_Target_Type.APSZone.value
          ]
        }
      ],
      fileService: [
        {
          id: 'nasfilesystem',
          slaId: ApplicationType.NASFileSystem,
          key: DataMap.Resource_Type.NASFileSystem.value,
          hide:
            this.isDistributed ||
            this.isDecouple ||
            !includes(items, ApplicationType.NASFileSystem),
          label: this.i18n.get('common_nas_file_systems_label'),
          prefix: 'N',
          color: '#EBAA44',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionDoradoFileSystem,
          copyUrl: RouterUrl.ExploreCopyDataFileSystem,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationFileSystem,
          antiUrl: RouterUrl.ExploreAntiApplicationDoradoFileSystem,
          resType: DataMap.Resource_Type.NASFileSystem.value,
          resourceSetType: ResourceSetType.NasFileSystem,
          jobTargetType: [DataMap.Job_Target_Type.NASFileSystem.value]
        },
        {
          id: 'nasshare',
          slaId: ApplicationType.NASShare,
          key: DataMap.Resource_Type.NASShare.value,
          hide: !includes(items, ApplicationType.NASShare),
          label: this.i18n.get('common_nas_shares_label'),
          prefix: 'N',
          color: '#EBAA44',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionNasShared,
          copyUrl: RouterUrl.ExploreCopyDataNasShared,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationNasshare,
          antiUrl: RouterUrl.ExploreAntiApplicationNasShared,
          resType: DataMap.Resource_Type.NASShare.value,
          resourceSetType: ResourceSetType.NasShare,
          jobTargetType: [DataMap.Job_Target_Type.NASShare.value]
        },
        {
          id: 'ndmp',
          slaId: ApplicationType.Ndmp,
          key: DataMap.Resource_Type.ndmp.value,
          hide: !includes(items, ApplicationType.Ndmp),
          label: this.i18n.get('protection_ndmp_protocol_label'),
          prefix: 'N',
          color: '#EBAA44',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionNdmp,
          copyUrl: RouterUrl.ExploreCopyDataNdmp,
          resType: DataMap.Resource_Type.ndmp.value,
          tabs: [
            {
              label: this.i18n.get('common_file_systems_label'), //修改名称需要注意同步修改资源集那里
              resType: DataMap.Resource_Type.ndmp.value
            },
            {
              label: this.i18n.get('common_file_directory_label'),
              resType: DataMap.Resource_Type.ndmp.value
            }
          ],
          resourceSetType: ResourceSetType.Ndmp,
          jobTargetType: [
            DataMap.Job_Target_Type.ndmp.value,
            DataMap.Job_Target_Type.ndmpServer.value
          ]
        },
        {
          id: 'commonShare',
          slaId: ApplicationType.CommonShare,
          key: DataMap.Resource_Type.commonShare.value,
          hide:
            this.isDistributed ||
            this.isDecouple ||
            !includes(items, ApplicationType.CommonShare),
          label: this.i18n.get('protection_commonshare_label'),
          prefix: 'C',
          color: '#F86603',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionCommonShare,
          copyUrl: RouterUrl.ExploreCopyDataCommonShare,
          resType: DataMap.Resource_Type.commonShare.value,
          resourceSetType: ResourceSetType.CommonShare,
          jobTargetType: [DataMap.Job_Target_Type.commonShare.value]
        },
        {
          id: 'objectStorage',
          slaId: ApplicationType.ObjectStorage,
          key: DataMap.Resource_Type.ObjectSet.value,
          hide: !includes(items, ApplicationType.ObjectStorage),
          label: this.i18n.get('common_object_storage_label'),
          prefix: 'O',
          color: '#C8000C',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionObject,
          copyUrl: RouterUrl.ExploreCopyDataObject,
          tabs: [
            {
              label: this.i18n.get('common_object_storage_label'),
              resType: DataMap.Resource_Type.ObjectStorage.value
            },
            {
              label: this.i18n.get('protection_object_set_label'),
              resType: DataMap.Resource_Type.ObjectSet.value
            }
          ],
          resourceSetType: ResourceSetType.ObjectStorage,
          jobTargetType: [DataMap.Job_Target_Type.ObjectSet.value]
        },
        {
          id: 'fileset',
          slaId: ApplicationType.Fileset,
          key: DataMap.Resource_Type.fileset.value,
          hide: !includes(items, ApplicationType.Fileset),
          label: this.i18n.get('common_fileset_label'),
          prefix: 'F',
          color: '#EBAA44',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppFilesetTemplate,
          copyUrl: RouterUrl.ExploreCopyDataFileset,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationFileset,
          antiUrl: RouterUrl.ExploreAntiApplicationFileset,
          tabs: [
            {
              label: this.i18n.get('common_fileset_label'),
              resType: DataMap.Resource_Type.fileset.value
            },
            {
              label: this.i18n.get('common_template_label'),
              resType: 'filesetTemplate'
            }
          ],
          resourceSetType: ResourceSetType.Fileset,
          jobTargetType: [DataMap.Job_Target_Type.Fileset.value]
        },
        {
          id: 'volume',
          slaId: ApplicationType.Volume,
          key: DataMap.Resource_Type.volume.value,
          hide: !includes(items, ApplicationType.Volume),
          label: this.i18n.get('protection_volumes_label'),
          prefix: 'V',
          color: '#000000',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppVolume,
          copyUrl: RouterUrl.ExploreCopyDataVolume,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationVolume,
          resType: DataMap.Resource_Type.volume.value,
          resourceSetType: ResourceSetType.Volume,
          jobTargetType: [DataMap.Job_Target_Type.volume.value]
        }
      ],
      application: [
        {
          id: 'activedirectory',
          slaId: ApplicationType.ActiveDirectory,
          key: DataMap.Resource_Type.ActiveDirectory.value,
          hide:
            !includes(items, ApplicationType.ActiveDirectory) ||
            this.isDistributed,
          label: this.i18n.get('Active Directory'),
          prefix: 'A',
          color: '#717074',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionActiveDirectory,
          copyUrl: RouterUrl.ExploreCopyDataActiveDirectory,
          resType: DataMap.Resource_Type.ActiveDirectory.value,
          resourceSetType: ResourceSetType.ADDS,
          jobTargetType: [DataMap.Job_Target_Type.ActiveDirectory.value]
        },
        {
          id: 'exchange',
          slaId: ApplicationType.Exchange,
          key: [
            DataMap.Resource_Type.ExchangeDataBase.value,
            DataMap.Resource_Type.ExchangeGroup.value,
            DataMap.Resource_Type.ExchangeSingle.value,
            DataMap.Resource_Type.ExchangeEmail.value
          ],
          hide:
            !includes(items, ApplicationType.Exchange) || this.isDistributed,
          label: this.i18n.get('Exchange'),
          prefix: 'E',
          color: '#0072C6',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppExchange,
          copyUrl: RouterUrl.ExploreCopyDataDatabaseExchange,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationExchange,
          tabs: [
            {
              label: this.i18n.get('protection_host_cluster_group_name_label'),
              resType: DataMap.Resource_Type.Exchange.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.ExchangeDataBase.value
            },
            {
              label: this.i18n.get('common_email_label'),
              resType: DataMap.Resource_Type.ExchangeEmail.value
            }
          ],
          resourceSetType: ResourceSetType.Exchange,
          jobTargetType: [
            DataMap.Job_Target_Type.ExchangeSingle.value,
            DataMap.Job_Target_Type.ExchangeGroup.value,
            DataMap.Job_Target_Type.ExchangeDataBase.value,
            DataMap.Job_Target_Type.ExchangeEmail.value
          ]
        },
        {
          id: 'saphana',
          slaId: ApplicationType.SapHana,
          key: [
            DataMap.Resource_Type.saphanaInstance.value,
            DataMap.Resource_Type.saphanaDatabase.value
          ],
          hide: !includes(items, ApplicationType.SapHana),
          label: this.i18n.get('protection_saphana_label'),
          prefix: 'S',
          color: '#06A6E8',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppSapHana,
          copyUrl: RouterUrl.ExploreCopyDataSapHana,
          tabs: [
            {
              label: this.i18n.get('protection_database_instance_label'),
              resType: DataMap.Resource_Type.saphanaInstance.value
            },
            {
              label: this.i18n.get('common_database_label'),
              resType: DataMap.Resource_Type.saphanaDatabase.value
            }
          ],
          resourceSetType: ResourceSetType.SAP_HANA,
          jobTargetType: [
            DataMap.Job_Target_Type.saphanaInstance.value,
            DataMap.Job_Target_Type.saphanaDatabase.value
          ]
        },
        {
          id: 'Saponoracle',
          slaId: ApplicationType.Saponoracle,
          key: [DataMap.Resource_Type.saponoracleDatabase.value],
          hide: !includes(items, ApplicationType.Saponoracle) || this.isHcsUser,
          label: this.i18n.get('common_sap_on_oracle_label'),
          prefix: 'S',
          color: '#006DB8',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionHostAppSaponoracle,
          copyUrl: RouterUrl.ExploreCopyDataSaponoracle,
          resType: DataMap.Resource_Type.saponoracleDatabase.value,
          resourceSetType: ResourceSetType.SAP_ON_ORACLE,
          jobTargetType: [DataMap.Job_Target_Type.saponoracleDatabase.value]
        }
      ],
      bareMetal: [
        {
          id: 'fileset',
          slaId: ApplicationType.Fileset,
          key: DataMap.Resource_Type.fileset.value,
          hide: !includes(items, ApplicationType.Fileset),
          label: this.i18n.get('common_fileset_label'),
          prefix: 'F',
          color: '#EBAA44',
          protected_count: 0,
          count: 0,
          protectionUrl: RouterUrl.ProtectionBareMetalFilesetTemplate,
          copyUrl: RouterUrl.ExploreCopyDataBareMetalFileset,
          livemountUrl: RouterUrl.ExploreLiveMountApplicationFileset,
          antiUrl: RouterUrl.ExploreAntiApplicationFileset,
          jobTargetType: [DataMap.Job_Target_Type.Fileset.value]
        }
      ]
    };
    each(resource, (value, key) => {
      resource[key] = filter(value, (item: any) => !item.hide);
    });
    return resource;
  }

  getResourceType(subType): string {
    const apps = [];
    each(values(this.getApplicationConfig()), item => {
      apps.push(...item);
    });
    const app = find(apps, item => {
      return includes(item.key, subType);
    });
    if (app) {
      return app.label;
    }
    return subType;
  }

  getProxyOptions(self, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        type: 'Host',
        subType: [DataMap.Resource_Type.UBackupAgent.value],
        scenario: [['!='], '1'],
        isCluster: [['=='], false]
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        recordsTemp = recordsTemp.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        each(recordsTemp, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        self.proxyOptions = hostArray;
        return;
      }
      this.getProxyOptions(self, recordsTemp, startPage);
    });
  }

  getDetectionPolicy(backupItem, datePipe): string {
    const retentionLabel =
      backupItem.retention?.retention_type ===
      RetentionType.PERMANENTLY_RETAINED
        ? this.i18n.get('explore_permanent_retention_label')
        : this.i18n.get('explore_execute_retention_label', [
            backupItem.retention?.retention_duration,
            this.dataMapService.getLabel(
              'Interval_Unit',
              backupItem.retention?.duration_unit
            )
          ]);
    if (backupItem.schedule?.trigger_action === TRIGGER_TYPE.week) {
      return this.i18n.get('explore_execute_week_label', [
        map(backupItem.schedule?.days_of_week, item => {
          return this.dataMapService.getLabel('Days_Of_Week', item);
        }).join(this.i18n.isEn ? ',' : '，'),
        retentionLabel
      ]);
    } else if (backupItem.schedule?.trigger_action === TRIGGER_TYPE.month) {
      if (
        backupItem.schedule?.days_of_month ===
        DataMap.Days_Of_Month_Type.lastDay.value
      ) {
        return this.i18n.get('explore_execute_month_label', [retentionLabel]);
      } else {
        return this.i18n.get('explore_execute_months_label', [
          this.i18n.isEn
            ? backupItem.schedule?.days_of_month
            : backupItem.schedule?.days_of_month.replace(/,/g, '，'),
          retentionLabel
        ]);
      }
    } else if (backupItem.schedule?.trigger_action === TRIGGER_TYPE.year) {
      return this.i18n.get('explore_execute_year_label', [
        datePipe.transform(backupItem.schedule?.days_of_year, 'MM-dd'),
        retentionLabel
      ]);
    } else {
      if (
        backupItem.schedule?.interval_unit === DataMap.Interval_Unit.day.value
      ) {
        return this.i18n.get('explore_execute_day_label', [
          datePipe.transform(
            backupItem.schedule?.start_time,
            'yyyy-MM-dd HH:mm:ss'
          ),
          backupItem.schedule?.interval,
          retentionLabel
        ]);
      } else {
        return this.i18n.get('explore_execute_hour_label', [
          datePipe.transform(
            backupItem.schedule?.start_time,
            'yyyy-MM-dd HH:mm:ss'
          ),
          backupItem.schedule?.interval,
          retentionLabel
        ]);
      }
    }
  }

  getCyberEngineStorage(): Observable<any> {
    return this.protectedResourceApiService
      .ListResources({
        akLoading: false,
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 5,
        conditions: JSON.stringify({
          type: 'StorageEquipment',
          subType: [['!='], DataMap.Device_Storage_Type.Other.value]
        })
      })
      .pipe(
        _map(res => {
          res.records = filter(
            res.records,
            item =>
              item.extendInfo?.detectType !==
              DataMap.storageDeviceDetectType.inDevice.value
          );
          res.totalCount = size(res.records);
          return res;
        })
      );
  }

  getRestoreOptions(options, callback) {
    this.protectedResourceApiService
      .CheckAllowRestore({
        resourceIds: String(
          options
            .map(item => {
              return item.uuid;
            })
            .join(',')
        )
      })
      .subscribe(res => {
        callback(
          options.map(item => {
            const tmpData = find(res, { uuid: item.uuid });
            return {
              ...item,
              disabled: get(tmpData, 'isAllowRestore', 'true') === 'false'
            };
          })
        );
      });
  }

  getResourceByRecursion(
    extParams,
    resourceAction,
    callback,
    specialPage?,
    recordsTemp?,
    startPage?
  ) {
    const start = specialPage
      ? CommonConsts.PAGE_START + 1
      : CommonConsts.PAGE_START;
    const params = {
      [specialPage ? 'pageNum' : 'pageNo']: startPage || start,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      ...extParams
    };
    resourceAction(params).subscribe(
      res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = start;
        }
        startPage++;
        if (extParams?.startPage) {
          let page = cloneDeep(extParams?.startPage);
          page++;
          set(extParams, 'startPage', page);
        }
        recordsTemp = [...recordsTemp, ...res.records];
        const page = specialPage
          ? Math.ceil(res.totalCount / CommonConsts.MAX_PAGE_SIZE) + 1
          : Math.ceil(res.totalCount / CommonConsts.MAX_PAGE_SIZE);
        if (startPage === page || res.totalCount === 0) {
          if (isFunction(callback)) {
            callback(recordsTemp);
          }
          return;
        }
        this.getResourceByRecursion(
          extParams,
          resourceAction,
          callback,
          specialPage,
          recordsTemp,
          startPage
        );
      },
      () => {
        if (isFunction(callback)) {
          callback([]);
        }
      }
    );
  }

  validLabel(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }
      if (control.value.split('/')?.length === 2) {
        if (
          isEmpty(control.value.split('/')[0]) ||
          includes(control.value.split('/')[0], '=') ||
          !CommonConsts.REGEX.label.test(control.value.split('/')[1])
        ) {
          return {
            invalidLabel: { value: control.value }
          };
        }
        return null;
      } else if (control.value.split('/')?.length > 2) {
        return {
          invalidLabel: { value: control.value }
        };
      } else {
        if (!CommonConsts.REGEX.label.test(control.value)) {
          return {
            invalidLabel: { value: control.value }
          };
        }
        return null;
      }
    };
  }

  openSpecialHelp(url: string | string[]) {
    setTimeout(() => {
      const doms = document.getElementsByClassName('special-help-link');
      each(doms, (dom, index) => {
        const targetUrl = isArray(url) ? url[index] : url;
        dom.addEventListener('click', () => {
          if (this.isHcsUser) {
            const herf: string = first(window.location.href.split('#'));
            window.open(herf.replace('/console/', targetUrl), '_blank');
          } else if (this.isDmeUser) {
            const herf: string = first(window.location.href.split('#'));
            window.open(
              herf.replace('/console/index.html', targetUrl),
              '_blank'
            );
          } else {
            window.open(
              targetUrl.replace('/a8000/', `/${this.helpPkg}/`),
              '_blank'
            );
          }
        });
      });
    }, 300);
  }

  openRouter(url: string) {
    setTimeout(() => {
      const dom = first(document.getElementsByClassName('special-router-link'));
      if (dom) {
        dom.addEventListener('click', () => {
          if (this.isHcsUser && window.parent) {
            const parentUrl = window.parent.location.href;
            window.parent.location.href = `${first(
              parentUrl.split('#')
            )}#${url}`;
          } else {
            this.router.navigateByUrl(url);
          }
        });
      }
    }, 300);
  }

  // 获取帮助文档跳转链接
  getHelpUrl() {
    const type = this.router.url.split('/')[2] || '';
    // 去除字符串中的'-'
    const newType = type.replace(/-/g, '');

    const baseUrl = this.i18n.isEn
      ? `/console/assets/help/${this.helpPkg}/en-us/index.html#${HelpUrlCode.en[newType]}.html`
      : `/console/assets/help/${this.helpPkg}/zh-cn/index.html#${HelpUrlCode.zh[newType]}.html`;

    if (this.isHcsUser) {
      const herf: string = first(window.location.href.split('#'));
      return herf.replace('/console/', baseUrl);
    }
    if (this.isDmeUser) {
      const herf: string = first(window.location.href.split('#'));
      return herf.replace('/console/index.html', baseUrl);
    }
    return `${location.origin}${baseUrl}`;
  }

  /**
   * 从this.getApplicationConfig()返回的对象resource中，取出key-valueKey对应的值组成新的对象
   * @param {string} key 对象中的键名
   * @param {string} valueKey 对象中的键值
   * @returns {Object} 返回key-valueKey对应的值组成的新对象
   */
  findResourceTypeByKey(key: string = 'id', valueKey: string = 'key'): object {
    return reduce(
      this.getApplicationConfig(),
      (resourceSubTypeObj, obj) => {
        each(values(obj), value => {
          resourceSubTypeObj[value[key]] = value[valueKey];
        });
        return resourceSubTypeObj;
      },
      {}
    );
  }

  getDuration(msTime): string {
    const time = msTime / 1000;
    let hour: any = Math.floor(time / 60 / 60);
    hour = hour.toString().padStart(2, '0');
    let minute: any = Math.floor(time / 60) % 60;
    minute = minute.toString().padStart(2, '0');
    let second: any = Math.floor(time) % 60;
    second = second.toString().padStart(2, '0');
    return `${hour}:${minute}:${second}`;
  }

  getZeroNum(num) {
    return num > 9 ? num : `0${num}`;
  }

  convertDateLongToString(dateLong: number): string {
    const date = new Date(dateLong);
    const year = date.getFullYear();
    const month = this.getZeroNum(date.getMonth() + 1);
    const day = this.getZeroNum(date.getDate());
    const h = this.getZeroNum(date.getHours());
    const min = this.getZeroNum(date.getMinutes());
    const s = this.getZeroNum(date.getSeconds());
    return `${year}-${month}-${day} ${h}:${min}:${s}`;
  }

  getCopyType(appType, software): string {
    if (includes([SoftwareType.CV, SoftwareType.NBU], software)) {
      if (includes([0x02, 0x07], appType)) {
        return this.i18n.get('common_vmware_label');
      } else if (includes([0x03, 0x10, 0x12], appType)) {
        return this.i18n.get('common_oracle_label');
      } else if (includes([0x08, 0x09], appType)) {
        return this.i18n.get('MySQL');
      } else {
        return this.i18n.get('common_file_system_label');
      }
    } else {
      if (includes([0x01, 0x04, 0x05, 0x06], appType)) {
        return this.i18n.get('common_file_system_label');
      } else if (includes([0x02, 0x07], appType)) {
        return this.i18n.get('common_vmware_label');
      } else {
        return this.i18n.get('explore_detection_copy_type_label');
      }
    }
  }

  /**
   * 文件导出
   * @param fileName 文件名称
   * @param blob 二进制流
   */
  downloadFile(fileName: string, blob: Blob) {
    const a = document.createElement('a');
    a.download = fileName;
    a.href = URL.createObjectURL(blob);
    a.click();
    setTimeout(() => {
      URL.revokeObjectURL(a.href);
    }, 1e4);
  }

  strToArrayBuffer(str) {
    const buffer = new ArrayBuffer(str.length);
    const viewArray = new Uint8Array(buffer);
    const len = str?.length > 0 ? str.length : 0;
    let i = 0;
    while (i < len) {
      viewArray[i] = str.charCodeAt(i);
      i++;
    }
    return buffer;
  }

  /**
   * 生成对应的arrayBuffer
   * @param pem 公钥key
   * @returns
   */

  pemToBuffer(pem) {
    const headerStr = `-----BEGIN PUBLIC KEY-----`;
    const footerStr = `-----END PUBLIC KEY-----`;
    const isStrValid = pem.includes(headerStr) && pem.includes(footerStr);
    if (!isStrValid) {
      throw new Error('invalid pem key string');
    }
    let headerLen = headerStr.length;
    if (pem[headerLen] === '\n') {
      headerLen++;
    }
    let footerLen = footerStr.length;
    if (pem[pem.length - footerStr.length] === '\n') {
      footerLen++;
    }
    if (pem[pem.length - 1] === '\n') {
      footerLen++;
    }
    const base64Contents = pem.substring(headerLen, pem.length - footerLen);
    const pemStr = window.atob(base64Contents);
    return this.strToArrayBuffer(pemStr);
  }

  /**
   * 导入密钥，生成CryptoKey对象
   * @param pem 公钥
   */
  async importRsaPublicKey(pem, hash) {
    const pemBuffer = this.pemToBuffer(pem);
    return await window.crypto.subtle.importKey(
      'spki',
      pemBuffer,
      {
        name: 'RSA-OAEP',
        hash
      },
      true,
      ['encrypt']
    );
  }

  utf8ToB64(str) {
    return unescape(encodeURIComponent(str));
  }

  arrayBufferToB64(buffer) {
    return window.btoa(String.fromCharCode.apply(null, new Uint8Array(buffer)));
  }

  strToUint8Array(str) {
    const arr = [];
    const len = str?.length > 0 ? str.length : 0;
    let i = 0;
    while (i < len) {
      arr.push(str.charCodeAt(i));
      i++;
    }
    return new Uint8Array(arr);
  }

  /**
   *
   * @param srcStr 要加密的参数
   * @param publicKey 加密公钥
   * @returns
   */
  async rsaEncrypt(srcStr, publicKey) {
    const encodedStr = this.strToUint8Array(srcStr);
    return await crypto.subtle.encrypt(
      {
        name: 'RSA-OAEP'
      },
      publicKey,
      encodedStr
    );
  }

  /**
   * 获取系统时间
   */
  getSystemTimeLong(akLoading = false): Observable<any> {
    return this.systemApiService
      .getSystemTimeUsingGET({
        akDoException: false,
        akLoading
      })
      .pipe(
        _map(res => {
          return new Date(res.time).getTime();
        })
      );
  }

  /**
   * 客户端时间的时间戳转为服务端时间的时间戳
   */

  toSystemTimeLong(timeString): number {
    // 客户端时间偏移量（小时）
    const clientTimeZoneOffset = new Date().getTimezoneOffset() / 60;
    return (
      new Date(timeString).getTime() -
      (clientTimeZoneOffset + SYSTEM_TIME.offset) * 3600 * 1e3
    );
  }

  /**
   * 此刻设置系统时间
   * @param formControl 需要赋值的表单控件
   */
  setTimePickerCurrent(formControl?: any) {
    const tempSysLong = new Date(SYSTEM_TIME.sysTime).getTime();
    const intervalTimeFromQuery =
      new Date().getTime() - SYSTEM_TIME.userSystemTime;
    const controlTime = new Date(tempSysLong + intervalTimeFromQuery);
    if (formControl) {
      formControl.setValue(controlTime);
    }
    return controlTime;
  }

  /**
   *
   * @returns 系统此刻时间，非精确
   */
  getCurrentSysTime(): number {
    const querySysTimeLong = this.toSystemTimeLong(SYSTEM_TIME.sysTime);
    const intervalTimeFromQuery =
      new Date().getTime() - SYSTEM_TIME.userSystemTime;
    return querySysTimeLong + intervalTimeFromQuery;
  }

  downloadUseAElement(url: string, fileName: string) {
    const a = document.createElement('a');
    let downloadUrl: string;
    const herf: string = first(window.location.href.split('#'));
    if (this.isHcsUser) {
      downloadUrl = herf.replace('/console/', `/console/rest${url}`);
    } else if (this.isDmeUser) {
      downloadUrl = herf.replace('/console/index.html', `/console/rest${url}`);
    } else {
      downloadUrl = `${location.protocol}//${location.host}/console/rest${url}`;
    }
    a.href = downloadUrl;
    a.setAttribute('target', '_blank');
    document.body.appendChild(a);
    a.download = fileName;
    a.click();
    document.body.removeChild(a);
  }

  /**
   *
   * @param targetServer 待查询的虚拟机或云服务器
   * @param resourceType 查询资源类型，例如：网卡、磁盘、数据存储等
   * @param paramsExt 扩展条件参数
   * @param conditionsExt conditions扩展条件参数
   * @param hasConditions 是否需要conditions
   */
  getResourcesDetails(
    targetServer,
    resourceType = '',
    paramsExt = {},
    conditionsExt = {},
    hasConditions = true
  ): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const sendEmpty = () => {
        observer.next([]);
        observer.complete();
      };
      if (!targetServer) {
        sendEmpty();
        return;
      }
      let agentsId = [];
      // 查询详情方法
      const listDetails = (
        agentId: string,
        recordsTemp = [],
        startPage = 1
      ) => {
        const conditions = { uuid: targetServer.uuid, ...conditionsExt };
        if (!isEmpty(resourceType)) {
          assign(conditions, { resourceType: resourceType });
        }
        const params = {
          agentId: agentId,
          envId: targetServer.rootUuid || targetServer.root_uuid,
          resourceIds: [targetServer.uuid || targetServer.root_uuid],
          pageNo: startPage,
          pageSize: CommonConsts.MAX_PAGE_SIZE,
          ...paramsExt,
          conditions: JSON.stringify(conditions)
        };
        if (!hasConditions) {
          delete params.conditions;
        }
        this.appService
          .ListResourcesDetails({ ...params, akDoException: isEmpty(agentsId) })
          .subscribe({
            next: res => {
              recordsTemp = [...recordsTemp, ...res.records];
              if (
                startPage ===
                  Math.ceil(res.totalCount / CommonConsts.MAX_PAGE_SIZE) ||
                res.totalCount === 0 ||
                res.totalCount === size(res.records)
              ) {
                observer.next(recordsTemp);
                observer.complete();
                return;
              }
              startPage++;
              listDetails(agentId, recordsTemp, startPage);
            },
            error: err => {
              // 如果是多代理注册、界面取到的代理状态是在线，实际是离线，PM有5分钟的更新时间，接口会报错。此时从已有列表中选取下一个重试下发。
              if (
                err?.error?.errorCode === '1677931275' &&
                !isEmpty(agentsId)
              ) {
                const agent = agentsId.shift();
                listDetails(agent);
              } else {
                sendEmpty();
              }
            }
          });
      };
      this.protectedResourceApiService
        .ListResources({
          pageNo: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          queryDependency: true,
          conditions: JSON.stringify({
            uuid: targetServer.rootUuid || targetServer.root_uuid
          })
        })
        .subscribe({
          next: (res: any) => {
            if (first(res.records)) {
              const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
                item =>
                  item.linkStatus ===
                  DataMap.resource_LinkStatus_Special.normal.value
              );
              if (isEmpty(onlineAgents)) {
                sendEmpty();
                return;
              }
              agentsId = map(onlineAgents, 'uuid');
              // 默认获取第一个agent
              const agent = agentsId.shift();
              listDetails(agent);
            } else {
              sendEmpty();
            }
          },
          error: () => sendEmpty()
        });
    });
  }

  /**
   * 资源详情右上角的操作执行后，用最新的数据重新打开一次modal
   * @param params
   * @param self
   * @param funName 打开资源详情函数名，默认为getResourceDetail
   */
  openDetailModalAfterQueryData(
    params: {
      autoPolling: boolean;
      records: any[];
    },
    self,
    funName = 'getResourceDetail'
  ) {
    const { autoPolling, records } = params;
    if (
      autoPolling ||
      !includes(map(self.drawModalService.modals, 'key'), 'detail-modal')
    ) {
      return;
    }
    const target = find(records, { uuid: self.currentDetailUuid });
    if (target) {
      self[funName](target);
    }
  }
}
