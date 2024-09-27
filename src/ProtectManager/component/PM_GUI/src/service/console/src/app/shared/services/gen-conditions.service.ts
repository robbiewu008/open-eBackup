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
import { eq, isNil } from 'lodash';
import { DataMap, ResourceType } from '../consts';

type getConditionsMethod = (
  typeKey: string,
  version: CONDITION_VERSION
) => IConditions;
type conditionsItem = Array<string | string[]> | string;
interface IConditions {
  subType?: conditionsItem;
  isTopInstance?: conditionsItem;
  type?: conditionsItem;
  resourceType?: conditionsItem;
  visible?: string;
  agId?: any;
}

interface IGenConditions {
  getConditions: getConditionsMethod;
}

enum CONDITION_VERSION {
  V1 = 'v1',
  V2 = 'v2'
}

/**
 * 生成相应类型资源的接口查询conditions
 */
@Injectable({
  providedIn: 'root'
})
export class GenConditionsService implements IGenConditions {
  // 方法映射Map
  private methodsMap: Map<string, getConditionsMethod> = new Map();

  constructor() {
    this.initMap();
  }

  /**
   * 获取 conditions 查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions  查询信息
   */
  public getConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    const method = this.methodsMap.get(typeKey);
    if (isNil(method)) {
      return {
        subType: [typeKey]
      };
    }
    return method(typeKey, version);
  }

  /**
   * 获取 Dameng 单节点查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getDamengSingleNodeConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [['in'], typeKey],
        isTopInstance: [['=='], '1']
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Dameng 集群查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions  查询信息
   */
  private getDamengClusterConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [['in'], typeKey],
        isTopInstance: [['=='], '1']
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Clickhouse 查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getClickHouseConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        type: [['in'], ResourceType.DATABASE, ResourceType.TABLE_SET]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Redis 查询信息
   * 获取 Clickhouse 集群查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getClickHouseClusterConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        type: [['in'], ResourceType.CLUSTER]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Clickhouse 数据库查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getClickHouseDatabaseConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        type: [['in'], ResourceType.DATABASE]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Clickhouse 数据库查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getClickHouseTablesetConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        type: [['in'], ResourceType.TABLE_SET]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 Clickhouse 查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getRedisConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        resourceType: [['=='], 'cluster']
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 PostgreSQL 查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getPostgreSQLInstanceConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.PostgreSQLInstance.value],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 PostgreSQL 查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getPostgreSQLClusterInstanceConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.PostgreSQLClusterInstance.value],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 KingBase 查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getKingBaseInstanceConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [
          DataMap.Resource_Type.KingBaseInstance.value,
          DataMap.Resource_Type.KingBaseClusterInstance.value
        ],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 MySQL 实例查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getMySQLInstanceConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  private getSQLServerDatabaseConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        agId: [['=='], '']
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 MySQL 实例查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getMySQLClusterInstanceConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [typeKey],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 FC虚拟化平台查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getFusionComputePlatformConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.FusionCompute.value],
        type: [ResourceType.PLATFORM]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 FC 虚拟机查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getFusionComputeVMConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.FusionCompute.value],
        type: [ResourceType.VM]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 FC主机查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getFusionComputeHostConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.FusionCompute.value],
        type: [ResourceType.HOST]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 FC集群查询信息
   * @param  _ - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getFusionComputeClusterConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Resource_Type.FusionCompute.value],
        type: [ResourceType.CLUSTER]
      };
    }
    // v1 预留
    return {};
  }

  private getHCSPlatformConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [DataMap.Job_Target_Type.HCSContainer.value],
        type: [ResourceType.HCS]
      };
    }
    // v1 预留
    return {};
  }

  private getHCSTenantConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        type: [ResourceType.TENANT],
        visible: '1'
      };
    }
    // v1 预留
    return {};
  }

  private getHCSProjectConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        type: [ResourceType.PROJECT]
      };
    }
    // v1 预留
    return {};
  }

  private getHCSCloudHostConditions(
    _: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        type: [ResourceType.CLOUD_HOST]
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 获取 informix 实例查询信息
   * @param  typeKey - 资源类型标识
   * @param  version - condition 版本(默认v2)
   * @return IConditions 查询信息
   */
  private getInformixInstanceConditions(
    typeKey: string,
    version: CONDITION_VERSION = CONDITION_VERSION.V2
  ): IConditions {
    if (eq(version, CONDITION_VERSION.V2)) {
      return {
        subType: [
          DataMap.Resource_Type.informixInstance.value,
          DataMap.Resource_Type.informixClusterInstance.value
        ],
        isTopInstance: '1'
      };
    }
    // v1 预留
    return {};
  }

  /**
   * 初始化方法映射Map
   */
  private initMap() {
    // informix 集群实例
    this.methodsMap.set(
      DataMap.Resource_Type.informixClusterInstance.value,
      this.getInformixInstanceConditions
    );
    // informix 单实例
    this.methodsMap.set(
      DataMap.Resource_Type.informixInstance.value,
      this.getInformixInstanceConditions
    );
    // Dameng 单节点
    this.methodsMap.set(
      DataMap.Resource_Type.Dameng_singleNode.value,
      this.getDamengSingleNodeConditions
    );
    // Dameng 集群
    this.methodsMap.set(
      DataMap.Resource_Type.Dameng_cluster.value,
      this.getDamengClusterConditions
    );
    // ClickHouse
    this.methodsMap.set(
      DataMap.Resource_Type.ClickHouse.value,
      this.getClickHouseConditions
    );
    // ClickHouseCluster
    this.methodsMap.set(
      DataMap.Resource_Type.ClickHouseCluster.value,
      this.getClickHouseClusterConditions
    );
    // ClickHouseDatabase
    this.methodsMap.set(
      DataMap.Resource_Type.ClickHouseDatabase.value,
      this.getClickHouseDatabaseConditions
    );
    // ClickHouseTableset
    this.methodsMap.set(
      DataMap.Resource_Type.ClickHouseTableset.value,
      this.getClickHouseTablesetConditions
    );
    // Redis
    this.methodsMap.set(
      DataMap.Resource_Type.Redis.value,
      this.getRedisConditions
    );
    // PostgreSQLInstance
    this.methodsMap.set(
      DataMap.Resource_Type.PostgreSQLInstance.value,
      this.getPostgreSQLInstanceConditions
    );
    // PostgreSQLClusterInstance
    this.methodsMap.set(
      DataMap.Resource_Type.PostgreSQLClusterInstance.value,
      this.getPostgreSQLClusterInstanceConditions
    );
    // KingBaseInstance
    this.methodsMap.set(
      DataMap.Resource_Type.KingBaseInstance.value,
      this.getKingBaseInstanceConditions
    );
    // KingBaseClusterInstance
    this.methodsMap.set(
      DataMap.Resource_Type.KingBaseClusterInstance.value,
      this.getKingBaseInstanceConditions
    );
    // MySQLClusterInstance
    this.methodsMap.set(
      DataMap.Resource_Type.MySQLClusterInstance.value,
      this.getMySQLClusterInstanceConditions
    );
    // MySQLInstance
    this.methodsMap.set(
      DataMap.Resource_Type.MySQLInstance.value,
      this.getMySQLInstanceConditions
    );
    // SQLServerDatabase
    this.methodsMap.set(
      DataMap.Resource_Type.SQLServerDatabase.value,
      this.getSQLServerDatabaseConditions
    );
    // FusionComputePlatform
    this.methodsMap.set(
      DataMap.Resource_Type.FusionComputePlatform.value,
      this.getFusionComputePlatformConditions
    );
    this.methodsMap.set(
      DataMap.Job_Target_Type.FusionComputePlatform.value,
      this.getFusionComputePlatformConditions
    );

    // FusionComputeVM
    this.methodsMap.set(
      DataMap.Resource_Type.FusionComputeVM.value,
      this.getFusionComputeVMConditions
    );
    this.methodsMap.set(
      DataMap.Job_Target_Type.FusionCompute.value,
      this.getFusionComputeVMConditions
    );

    // FusionComputeCNA
    this.methodsMap.set(
      DataMap.Resource_Type.FusionComputeCNA.value,
      this.getFusionComputeHostConditions
    );
    this.methodsMap.set(
      DataMap.Job_Target_Type.FusionComputeHost.value,
      this.getFusionComputeHostConditions
    );
    this.methodsMap.set(
      'FusionComputeHost',
      this.getFusionComputeHostConditions
    );

    // FusionComputeCluster
    this.methodsMap.set(
      DataMap.Resource_Type.FusionComputeCluster.value,
      this.getFusionComputeClusterConditions
    );
    this.methodsMap.set(
      DataMap.Job_Target_Type.FusionComputeCluster.value,
      this.getFusionComputeClusterConditions
    );
    this.methodsMap.set(
      'FusionComputeCluster',
      this.getFusionComputeClusterConditions
    );
  }
}
