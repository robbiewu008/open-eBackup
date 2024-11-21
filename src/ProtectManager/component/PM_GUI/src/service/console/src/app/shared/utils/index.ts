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
import {
  assign,
  cloneDeep,
  each,
  every,
  filter,
  find,
  forEach,
  get,
  includes,
  intersection,
  isEmpty,
  isFunction,
  isNil,
  isObject,
  isString,
  isUndefined,
  keys as _keys,
  size,
  some,
  split,
  trim
} from 'lodash';
import {
  CommonConsts,
  Page_Size_Options,
  Table_Size,
  ThemeEnum
} from '../consts/common.const';
import { DataMap } from '../consts/data-map.config';
import {
  OPERATE_PERMISSION,
  RoleOperationAuth,
  RoleType,
  RouterUrl,
  URL_PERMISSION,
  URL_PERMISSION_CLOUD_BACKUP,
  URL_PERMISSION_CYBER_ENGINE,
  URL_PERMISSION_HYPER_DETECT
} from '../consts/permission.const';
import { ApplicationType, RestoreFileType } from '../consts/protection.const';

export const hcsNoSupportApplication = [
  ApplicationType.LightCloudGaussDB,
  ApplicationType.KubernetesStatefulSet,
  ApplicationType.OpenStack,
  ApplicationType.LocalLun,
  ApplicationType.LocalFileSystem
];

export const hcsFilterUrl = [
  '/login',
  RouterUrl.Home,
  RouterUrl.Init,
  RouterUrl.ErrorPage
];

export const distributedFilterUrl = [
  RouterUrl.ProtectionDoradoFileSystem,
  RouterUrl.ExploreCopyDataFileSystem,
  RouterUrl.ExploreLiveMountFileSystem,
  RouterUrl.ExploreRansomwareDoradoFileSystem,
  RouterUrl.SftpService,
  RouterUrl.ExplorePolicyAirgap,
  RouterUrl.ExploreLiveMountFileset,
  RouterUrl.ExploreLiveMountNasShared,
  RouterUrl.SystemSamlSsoConfig,
  RouterUrl.ProtectionCommonShare
];

export const decoupleFilterUrl = [
  RouterUrl.SftpService,
  RouterUrl.ProtectionDoradoFileSystem,
  RouterUrl.ExploreCopyDataFileSystem,
  RouterUrl.ExploreLiveMountFileSystem,
  RouterUrl.ExploreRansomwareDoradoFileSystem,
  RouterUrl.ProtectionCommonShare
];

/**
 * 通过url权限表，获取当前角色查看的menu
 */
export function getAccessibleMenu(menu, cookie, i18n, absolute = true): any {
  // 若角色为RBAC自定义角色，则视为数据保护管理员，拥有与数据保护管理员相同的页面访问权限
  const role = cookie.role || RoleType.DataProtectionAdmin;
  let auth = URL_PERMISSION[role] || '';
  if (
    i18n.get('deploy_type') === DataMap.Deploy_Type.cloudbackup.value ||
    i18n.get('deploy_type') === DataMap.Deploy_Type.cloudbackup2.value
  ) {
    auth = URL_PERMISSION_CLOUD_BACKUP[cookie.role] || '';
  } else if (
    i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
  ) {
    auth = URL_PERMISSION_HYPER_DETECT[cookie.role] || '';
  } else if (
    i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
  ) {
    auth = URL_PERMISSION_CYBER_ENGINE[cookie.role] || '';
  }

  if (
    cookie.get('userType') === DataMap.loginUserType.saml.value &&
    cookie.role === 2
  ) {
    auth = filter(auth, item => {
      return !includes([RouterUrl.InsightPerformance], item);
    });
  }

  if (cookie.get('userType') === CommonConsts.HCS_USER_TYPE) {
    auth = filter(auth, item => {
      return !includes(hcsFilterUrl, item);
    });
  }

  if (i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value) {
    auth = filter(auth, item => {
      return !includes(distributedFilterUrl, item);
    });
  }

  if (
    includes(
      [
        DataMap.Deploy_Type.decouple.value,
        DataMap.Deploy_Type.openServer.value
      ],
      i18n.get('deploy_type')
    )
  ) {
    auth = filter(auth, item => {
      return !includes(decoupleFilterUrl, item);
    });
  }

  const filterMenu = item => {
    return filter(item, o => {
      if (o.items) {
        o.items = filterMenu(o.items);
        return o.items.length;
      }
      if (
        o.childrenLink &&
        !!size(intersection(o.childrenLink, auth)) &&
        o.routerLink
      ) {
        o.childrenLink = intersection(o.childrenLink, auth);
        o.routerLink = o.childrenLink[0];
      }
      if (o.routerLink.includes('/report-detail')) {
        return auth.includes('/report-detail');
      }
      if (o.routerLink.includes('/explore/modify-drill')) {
        return auth.includes('/explore/modify-drill');
      }
      if (o.routerLink.includes('/explore/drill-detail')) {
        return auth.includes('/explore/drill-detail');
      }
      if (o.routerLink.includes('/explore/drill-execute-log')) {
        return auth.includes('/explore/drill-execute-log');
      }
      return (
        (auth.indexOf(`${absolute ? '' : '/'}${o.routerLink}`) !== -1 ||
          !!size(intersection(o.childrenLink, auth))) &&
        !o.hidden
      );
    });
  };
  const rootLink = item => {
    if (item.items) {
      return rootLink(item.items[0]);
    }
    return item.routerLink;
  };
  const resetLink = items => {
    each(items, item => {
      if (item.routerLink) {
        item.routerLink = rootLink(item);
      }
    });
  };
  resetLink(filterMenu(menu));
  return filterMenu(menu);
}

