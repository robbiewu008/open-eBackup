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
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  ClientManagerApiService,
  CommonConsts,
  compareVersion,
  DataMap,
  PolicyAction,
  ProjectedObjectApiService,
  SlaApiService
} from 'app/shared';
import {
  ApplicationType,
  FCVmInNormalStatus,
  Features,
  HCSHostInNormalStatus,
  Scene
} from 'app/shared/consts';
import {
  BaseUtilService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import {
  assign,
  each,
  find,
  first as _first,
  get,
  includes,
  isArray,
  isEmpty,
  keys,
  now,
  reject,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-take-manual-backup',
  templateUrl: './take-manual-backup.component.html',
  styleUrls: ['./take-manual-backup.component.less']
})
export class TakeManualBackupComponent implements OnInit {
  isBatched = false;
  params;
  items = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  policyAction = PolicyAction;
  disableDiffAction = false;
  disableIncrementAction = false;
  disableLogAction = false;
  concurrentNumber = 10;
  actionDisabledMap = new Map([
    [PolicyAction.LOG, 'disableLogAction'],
    [PolicyAction.INCREMENT, 'disableIncrementAction'],
    [PolicyAction.DIFFERENCE, 'disableDiffAction']
  ]);
  tipMap = {
    [PolicyAction.FULL]: this.i18n.get('common_full_backup_tips_label'),
    [PolicyAction.LOG]: this.i18n.get('common_log_backup_tips_label'),
    [PolicyAction.INCREMENT]: this.i18n.get(
      'common_incremental_backup_tips_label'
    ),
    [PolicyAction.PERMANENT]: this.i18n.get(
      'common_permanent_backup_tips_label'
    ),
    [PolicyAction.DIFFERENCE]: this.i18n.get('common_diff_backup_tips_label')
  };
  name =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      ? this.i18n.get('common_name_label')
      : this.i18n.get('protection_copy_name_label');
  basicActionConfig = [
    {
      id: PolicyAction.FULL,
      label: this.i18n.get('common_full_backup_label')
    },
    {
      id: PolicyAction.INCREMENT,
      label: this.i18n.get('common_incremental_backup_label')
    }
  ];
  specialActionConfig = [
    {
      id: PolicyAction.FULL,
      label: this.i18n.get('common_full_backup_label')
    },
    {
      id: PolicyAction.INCREMENT,
      label: this.i18n.get('common_permanent_backup_label')
    }
  ];
  // 文件集批量手动备份页面的选项
  batchActionConfig = [
    {
      id: PolicyAction.FULL,
      label: this.i18n.get('common_full_backup_label')
    },
    {
      id: PolicyAction.PERMANENT,
      label: this.i18n.get('common_permanent_or_increment_backup_label'),
      tips: this.i18n.get('common_permanent_or_increment_backup_tips_label')
    }
  ];
  specialActions = {
    [DataMap.Resource_Type.hostSystem.value]: this.specialActionConfig,
    [DataMap.Resource_Type.clusterComputeResource.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.virtualMachine.value]: this.specialActionConfig,
    [DataMap.Resource_Type.KubernetesNamespace.value]: this.specialActionConfig,
    [DataMap.Resource_Type.KubernetesStatefulset.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.kubernetesNamespaceCommon.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.kubernetesDatasetCommon.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.HCSProject.value]: this.specialActionConfig,
    [DataMap.Resource_Type.HCSCloudHost.value]: this.specialActionConfig,
    [DataMap.Resource_Type.HDFSFileset.value]: this.specialActionConfig,
    [DataMap.Resource_Type.HBase.value]: this.specialActionConfig,
    [DataMap.Resource_Type.HiveBackupSet.value]: this.specialActionConfig,
    [DataMap.Resource_Type.ElasticsearchBackupSet.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.FusionCompute.value]: this.specialActionConfig,
    [DataMap.Resource_Type.fusionOne.value]: this.specialActionConfig,
    [DataMap.Resource_Type.openStackProject.value]: this.specialActionConfig,
    [DataMap.Resource_Type.openStackCloudServer.value]: this
      .specialActionConfig,
    [DataMap.Resource_Type.cNwareVm.value]: this.specialActionConfig,
    [DataMap.Resource_Type.hyperVVm.value]: this.specialActionConfig,
    [DataMap.Resource_Type.nutanixVm.value]: this.specialActionConfig
  };
  actions = {
    ...this.specialActions,
    [DataMap.Resource_Type.volume.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.fileset.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.informixInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.informixClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.oracle.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.oracleCluster.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.oraclePDB.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.dbTwoDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.dbTwoTableSet.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      }
    ],
    [DataMap.Resource_Type.tdsqlInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_permanent_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.tdsqlDistributedInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.GaussDB_DWS.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      }
    ],
    [DataMap.Resource_Type.DWS_Database.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.DWS_Schema.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.DWS_Table.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.SQLServerClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.SQLServerInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.SQLServerGroup.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.SQLServerDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.NASShare.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.NASFileSystem.value]: [
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.ndmp.value]: this.basicActionConfig,
    [DataMap.Resource_Type.LocalFileSystem.value]: [
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
        ? {
            id: PolicyAction.SNAPSHOT,
            label: this.i18n.get('common_anti_detection_snapshot_label')
          }
        : {
            id: PolicyAction.INCREMENT,
            label: this.i18n.get('common_incremental_backup_label')
          }
    ],
    [DataMap.Resource_Type.LocalLun.value]: [
      {
        id: PolicyAction.SNAPSHOT,
        label: this.i18n.get('common_anti_detection_snapshot_label')
      }
    ],
    [DataMap.Resource_Type.HBaseBackupSet.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_permanent_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.gaussdbTSingle.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.GaussDB_T.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.MySQLClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.MySQLInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.AntDBInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.AntDBClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.PostgreSQLInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.PostgreSQLClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.KingBaseInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.KingBaseClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.gaussdbForOpengauss.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.lightCloudGaussdbInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.MySQLDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.Redis.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.ClickHouse.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.ClickHouseDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.ClickHouseTableset.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.Dameng_singleNode.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.Dameng_cluster.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      }
    ],
    [DataMap.Resource_Type.OpenGauss_instance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.OpenGauss_database.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.DWS_Database.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.generalDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.MongodbSingleInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.MongodbClusterInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.OceanBaseCluster.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.OceanBaseTenant.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.tidbCluster.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.tidbDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.tidbTable.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.ActiveDirectory.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      }
    ],
    [DataMap.Resource_Type.ExchangeDataBase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.ExchangeGroup.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.ExchangeSingle.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.ObjectSet.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.PERMANENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.goldendbInstance.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.saphanaDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ],
    [DataMap.Resource_Type.APSCloudServer.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.APSResourceSet.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.APSZone.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_permanent_backup_label')
      }
    ],
    [DataMap.Resource_Type.saponoracleDatabase.value]: [
      {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    ]
  };
  maxlen = 550;
  nameErrorTip = {
    invalidNameBegin: this.i18n.get('common_valid_name_begin_label'),
    invalidNameCombination: this.i18n.get(
      'common_valid_name_combination_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [550])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private slaApiService: SlaApiService,
    private baseUtilService: BaseUtilService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private batchOperateService: BatchOperateService,
    private messageService: MessageService,
    private clientManagerApiService: ClientManagerApiService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    if (this.params?.resourceGroupMembers) {
      this.nameErrorTip = {
        invalidNameBegin: this.i18n.get('common_valid_name_begin_label'),
        invalidNameCombination: this.i18n.get(
          'common_valid_name_combination_label'
        ),
        invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [400])
      };
      this.maxlen = 400;
    }
    this.name =
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
        ? this.i18n.get('common_name_label')
        : this.isBatched
        ? this.i18n.get('protection_copy_name_prefix_label')
        : this.i18n.get('protection_copy_name_label');
    this.getDisabledDiffAction();
    this.getDisabledIncrementalAction();
    this.getDisableLogAction();
    this.initForm();
    this.initItems();
    if (
      includes(
        [DataMap.Resource_Type.goldendbInstance.value],
        this.params.resource_type
      )
    ) {
      this.isSupportFunc();
    }
    this.initItemTips();
  }

  initForm() {
    let name = '';
    if (this.params?.resourceGroupMembers) {
      name = `${this.params.name}` + '_' + `${now()}`;
    } else {
      name = isArray(this.params) ? 'backup' : `backup_${now()}`;
    }

    this.formGroup = this.fb.group({
      copy_name: new FormControl(name, {
        validators: [
          this.validCopyName(),
          this.baseUtilService.VALID.maxLength(this.maxlen)
        ]
      }),
      action: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });
  }

  validCopyName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const value = control.value;
      const reg1 = CommonConsts.REGEX.nameBegin;
      if (!reg1.test(value)) {
        return { invalidNameBegin: { value: control.value } };
      }

      const reg2 = CommonConsts.REGEX.nameCombination;
      if (!reg2.test(value)) {
        return { invalidNameCombination: { value: control.value } };
      }

      return null;
    };
  }

  goldenDBLogFilter(): boolean {
    if (
      !includes(
        [DataMap.Resource_Type.goldendbInstance.value],
        this.params.resource_type
      )
    ) {
      return false;
    }
    if (isArray(this.params)) {
      return !isEmpty(
        find(
          this.params,
          param =>
            compareVersion(
              param.environment?.version?.replace(/V/gi, ''),
              '6.1.01'
            ) < 1
        )
      );
    }
    return (
      compareVersion(
        this.params?.environment?.version?.replace(/V/gi, ''),
        '6.1.01'
      ) < 1
    );
  }

  initItems() {
    if (
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.ObjectStorage
        ],
        this.params.resource_type
      )
    ) {
      if (
        !this.isBatched &&
        ((!!this.params?.protectedObject?.extParameters
          ?.small_file_aggregation &&
          includes([ApplicationType.Fileset], this.params.resource_type)) ||
          (this.params?.protectedObject?.extParameters
            ?.small_file_aggregation ===
            DataMap.Aggregation_Mode.enable.value &&
            includes([ApplicationType.NASShare], this.params.resource_type)) ||
          (includes(
            [ApplicationType.ObjectStorage],
            this.params.resource_type
          ) &&
            this.params?.protectedObject?.extParameters?.aggregateSwitch))
      ) {
        this.items = this.basicActionConfig;
      } else if (
        this.isBatched &&
        includes([ApplicationType.Fileset], this.params.resource_type)
      ) {
        this.items = this.batchActionConfig;
      } else {
        this.items = this.actions[this.params.resource_type];
      }
      this.formGroup.get('action').setValue(this.items[0]?.id);
      return;
    }

    if (
      includes([ApplicationType.GeneralDatabase], this.params.resource_type)
    ) {
      this.getGeneralDbSupportBackupType();
      this.formGroup.get('action').setValue(this.items[0]?.id);
      return;
    }

    this.items =
      this.actions[this.params.resource_type] || this.basicActionConfig;

    // openGauss数据库不是PanWeiDB的不支持日志备份
    if (
      this.params.resource_type ===
        DataMap.Resource_Type.OpenGauss_instance.value &&
      !this.params?.extendInfo?.clusterVersion.includes('PanWeiDB')
    ) {
      this.items = reject(this.items, item => item.id === PolicyAction.LOG);
    }

    // DB2 RHEL HA集群不支持增量、差异
    if (
      includes(
        [DataMap.Resource_Type.dbTwoDatabase.value],
        this.params.resource_type
      ) &&
      this.params?.extendInfo.clusterType === DataMap.dbTwoType.standby.value &&
      this.params?.extendInfo?.deployOperatingSystem === 'Red Hat'
    ) {
      this.items = reject(
        this.items,
        item =>
          item.id === PolicyAction.INCREMENT ||
          item.id === PolicyAction.DIFFERENCE
      );
    }

    // Mysql EAPP集群不支持日志备份
    if (
      includes(
        [DataMap.Resource_Type.MySQLClusterInstance.value],
        this.params.resource_type
      ) &&
      this.params?.environment?.extendInfo?.clusterType ===
        DataMap.Mysql_Cluster_Type.eapp.value
    ) {
      this.items = reject(this.items, item => item.id === PolicyAction.LOG);
    }

    // MongoDB单实例--类型为单节点副本集才支持日志备份
    if (
      includes(
        [DataMap.Resource_Type.MongodbSingleInstance.value],
        this.params.resource_type
      )
    ) {
      if (
        this.params.extendInfo.singleType !==
        DataMap.mongoDBSingleInstanceType.copySet.value
      ) {
        this.items = reject(this.items, item => item.id === PolicyAction.LOG);
      }
    }
    if (DataMap.Resource_Type.NASShare.value !== this.params.resource_type) {
      this.formGroup.get('action').setValue(this.items[0]?.id);
    }

    // oracle开启存储快照后不支持差异备份
    if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ],
        this.params.resource_type
      )
    ) {
      if (
        !!this.params?.protectedObject?.extParameters?.storage_snapshot_flag
      ) {
        this.items = reject(
          this.items,
          item => item.id === PolicyAction.DIFFERENCE
        );
      }
    }

    // DWS schema集手动备份，版本在8.2.1及以上时，增加增量备份选项
    if (
      includes(
        [DataMap.Resource_Type.DWS_Schema.value],
        this.params.resource_type
      ) &&
      compareVersion(this.params?.environment?.version, '8.2.1') !== -1
    ) {
      this.items.push({
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      });
    }

    // goldenDB 6.1.01及以下版本不支持日志备份
    if (this.goldenDBLogFilter()) {
      this.items = reject(this.items, item => item.id === PolicyAction.LOG);
    }
  }

  // 判断当前版本是否支持添加存储资源
  isSupportFunc() {
    let arr = [];
    if (this.isBatched) {
      arr = this.params
        .map(item => item?.path.split(','))
        .reduce((v, cur) => v.concat(cur), []);
    } else {
      arr = this.params.path.split(',');
    }

    const params = {
      hostUuidsAndIps: uniq(arr),
      applicationType: 'GoldenDB',
      scene: Scene.Backup,
      buttonNames: [Features.LogBackup]
    };
    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        if (!res?.LogBackup) {
          this.items = reject(this.items, item => item.id === PolicyAction.LOG);
        }
      });
  }

  initItemTips() {
    if (includes(keys(this.specialActions), this.params.resource_type)) {
      this.items.map(e => {
        return assign(e, {
          tips: this.tipMap[
            e.id === this.policyAction.INCREMENT
              ? this.policyAction.PERMANENT
              : e.id
          ]
        });
      });
    }
    this.items.map(e => {
      if (!e.tips) {
        return assign(e, {
          tips:
            includes(
              [DataMap.Resource_Type.ObjectSet.value],
              this.params.resource_type
            ) && e.id === PolicyAction.INCREMENT
              ? this.i18n.get('protection_object_sla_incremental_tip_label')
              : this.tipMap[e.id]
        });
      }
    });
  }

  getDisableActionByActionId(id) {
    // 检查 id 是否存在于 Map 中
    if (!this.actionDisabledMap.has(id)) {
      return false;
    }
    return this[this.actionDisabledMap.get(id)];
  }

  getDisabledDiffAction() {
    if (
      includes(
        [DataMap.Resource_Type.saphanaDatabase.value],
        this.params.resource_type
      )
    ) {
      this.disableDiffAction =
        get(this.params, 'environment.extendInfo.enableLogBackup', 'false') ===
        'false';
    }
  }

  getDisabledIncrementalAction() {
    if (
      includes(
        [DataMap.Resource_Type.saphanaDatabase.value],
        this.params.resource_type
      )
    ) {
      this.disableIncrementAction =
        get(this.params, 'environment.extendInfo.enableLogBackup', 'false') ===
        'false';
    }
  }

  getDisableLogAction() {
    if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ],
        this.params.resource_type
      )
    ) {
      this.disableLogAction =
        !isEmpty(this.params.policy_list) &&
        isEmpty(find(this.params.policy_list, { action: PolicyAction.LOG }));
    }

    // TiDB集群版本低于6.2的不支持日志备份
    if (
      this.params.resource_type === DataMap.Resource_Type.tidbCluster.value &&
      this.params?.version
    ) {
      const targetVersion = '6.2.0';
      const originVersion = this.params.version.replace('v', '');
      this.disableLogAction =
        compareVersion(originVersion, targetVersion) === -1;
    }

    if (
      includes(
        [DataMap.Resource_Type.saphanaDatabase.value],
        this.params.resource_type
      )
    ) {
      this.disableLogAction =
        get(this.params, 'environment.extendInfo.enableLogBackup', 'false') ===
        'false';
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      if (!this.isBatched) {
        if (
          this.params.subType === ApplicationType.HCSCloudHost &&
          HCSHostInNormalStatus.includes(this.params.status)
        ) {
          this.messageService.error(
            this.i18n.get('protect_hcs_host_innormal_status_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'hcs_host_innormal_key'
            }
          );
          observer.error(
            this.i18n.get('protect_hcs_host_innormal_status_label')
          );
          return;
        }
        if (
          this.params.subType === ApplicationType.FusionCompute &&
          FCVmInNormalStatus.includes(this.params.status)
        ) {
          this.messageService.error(
            this.i18n.get('protect_fc_vm_innormal_status_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'fc_vm_innormal_key'
            }
          );
          observer.error(this.i18n.get('protect_fc_vm_innormal_status_label'));
          return;
        }
        const body = {
          ...this.formGroup.value,
          sla_id: this.params.sla_id
        };
        if (this.params?.resourceGroupMembers) {
          assign(body, { is_resource_group: true });
        }
        const params = {
          resourceId: this.params.resource_id,
          body
        };
        if (
          this.params.resource_type ===
          DataMap.Resource_Type.ActiveDirectory.value
        ) {
          this.warningMessageService.create({
            content: this.i18n.get('protection_ad_manual_backup_tip_label'),
            onOK: () => {
              this.dealBackup(observer, params);
            },
            onCancel: () => {
              observer.error(null);
              observer.complete();
            }
          });
        } else {
          this.dealBackup(observer, params);
        }
      } else {
        if (
          this.params.resource_type ===
          DataMap.Resource_Type.ActiveDirectory.value
        ) {
          this.warningMessageService.create({
            content: this.i18n.get('protection_ad_manual_backup_tip_label'),
            onOK: () => {
              this.batchOperateService.selfGetResults(
                item => {
                  return this.asyncAction(item);
                },
                this.params,
                () => {
                  observer.next();
                  observer.complete();
                },
                '',
                false,
                this.concurrentNumber
              );
            },
            onCancel: () => {
              observer.error(null);
              observer.complete();
            }
          });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.asyncAction(item);
            },
            this.params,
            () => {
              observer.next();
              observer.complete();
            },
            '',
            false,
            this.concurrentNumber
          );
        }
      }
    });
  }

  private asyncAction(item): Observable<any> {
    return new Observable<void>((observer: Observer<void>) => {
      const timeStamp = new Date().getTime();
      const body = {
        sla_id: item.sla_id,
        ...this.formGroup.value
      };
      if (!isEmpty(this.formGroup.value.copy_name)) {
        body.copy_name = `${this.formGroup.value.copy_name}_${timeStamp}`;
      }

      // 聚合文件集备份类型为增量备份，非聚合为永久增量备份
      if (
        includes([ApplicationType.Fileset], this.params.resource_type) &&
        item?.protectedObject?.extParameters?.small_file_aggregation &&
        body.action === PolicyAction.PERMANENT
      ) {
        body.action = PolicyAction.INCREMENT;
      }
      const params = {
        resourceId: item.resource_id,
        body,
        akLoading: false,
        akOperationTips: false,
        akDoException: false
      };
      this.dealBackup(observer, params);
    });
  }

  dealBackup(observer, params) {
    this.projectedObjectApiService
      .manualBackupV1ProtectedObjectsResourceIdActionBackupPost(params)
      .subscribe({
        next: () => {
          observer.next();
          observer.complete();
        },
        error: err => {
          observer.error(err);
          observer.complete();
        }
      });
  }

  private getGeneralDbSupportBackupType() {
    const resource = isArray(this.params) ? _first(this.params) : this.params;
    const config = JSON.parse(get(resource, 'extendInfo.scriptConf') || '{}');
    const version = get(resource, 'version');
    const supportBackupType = config?.backup?.support;
    const actionList = {
      full: {
        id: PolicyAction.FULL,
        label: this.i18n.get('common_full_backup_label')
      },
      cumulative_increment: {
        id: PolicyAction.DIFFERENCE,
        label: this.i18n.get('common_diff_backup_label')
      },
      difference_increment: {
        id: PolicyAction.INCREMENT,
        label: this.i18n.get('common_incremental_backup_label')
      },
      log: {
        id: PolicyAction.LOG,
        label: this.i18n.get('common_log_backup_label')
      }
    };

    each(supportBackupType, item => {
      if (
        (!item.minVersion && !item.maxVersion) ||
        (item.minVersion && !item.maxVersion && version >= item.minVersion) ||
        (!item.minVersion && item.maxVersion && version <= item.maxVersion) ||
        (item.minVersion &&
          item.maxVersion &&
          version <= item.maxVersion &&
          version >= item.minVersion)
      ) {
        this.items.push(actionList[item.backupType]);
      }
    });
  }
}
