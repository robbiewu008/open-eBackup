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
import { CatalogName } from './common.const';
import { DataMap } from './data-map.config';

export const RESOURCE_CATALOGS = [
  {
    catalog_name: CatalogName.HostApps,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.ABBackupClient.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.AntDB.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.fileset.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.volume.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.oracle.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.DB2.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.SQLServer.value,
        show: false
      },
      {
        catalog_name: DataMap.Resource_Type.MySQL.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.PostgreSQL.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.KingBase.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.Redis.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.OpenGauss.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.Dameng.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.KingBase.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.OceanBaseCluster.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.GBase.value,
        show: false
      },
      {
        catalog_name: DataMap.Resource_Type.MongoDB.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.GaussDB_T.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.GaussDB_DWS.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.SQLServer.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.ClickHouse.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.generalDatabase.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.goldendb.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.informixService.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.tdsqlInstance.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.tidb.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.GBase.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.ExchangeDataBase.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.Virtualization,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.virtualMachine.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.hyperVVm.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.H3cCas.value,
        show: false
      },
      {
        catalog_name: DataMap.Resource_Type.KubernetesStatefulset.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.kubernetesClusterCommon.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.FusionCompute.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.cNwareVm.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.fusionOne.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.nutanixVm.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.Copies,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.Replica.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.BigData,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.HDFSFileset.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.HBaseBackupSet.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.HiveBackupSet.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.ElasticsearchBackupSet.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.Storage,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.NasEquipment.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.NASFileSystem.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.NASShare.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.LocalFileSystem.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.LocalLun.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.commonShare.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.ndmp.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.Cloud,
    show: true,
    children: [
      {
        catalog_name: DataMap.Resource_Type.HCSCloudHost.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.openStackCloudServer.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.gaussdbForOpengauss.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.ApsaraStack.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.ObjectSet.value,
        show: true
      }
    ]
  },
  {
    catalogName: CatalogName.Application,
    show: true,
    children: [
      {
        catalogName: DataMap.Resource_Type.ActiveDirectory.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.saphanaDatabase.value,
        show: true
      },
      {
        catalogName: DataMap.Resource_Type.Saponoracle.value,
        show: true
      }
    ]
  },
  {
    catalog_name: CatalogName.Vessel,
    show: false,
    children: [
      {
        catalog_name: DataMap.Resource_Type.KubernetesCommon.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.KubernetesMySQL.value,
        show: true
      },
      {
        catalog_name: DataMap.Resource_Type.RuleManagement.value,
        show: true
      }
    ]
  }
];