export function getAccessibleUrl(url, cookie, i18n, absolute = false) {
  const menus = [{ routerLink: url }];
  return getAccessibleMenu(menus, cookie, i18n, absolute);
}

/**
 * 通过操作项权限表，获取当前角色查看的操作项
 * @param role 角色
 */
export function getAccessibleViewList(role) {
  let auth = OPERATE_PERMISSION[role] || [],
    map: any = {};
  if (
    getUserTypeByCookie() === CommonConsts.HCS_USER_TYPE ||
    getUserTypeByCookie() === CommonConsts.DME_USER_TYPE
  ) {
    auth = OPERATE_PERMISSION[RoleType.DataProtectionAdmin];
  }
  // rbac自定义角色
  if (((role && isNaN(Number(role))) || isNaN(role)) && isEmpty(auth)) {
    auth = OPERATE_PERMISSION[RoleType.DataProtectionAdmin];
  }
  auth.forEach(item => (map[item] = true));
  return map;
}

/**
 * 通过当前角色，过滤支持的操作项
 * @param role 角色
 */
export function getPermissionMenuItem(menus, role = null) {
  const authMap = getAccessibleViewList(isNil(role) ? _getRoleCookies() : role);
  const roleOperationAuth = RoleOperationAuth;
  const filterMenu = (source: any) => {
    forEach(source, item => {
      if (item?.hidden) {
        return;
      } // 已隐藏不处理

      // 叶子节点
      if (!item.items || !item.items.length) {
        item.hidden = !(
          isNil(item.permission) ||
          authMap[item.permission] ||
          includes(roleOperationAuth, item.permission)
        );
        return;
      }

      filterMenu(item.items);
      item.hidden = item.items.every(o => o.hidden);

      // 如果只有一个显示子项
      const showItems = filter(item.items, o => !o.hidden);
      if (showItems.length === 1) {
        assign(item, showItems[0]);
        delete item.items;
      }
    });
  };

  filterMenu(menus);

  const displayMenus = menus.filter(item => !item.hidden);

  // 如果最后非隐藏项 配置divide需要移除
  delete (displayMenus[displayMenus.length - 1] || ({} as any)).divide;
  return displayMenus;
}

/**
 * 获取角色对应枚举索引
 * @returns {String|void} 角色对应枚举索引
 */
function _getRoleCookies(): string | void {
  const cookies = document.cookie.split(';');

  // tslint:disable-next-line: prefer-for-of
  for (let i = 0; i < cookies.length; i++) {
    const cookiePair = cookies[i].split('=');
    if (cookiePair[0].trim() === 'role') {
      return decodeURIComponent(cookiePair[1]);
    }
  }

  return null;
}

/**
 * 获取当前userType
 * @param object 对象
 */
function getUserTypeByCookie() {
  const cookies = document.cookie.split(';');
  for (let i = 0; i < cookies.length; i++) {
    const cookiePair = cookies[i].split('=');
    if (cookiePair[0].trim() === 'userType') {
      return decodeURIComponent(cookiePair[1]);
    }
  }
  return '';
}

/**
 * 随机生成不相同的Modal Key
 *
 * @return  {[type]}  [return description]
 */
export function guid() {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    const r = (Math.random() * 16) | 0,
      v = c == 'x' ? r : (r & 0x3) | 0x8;
    return v.toString(16);
  });
}

/**
 * 比较2个对象属性值是否一致
 * @param object 对象
 */
export function deepEqualObject(object, newObject): boolean {
  const keys = _keys(object);
  const keysNew = _keys(newObject);
  if (size(keys) !== size(keysNew)) {
    return false;
  }

  for (let index = 0; index < size(keys); index++) {
    const val = object[keys[index]];
    const valNew = newObject[keys[index]];
    if (isUndefined(valNew)) {
      return false;
    }
    const bothObject = isObject(val) && isObject(valNew);
    if (
      (bothObject && !deepEqualObject(val, valNew)) ||
      (!bothObject &&
        trim(val) !== trim(valNew) &&
        Math.floor(new Date(val).getTime() / 1000) !==
          Math.floor(new Date(valNew).getTime() / 1000))
    ) {
      return false;
    }
  }
  return true;
}

export function getTableOptsItems(optItems, item, self) {
  const optsItems =
    optItems === self?.optItems ? cloneDeep(optItems) : optItems;
  each(optsItems, opt => {
    if (isFunction(opt.displayCheck)) {
      opt.hidden = !opt.displayCheck.call(self, [item]);
    } else {
      opt.hidden = false;
    }
    opt.disabledTips = '';
    if (isFunction(opt.onClick)) {
      const callback = opt.onClick;
      opt.onClick = event => {
        callback.call(self, [item], event);
      };
    }
    if (isFunction(opt.disableCheck)) {
      assign(opt, {
        disabled: opt.disableCheck.call(self, [item])
      });
    }
  });
  return optsItems;
}

// 复制到剪贴板
export function copyClipboard(value) {
  const el = document.createElement('textarea');
  el.value = value;
  el.setAttribute('readonly', '');
  el.style.position = 'absolute';
  el.style.left = '-10000px';
  document.body.appendChild(el);
  el.select();
  document.execCommand('copy');
  document.body.removeChild(el);
}

export function extendSlaInfo(resource) {
  assign(resource, {
    protection_status: resource?.protectionStatus
  });
  if (!resource || !resource.protectedObject) {
    return;
  }
  assign(resource, {
    protection_status:
      resource['protectedObject']['protectionStatus'] ||
      resource['protectionStatus'],
    sla_id: resource['protectedObject']['slaId'],
    sla_name: resource['protectedObject']['slaName'],
    sla_compliance: resource['protectedObject']['slaCompliance'],
    sla_status: resource['protectedObject']['status'] === 1
  });
}

export function extendDesesitizationInfo(resource) {
  if (!resource.resourceDesesitization) {
    assign(resource, {
      desesitization_status:
        DataMap.Desensitization_Status.not_desesitize.value,
      identification_status: DataMap.Identification_Status.not_identified.value
    });
    return;
  }
  assign(resource, {
    desesitization_status: resource.resourceDesesitization.desesitizationStatus,
    identification_status: resource.resourceDesesitization.identificationStatus,
    desesitization_policy_id:
      resource.resourceDesesitization.desesitizationPolicyId,
    desesitization_policy_name:
      resource.resourceDesesitization.desesitizationPolicyName
  });
}

export function filterBackupType(item, resourceType, i18n) {
  if (!!size(intersection([DataMap.Resource_Type.MySQL.value], resourceType))) {
    return item.value !== DataMap.CopyData_Backup_Type.permanent.value;
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.APSCloudServer.value,
          DataMap.Resource_Type.APSResourceSet.value,
          DataMap.Resource_Type.APSZone.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        resourceType
      )
    )
  ) {
    if (item.value === DataMap.CopyData_Backup_Type.incremental.value) {
      item.label = i18n.get('protection_incremental_forever_full_label');
    }
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.HDFSFileset.value,
          DataMap.Resource_Type.HiveBackupSet.value
        ],
        resourceType
      )
    )
  ) {
    if (item.value === DataMap.CopyData_Backup_Type.incremental.value) {
      item.label = i18n.get('protection_incremental_forever_full_label');
    }
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection([DataMap.Resource_Type.HBaseBackupSet.value], resourceType)
    )
  ) {
    if (item.value === DataMap.CopyData_Backup_Type.incremental.value) {
      item.label = i18n.get('protection_incremental_forever_full_label');
    }
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value,
        DataMap.CopyData_Backup_Type.log.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        resourceType
      )
    )
  ) {
    return includes([DataMap.CopyData_Backup_Type.permanent.value], item.value);
  } else if (
    !!size(intersection([DataMap.Resource_Type.NASShare.value], resourceType))
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.permanent.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.Redis.value,
          DataMap.Resource_Type.ActiveDirectory.value
        ],
        resourceType
      )
    )
  ) {
    return includes([DataMap.CopyData_Backup_Type.full.value], item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.permanent.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.fusionComputeVirtualMachine.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.permanent.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.PostgreSQLInstance.value,
          DataMap.Resource_Type.PostgreSQLClusterInstance.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.log.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.Dameng.value,
          DataMap.Resource_Type.Dameng_singleNode.value,
          DataMap.Resource_Type.GaussDB_T.value,
          DataMap.Resource_Type.gaussdbTSingle.value,
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.log.value,
        DataMap.CopyData_Backup_Type.diff.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.OpenGauss.value,
          DataMap.Resource_Type.OpenGauss_instance.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value,
        DataMap.CopyData_Backup_Type.log.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection([DataMap.Resource_Type.Dameng_cluster.value], resourceType)
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.diff.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [DataMap.Resource_Type.OpenGauss_database.value],
        resourceType
      )
    )
  ) {
    return includes([DataMap.CopyData_Backup_Type.full.value], item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.KingBaseInstance.value,
          DataMap.Resource_Type.KingBaseClusterInstance.value
        ],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value,
        DataMap.CopyData_Backup_Type.log.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [DataMap.Resource_Type.lightCloudGaussdbInstance.value],
        resourceType
      )
    )
  ) {
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.diff.value,
        DataMap.CopyData_Backup_Type.log.value
      ],
      item.value
    );
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.kubernetesNamespaceCommon.value,
          DataMap.Resource_Type.kubernetesDatasetCommon.value
        ],
        resourceType
      )
    )
  ) {
    if (item.value === DataMap.CopyData_Backup_Type.incremental.value) {
      item.label = i18n.get('protection_incremental_forever_full_label');
    }
    return includes(
      [
        DataMap.CopyData_Backup_Type.full.value,
        DataMap.CopyData_Backup_Type.incremental.value
      ],
      item.value
    );
  }

  if (
    !!size(
      intersection(
        [DataMap.Resource_Type.ElasticsearchBackupSet.value],
        resourceType
      )
    ) &&
    item.value === DataMap.CopyData_Backup_Type.incremental.value
  ) {
    item.label = i18n.get('protection_incremental_forever_full_label');
  }

  return item;
}

export function filterGeneratedBy(item, resourceType) {
  if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.liveMount.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.Imported.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection([DataMap.Resource_Type.virtualMachine.value], resourceType)
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.liveMount.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value,
      DataMap.CopyData_generatedType.Imported.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.liveMount.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(intersection([DataMap.Resource_Type.ImportCopy.value], resourceType))
  ) {
    return [
      DataMap.CopyData_generatedType.import.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.download.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.HDFSFileset.value,
          DataMap.Resource_Type.HBaseBackupSet.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.Imported.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.HiveBackupSet.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.ndmp.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.liveMount.value,
      DataMap.CopyData_generatedType.Imported.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(intersection([DataMap.Resource_Type.MySQL.value], resourceType))
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.liveMount.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.GaussDB_T.value,
          DataMap.Resource_Type.gaussdbTSingle.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  } else if (
    !!size(
      intersection(
        [
          DataMap.Resource_Type.Redis.value,
          DataMap.Resource_Type.Dameng.value,
          DataMap.Resource_Type.Dameng_cluster.value,
          DataMap.Resource_Type.Dameng_singleNode.value,
          DataMap.Resource_Type.OpenGauss.value,
          DataMap.Resource_Type.OpenGauss_database.value,
          DataMap.Resource_Type.OpenGauss_instance.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionComputeVirtualMachine.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.PostgreSQLInstance.value,
          DataMap.Resource_Type.PostgreSQLClusterInstance.value,
          DataMap.Resource_Type.KingBaseInstance.value,
          DataMap.Resource_Type.KingBaseClusterInstance.value,
          DataMap.Resource_Type.kubernetesNamespaceCommon.value,
          DataMap.Resource_Type.kubernetesDatasetCommon.value
        ],
        resourceType
      )
    )
  ) {
    return [
      DataMap.CopyData_generatedType.replicate.value,
      DataMap.CopyData_generatedType.backup.value,
      DataMap.CopyData_generatedType.tapeArchival.value,
      DataMap.CopyData_generatedType.cloudArchival.value,
      DataMap.CopyData_generatedType.cascadedReplication.value,
      DataMap.CopyData_generatedType.reverseReplication.value
    ].includes(item.value);
  }
  return item;
}

export function filterJobType(item, resourceType) {
  if (
    includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oraclePDB.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.live_restore_job.value,
        DataMap.Job_type.live_mount_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes([DataMap.Resource_Type.virtualMachine.value], resourceType)
  ) {
    return includes(
      [
        DataMap.Job_type.resource_scan.value,
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.live_restore_job.value,
        DataMap.Job_type.live_mount_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.migrate.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (resourceType === DataMap.Resource_Type.ImportCopy.value) {
    return includes(
      [
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.HDFSFileset.value,
        DataMap.Resource_Type.HBaseBackupSet.value,
        DataMap.Resource_Type.HiveBackupSet.value,
        DataMap.Resource_Type.ElasticsearchBackupSet.value,
        DataMap.Resource_Type.OpenGauss.value,
        DataMap.Resource_Type.OpenGauss_database.value,
        DataMap.Resource_Type.OpenGauss_instance.value,
        DataMap.Resource_Type.Dameng_singleNode.value,
        DataMap.Resource_Type.Dameng_cluster.value,
        DataMap.Resource_Type.Dameng.value,
        DataMap.Resource_Type.GaussDB_T.value,
        DataMap.Resource_Type.gaussdbTSingle.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.NASShare.value,
        DataMap.Resource_Type.ndmp.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.live_mount_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.unmout.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (includes([DataMap.Resource_Type.MySQL.value], resourceType)) {
    return includes(
      [
        DataMap.Job_type.resource_scan.value,
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.live_mount_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.GaussDB_T.value,
        DataMap.Resource_Type.gaussdbTSingle.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_scan.value,
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.archive_import_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.Redis.value,
        DataMap.Resource_Type.FusionComputeVM.value,
        DataMap.Resource_Type.PostgreSQLInstance.value,
        DataMap.Resource_Type.PostgreSQLClusterInstance.value,
        DataMap.Resource_Type.KingBaseInstance.value,
        DataMap.Resource_Type.KingBaseClusterInstance.value,
        DataMap.Resource_Type.OceanBaseCluster.value,
        DataMap.Resource_Type.OceanBaseTenant.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.HCSProject.value,
        DataMap.Resource_Type.openStackProject.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.openStackCloudServer.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.backup_job.value,
        DataMap.Job_type.restore_job.value,
        DataMap.Job_type.copy_data_job.value,
        DataMap.Job_type.archive_job.value,
        DataMap.Job_type.delete_copy_job.value,
        DataMap.Job_type.copies_verify_job.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (
    includes(
      [
        DataMap.Resource_Type.FusionComputeCNA.value,
        DataMap.Resource_Type.FusionComputeCluster.value
      ],
      resourceType
    )
  ) {
    return includes(
      [
        DataMap.Job_type.resource_protection.value,
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.copyExpired.value
      ],
      item.value
    );
  } else if (includes([DataMap.Resource_Type.vmGroup.value], resourceType)) {
    return includes(
      [
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.groupBackup.value
      ],
      item.value
    );
  } else if (includes([DataMap.Resource_Type.vmGroup.value], resourceType)) {
    return includes(
      [
        DataMap.Job_type.resource_protection_modify.value,
        DataMap.Job_type.groupBackup.value
      ],
      item.value
    );
  }

  return item;
}

export function isDatabaseApp(subType) {
  return includes(
    [
      DataMap.Resource_Type.oracle.value,
      DataMap.Resource_Type.SQLServerDatabase.value,
      DataMap.Resource_Type.SQLServerClusterInstance.value,
      DataMap.Resource_Type.SQLServerInstance.value,
      DataMap.Resource_Type.SQLServerGroup.value,
      DataMap.Resource_Type.MySQL.value,
      DataMap.Resource_Type.MySQLInstance.value,
      DataMap.Resource_Type.MySQLClusterInstance.value,
      DataMap.Resource_Type.MySQLDatabase.value,
      DataMap.Resource_Type.PostgreSQLInstance.value,
      DataMap.Resource_Type.PostgreSQLClusterInstance.value,
      DataMap.Resource_Type.OpenGauss_instance.value,
      DataMap.Resource_Type.OpenGauss_database.value,
      DataMap.Resource_Type.Dameng_singleNode.value,
      DataMap.Resource_Type.Dameng_cluster.value,
      DataMap.Resource_Type.GaussDB_T.value,
      DataMap.Resource_Type.gaussdbTSingle.value,
      DataMap.Resource_Type.DWS_Cluster.value,
      DataMap.Resource_Type.DWS_Database.value,
      DataMap.Resource_Type.DWS_Schema.value,
      DataMap.Resource_Type.DWS_Table.value,
      DataMap.Resource_Type.Redis.value,
      DataMap.Resource_Type.informixInstance.value,
      DataMap.Resource_Type.informixClusterInstance.value,
      DataMap.Resource_Type.tidbCluster.value,
      DataMap.Resource_Type.tidbDatabase.value,
      DataMap.Resource_Type.saphanaDatabase.value
    ],
    subType
  );
}

export function isHideOracleInstanceRestore(data): boolean {
  if (
    !includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      data.sub_type
    )
  ) {
    return false;
  }
  const isSnapshotCopy = get(
    data,
    'ext_parameters.storage_snapshot_flag',
    false
  );
  return (
    (isString(data.version) && data.version.substring(0, 2) === '11') ||
    data.environment_os_type === DataMap.Os_Type.windows.value ||
    isSnapshotCopy
  );
}

export function isHideOracleMount(data): boolean {
  if (
    !includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      data.sub_type
    )
  ) {
    return false;
  }
  const isSnapshotCopy = get(
    data,
    'ext_parameters.storage_snapshot_flag',
    false
  );
  return (
    data.environment_os_type === DataMap.Os_Type.windows.value || isSnapshotCopy
  );
}

export function isSlaResourceSubType(subType: string): boolean {
  return includes(
    [
      DataMap.Resource_Type.ActiveDirectory.value,
      DataMap.Resource_Type.AntDB.value,
      DataMap.Resource_Type.fileset.value,
      DataMap.Resource_Type.oracle.value,
      DataMap.Resource_Type.oracleCluster.value,
      DataMap.Resource_Type.dbTwoDatabase.value,
      DataMap.Resource_Type.dbTwoTableSet.value,
      DataMap.Resource_Type.ExchangeSingle.value,
      DataMap.Resource_Type.ExchangeGroup.value,
      DataMap.Resource_Type.ExchangeDataBase.value,
      DataMap.Resource_Type.ExchangeEmail.value,
      DataMap.Resource_Type.SQLServerInstance.value,
      DataMap.Resource_Type.SQLServerClusterInstance.value,
      DataMap.Resource_Type.SQLServerGroup.value,
      DataMap.Resource_Type.SQLServerDatabase.value,
      DataMap.Resource_Type.MySQLInstance.value,
      DataMap.Resource_Type.MySQLClusterInstance.value,
      DataMap.Resource_Type.MySQLDatabase.value,
      DataMap.Resource_Type.PostgreSQLInstance.value,
      DataMap.Resource_Type.PostgreSQLClusterInstance.value,
      DataMap.Resource_Type.KingBaseInstance.value,
      DataMap.Resource_Type.KingBaseClusterInstance.value,
      DataMap.Resource_Type.OpenGauss_instance.value,
      DataMap.Resource_Type.OpenGauss_database.value,
      DataMap.Resource_Type.Dameng_singleNode.value,
      DataMap.Resource_Type.Dameng_cluster.value,
      DataMap.Resource_Type.GaussDB_T.value,
      DataMap.Resource_Type.gaussdbTSingle.value,
      DataMap.Resource_Type.DWS_Cluster.value,
      DataMap.Resource_Type.DWS_Schema.value,
      DataMap.Resource_Type.DWS_Table.value,
      DataMap.Resource_Type.Redis.value,
      DataMap.Resource_Type.ClickHouse.value,
      DataMap.Resource_Type.MongodbSingleInstance.value,
      DataMap.Resource_Type.MongodbClusterInstance.value,
      DataMap.Resource_Type.goldendbInstance.value,
      DataMap.Resource_Type.generalDatabase.value,
      DataMap.Resource_Type.virtualMachine.value,
      DataMap.Resource_Type.hostSystem.value,
      DataMap.Resource_Type.clusterComputeResource.value,
      DataMap.Resource_Type.KubernetesNamespace.value,
      DataMap.Resource_Type.KubernetesStatefulset.value,
      DataMap.Resource_Type.FusionCompute.value,
      DataMap.Resource_Type.fusionOne.value,
      DataMap.Resource_Type.HDFSFileset.value,
      DataMap.Resource_Type.HBaseBackupSet.value,
      DataMap.Resource_Type.HiveBackupSet.value,
      DataMap.Resource_Type.ElasticsearchBackupSet.value,
      DataMap.Resource_Type.NASFileSystem.value,
      DataMap.Resource_Type.NASShare.value,
      DataMap.Resource_Type.openStackProject.value,
      DataMap.Resource_Type.openStackCloudServer.value,
      DataMap.Resource_Type.HCSProject.value,
      DataMap.Resource_Type.HCSCloudHost.value,
      DataMap.Resource_Type.informixInstance.value,
      DataMap.Resource_Type.informixClusterInstance.value,
      DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
      DataMap.Resource_Type.lightCloudGaussdbInstance.value,
      DataMap.Resource_Type.lightCloudGaussdbInstance.value,
      DataMap.Resource_Type.OceanBaseCluster.value,
      DataMap.Resource_Type.OceanBaseTenant.value,
      DataMap.Resource_Type.tidbCluster.value,
      DataMap.Resource_Type.tidbDatabase.value,
      DataMap.Resource_Type.tidbTable.value
    ],
    subType
  );
}

export function hiddenDwsFileLevelRestore(data) {
  if (
    !includes(
      [
        DataMap.Resource_Type.DWS_Cluster.value,
        DataMap.Resource_Type.DWS_Schema.value
      ],
      data.resource_sub_type
    )
  ) {
    return false;
  }

  const properties = JSON.parse(get(data, 'properties', '{}'));
  const version = get(properties, 'version');
  const backupType = get(data, 'backup_type');

  if (backupType === DataMap.CopyData_Backup_Type.full.value) {
    return compareVersion(version, '8.1.2') === -1;
  } else {
    return compareVersion(version, '8.1.3') === -1;
  }
}

export function getGeneralDatabaseConf(data, restoreType, dataMapService) {
  if (data?.resource_sub_type !== DataMap.Resource_Type.generalDatabase.value) {
    return false;
  }

  const resourceData = JSON.parse(data?.resource_properties || '{}');
  const supportRestore = get(
    JSON.parse(resourceData.extendInfo.scriptConf || '{}'),
    'restore.support'
  );
  const version = get(resourceData, 'version');

  const restoreConf = find(
    supportRestore,
    item =>
      item.restoreType === restoreType &&
      includes(
        item.includeBackupType,
        dataMapService.getLabel('generalDbBackupConfig', data?.backup_type)
      )
  );

  return restoreConf
    ? !(
        (!restoreConf.minVersion && !restoreConf.maxVersion) ||
        (restoreConf.minVersion &&
          !restoreConf.maxVersion &&
          compareVersion(version, restoreConf.minVersion) !== -1) ||
        (!restoreConf.minVersion &&
          restoreConf.maxVersion &&
          compareVersion(restoreConf.maxVersion, version) !== -1) ||
        (restoreConf.minVersion &&
          restoreConf.maxVersion &&
          compareVersion(version, restoreConf.minVersion) !== -1 &&
          compareVersion(restoreConf.maxVersion, version) !== -1)
      )
    : true;
}

// 仅支持纯数字版本号，例如 1.1.0
export function compareVersion(ver1, ver2) {
  const version1 = split(ver1, '.');
  const version2 = split(ver2, '.');
  const len = size(version1) > size(version2) ? size(version1) : size(version2);

  for (let i = 0; i < len; i++) {
    const num1 = get(version1, i, 0);
    const num2 = get(version2, i, 0);

    if (Number(num1) > Number(num2)) {
      return 1;
    } else if (Number(num1) < Number(num2)) {
      return -1;
    }
  }

  return 0;
}

export function defaultWindowTime() {
  return new Date(
    `${new Date().getFullYear()}/${new Date().getMonth() +
      1}/${new Date().getDate()} 00:00:00`
  );
}

export function hiddenHcsUserFileLevelRestore(data, isHcsUser) {
  if (
    includes(
      [
        DataMap.Resource_Type.FusionCompute.value,
        DataMap.Resource_Type.fusionOne.value,
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.openStackCloudServer.value,
        DataMap.Resource_Type.cNwareVm.value,
        DataMap.Resource_Type.nutanixVm.value
      ],
      data.resource_sub_type
    ) &&
    isHcsUser
  ) {
    return true;
  }
  return false;
}

export function hiddenOracleFileLevelRestore(
  rowCopy,
  resourceProperties,
  properties
) {
  if (
    !includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      resourceProperties.sub_type
    )
  ) {
    return false;
  }
  // 生成方式屏蔽
  if (
    includes(
      [
        DataMap.CopyData_generatedType.liveMount.value,
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.tapeArchival.value,
        DataMap.CopyData_generatedType.cloudArchival.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      rowCopy.generated_by
    )
  ) {
    return true;
  }

  const version = get(resourceProperties, 'version', '');
  const isSnapshotCopy = get(
    resourceProperties,
    'ext_parameters.storage_snapshot_flag',
    false
  );
  /*
   * 屏蔽规则
   * 1.存储快照副本和日志副本
   * 2.暂时不支持oracle12以下的
   * */
  return isSnapshotCopy || compareVersion(version, '12') === -1;
}

/**
 * 函数用于判断oracle能否原位置恢复
 * @param data 原始副本数据
 * @param resourceProperties 副本的resource_properties
 */
export function disableOracleRestoreOriginalLocation(data, resourceProperties) {
  if (
    /*
     * 1. 复制、级联复制不能原位置
     * 2. 源资源不存在
     * */
    !includes(
      [
        DataMap.Resource_Type.oracleCluster.value,
        DataMap.Resource_Type.oracle.value
      ],
      resourceProperties.sub_type
    )
  ) {
    return false;
  }
  return (
    includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      data.generated_by
    ) ||
    data.is_replicated ||
    data?.resource_status === DataMap.Resource_Status.notExist.value
  );
}

/**
 * 函数用于判断oracle能否新位置恢复
 * @param data 原始副本数据
 * @param properties 副本的properties
 * @param resourceProperties 副本的resource_properties
 */
export function disableOracleRestoreNewLocation(
  data,
  properties,
  resourceProperties
) {
  let osType = '';
  const { sub_type, environment_os_type } = resourceProperties;
  osType = environment_os_type || data?.osType;
  if (
    !includes(
      [
        DataMap.Resource_Type.oracleCluster.value,
        DataMap.Resource_Type.oracle.value
      ],
      resourceProperties.sub_type
    )
  ) {
    return false;
  }
  /*
   * 1. 如果properties中new_loc_restore为false，不能新位置恢复
   * 2. 复制副本只能新位置恢复，复制副本需要禁用新位置恢复时，直接将操作->更多->恢复置灰
   * */
  return (
    !!data.storage_snapshot_flag && !get(properties, 'new_loc_restore', true)
  );
}

export function autoTableScroll(virtualScroll, height, dataTable?, cdr?) {
  window.addEventListener('resize', () => {
    virtualScroll.getScrollParam(
      height,
      Page_Size_Options.Three,
      Table_Size.Default
    );
    if (cdr) {
      cdr.detectChanges();
    }
    if (dataTable) {
      dataTable.setTableScroll(virtualScroll.scrollParam);
    }
  });
}
export function getMultiHostOps(resource, isAgentList?) {
  return filter(resource, item => {
    const val = isAgentList ? item : item.environment;
    const connection = val?.extendInfo?.connection_result;
    const target = JSON.parse(connection || '{}');
    return some(
      target,
      item =>
        item.link_status ===
        Number(DataMap.resource_LinkStatus_Special.normal.value)
    );
  });
}

export function extendSummaryCopiesParams(item) {
  if (!item) {
    return;
  }
  assign(item, {
    resource_id: item.resourceId,
    resource_name: item.resourceName,
    resource_location: item.resourceLocation,
    resource_environment_ip: item.resourceEnvironmentIp,
    resource_environment_name: item.resourceEnvironmentName,
    resource_properties: item.resourceProperties,
    resource_sub_type: item.resourceSubType,
    resource_type: item.resourceType,
    resource_status: item.resourceStatus,
    sla_name: item.slaName
  });
}

// 无效副本禁用删除以外按钮（除openStack、flex、fc、hcs、阿里云无效副本可以强制恢复）
export function disableValidCopyBtn(data, properties) {
  return (
    data.status === DataMap.copydata_validStatus.invalid.value &&
    properties?.isMemberDeleted === 'true' &&
    !includes(
      [
        DataMap.Resource_Type.FusionCompute.value,
        DataMap.Resource_Type.fusionOne.value,
        DataMap.Resource_Type.KubernetesStatefulset.value,
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.openStackCloudServer.value,
        DataMap.Resource_Type.APSCloudServer.value,
        DataMap.Resource_Type.cNwareVm.value,
        DataMap.Resource_Type.nutanixVm.value
      ],
      data.resource_sub_type
    )
  );
}

// openstack副本里没有系统盘不能做新建虚拟机恢复
export function disableOpenstackVmRestore(data, properties) {
  return (
    data.resource_sub_type ===
      DataMap.Resource_Type.openStackCloudServer.value &&
    !isEmpty(properties?.volList) &&
    isEmpty(
      find(
        properties?.volList,
        vol =>
          vol.bootable === 'true' &&
          vol.attachments &&
          includes(vol.attachments[0]?.device, 'da')
      )
    )
  );
}

// 非集中式和pacific底座的以外的存储接入的云主机不支持保护备份
export function enableAllFunc(diskInfo) {
  const enableType = ['FusionStorage', 'OceanStor', 'Dorado'];
  return every(diskInfo, item =>
    some(enableType, v => item.storageType.indexOf(v) !== -1)
  );
}

// 屏蔽部分应用类型
export function filterApplication(item, appUtilsService) {
  let applications = get(item, 'applications', []);
  if (appUtilsService.isDistributed || appUtilsService.isDecouple) {
    applications = applications.filter(
      item => item.appValue !== 'NasFileSystem'
    );
  }
  // 服务化屏蔽应用
  if (appUtilsService?.isHcsUser) {
    applications = applications.filter(item => {
      const apps = item.appValue.split(',');
      return (
        !includes(
          apps,
          DataMap.Resource_Type.lightCloudGaussdbInstance.value
        ) &&
        !includes(apps, DataMap.Resource_Type.openStackCloudServer.value) &&
        !includes(apps, DataMap.Resource_Type.kubernetesClusterCommon.value)
      );
    });
  }

  return applications;
}

export function isJson(str: string): boolean {
  if (typeof str == 'string') {
    try {
      var obj = JSON.parse(str);
      if (typeof obj == 'object' && obj) {
        return true;
      } else {
        return false;
      }
    } catch (e) {
      return false;
    }
  }
  return false;
}

// 侦测报告非文件系统应用类型
export function isNotFileSystem(appType): boolean {
  return includes([0x02, 0x03, 0x07, 0x08, 0x09, 0x10, 0x12], appType);
}

// 获取资源的标签数据
export function getLabelList(item) {
  let hoverList = [];
  hoverList = item?.labelList?.map(v => {
    return {
      ...v,
      label: v.name
    };
  });
  let showList = hoverList;
  if (size(hoverList) > 2) {
    const num = hoverList.length - 2;
    showList = cloneDeep(hoverList).slice(0, 2);
    showList.push({ uuid: '-1', label: `+${num}` });
  }
  return { showList, hoverList };
}

// 增加标签参数
export function extendParams(conditions, params) {
  if (!!params) {
    assign(conditions, params);
  }
  return conditions;
}

export function getFileIcon(item) {
  if (item.type === RestoreFileType.Directory) {
    return 'aui-icon-directory';
  }
  if (item.type === RestoreFileType.Link) {
    return 'aui-icon-link';
  }
  return 'aui-icon-file';
}

// 构造浏览节点
export function extendNodeParams(node, item) {
  item.name = item.path;
  item.nodeName = item.path;
  item.label = item.path;
  item.path =
    node.path === '/'
      ? `${node.path}${node.nodeName}`
      : `${node.path}/${node.nodeName}`;
  item.absolutePath =
    item.path === '/'
      ? `${item.path}${item.nodeName}`
      : `${item.path}/${item.nodeName}`;
  item.isLeaf = item.type !== RestoreFileType.Directory;
  item.children = item.type === RestoreFileType.Directory ? [] : null;
  item.contentToggleIcon = getFileIcon(item);
  return item;
}

// sqlserver日志副本/差异副本的手动归档入口要屏蔽
export function hideSqlserverArchive(data) {
  return (
    includes(
      [
        DataMap.Resource_Type.SQLServerDatabase.value,
        DataMap.Resource_Type.SQLServerClusterInstance.value,
        DataMap.Resource_Type.SQLServerInstance.value,
        DataMap.Resource_Type.SQLServerGroup.value
      ],
      data?.resource_sub_type
    ) &&
    includes([DataMap.CopyData_Backup_Type.log.value], data?.source_copy_type)
  );
}

// sqlserver日志副本保留策略设置入口要屏蔽
export function hideSqlserverRetention(data) {
  return (
    includes(
      [
        DataMap.Resource_Type.SQLServerDatabase.value,
        DataMap.Resource_Type.SQLServerClusterInstance.value,
        DataMap.Resource_Type.SQLServerInstance.value,
        DataMap.Resource_Type.SQLServerGroup.value
      ],
      data?.resource_sub_type
    ) &&
    includes([DataMap.CopyData_Backup_Type.log.value], data?.source_copy_type)
  );
}

// 获取深浅模式
export function getAppTheme(i18n): any {
  if (
    includes(
      [
        DataMap.Deploy_Type.cloudbackup.value,
        DataMap.Deploy_Type.cloudbackup2.value,
        DataMap.Deploy_Type.hyperdetect.value,
        DataMap.Deploy_Type.cyberengine.value
      ],
      i18n.get('deploy_type')
    )
  ) {
    return ThemeEnum.light;
  }
  const storageTheme = localStorage.getItem('app_theme');
  if (storageTheme === ThemeEnum.auto) {
    const prefers =
      window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)');
    return prefers?.matches ? ThemeEnum.dark : ThemeEnum.light;
  }
  return storageTheme || ThemeEnum.light;
}
