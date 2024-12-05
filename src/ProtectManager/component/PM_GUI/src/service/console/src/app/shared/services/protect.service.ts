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
import { Router } from '@angular/router';
import { MessageboxService, MessageService } from '@iux/live';
import { LearningConfigComponent } from 'app/business/explore/ransomware-protection/data-backup/file-system/learning-config/learning-config.component';
import { WarnModalComponent } from 'app/business/explore/ransomware-protection/real-time-detection/file-system/warn-modal/warn-modal.component';
import { AdvancedParameterComponent as ActiveDirectoryAdvancedComponent } from 'app/business/protection/application/active-directory/advanced-parameter/advanced-parameter.component';
import { SelectBackupSetListComponent } from 'app/business/protection/big-data/hbase/backup-set/select-backup-set-list/select-backup-set-list.component';
import { AdvancedParameterComponent as HDFSAdvancedParameterComponent } from 'app/business/protection/big-data/hdfs/filesets/advanced-parameter/advanced-parameter.component';
import { SelectFilesetsListComponent } from 'app/business/protection/big-data/hdfs/filesets/select-filesets-list/select-filesets-list.component';
import { ApsAdvanceParameterComponent } from 'app/business/protection/cloud/apsara-stack/aps-advance-parameter/aps-advance-parameter.component';
import { ApsProtectSelectComponent } from 'app/business/protection/cloud/apsara-stack/aps-protect-select/aps-protect-select.component';
import { SelectDatabaseListComponent as HCSSelectDatabaseListComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/select-database-list/select-database-list.component';
import { CloudStackAdvancedParameterComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/stack-advanced-parameter/cloud-stack-advanced-parameter.component';
import { AdvancedComponent } from 'app/business/protection/cloud/openstack/openstack-list/advanced/advanced.component';
import { SelectPoComponent } from 'app/business/protection/cloud/openstack/openstack-list/select-po/select-po.component';
import { AdvancedParameterComponent as MysqlAdvancedParameterComponent } from 'app/business/protection/host-app//mysql/instance-database/advanced-parameter/advanced-parameter.component';
import { SelectDatabaseComponent as SelectClickhouseComponent } from 'app/business/protection/host-app/click-house/database/select-database/select-database.component';
import { SelectProtectRowComponent as SelectClickhouseTablesetComponent } from 'app/business/protection/host-app/click-house/tabel-set/select-protect-row/select-protect-row.component';
import { AdvancedExchangeComponent } from 'app/business/protection/host-app/exchange/database/advanced-exchange/advanced-exchange.component';
import { AdvancedEmailComponent } from 'app/business/protection/host-app/exchange/email/advanced-email/advanced-email.component';
import { AdvancedParameterComponent as FilesetAdvancedParameterComponent } from 'app/business/protection/host-app/fileset/advanced-parameter/advanced-parameter.component';
import { SelectFilesetListComponent as SelectHostFilesetsListComponent } from 'app/business/protection/host-app/fileset/select-fileset-list/select-fileset-list.component';
import { SelectInstanceDatabaseComponent as SelectDWSListComponent } from 'app/business/protection/host-app/gaussdb-dws/instance-database/select-instance-database/select-instance-database.component';
import { SelectGaussdbTListComponent } from 'app/business/protection/host-app/gaussdb-t/select-guassdb-t-list/select-guassdb-t-list.component';
import { ProtectionAdvanceComponent } from 'app/business/protection/host-app/mongodb/protection-advance/protection-advance.component';
import { SelectInstanceDatabaseComponent } from 'app/business/protection/host-app/mysql/instance-database/select-instance-database/select-instance-database.component';
import { AdvancedParameterComponent as OracleAdvancedParameterComponent } from 'app/business/protection/host-app/oracle/database-list/advanced-parameter/advanced-parameter.component';
import { SelectDatabaseListComponent as OracleSelectDatabaseListComponent } from 'app/business/protection/host-app/oracle/database-list/select-database-list/select-database-list.component';
import { SelectInstanceDatabaseComponent as SelectSQLServerListComponent } from 'app/business/protection/host-app/sql-server/select-instance-database/select-instance-database.component';
import { AdvancedParameterComponent as TDSQLAdvancedParameterComponent } from 'app/business/protection/host-app/tdsql/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterComponent as TDSQLDistributedAdvancedParameterComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/advanced-parameter/advanced-parameter.component';
import { AdvancedParameterComponent } from 'app/business/protection/host-app/tidb/advanced-parameter/advanced-parameter.component';
import { VolumeAdvancedParameterComponent } from 'app/business/protection/host-app/volume/volume-advanced-parameter/volume-advanced-parameter.component';
import { SelectDoradoListComponent } from 'app/business/protection/storage/dorado-file-system/select-dorado-list/select-dorado-list.component';
import { AdvancedParameterComponent as NasSharedAdvancedParameterComponent } from 'app/business/protection/storage/nas-shared/advanced-parameter/advanced-parameter.component';
import { SelectNasSharedListComponent } from 'app/business/protection/storage/nas-shared/select-nas-shared-list/select-nas-shared-list.component';
import { ObjectAdvancedParameterComponent } from 'app/business/protection/storage/object/object-service/object-advanced-parameter/object-advanced-parameter.component';
import { FusionAdvancedParameterComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-advanced-parameter/fusion-advanced-parameter.component';
import { SelectDatabaseListComponent as FusionComputeSelectDatabaseListComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/select-database-list/select-database-list.component';
import { AdvancedParamComponent } from 'app/business/protection/virtualization/kubernetes-container/advanced-param/advanced-param.component';
import { SelectDatabaseListComponent as SelectStatefulSetComponent } from 'app/business/protection/virtualization/kubernetes/base-template/select-database-list/select-database-list.component';
import { ProtectionAdvancedComponent } from 'app/business/protection/virtualization/virtualization-base/protection-advanced/protection-advanced.component';
import { ProtectionObjectComponent } from 'app/business/protection/virtualization/virtualization-base/protection-object/protection-object.component';
import { AdvancedComponent as VMwareAdvancedComponent } from 'app/business/protection/virtualization/vmware/vm/advanced/advanced.component';
import { SelectObjectsComponent as VMSelectObjectsComponent } from 'app/business/protection/virtualization/vmware/vm/select-objects/select-objects.component';
import {
  IODETECTFILESYSTEMService,
  MODAL_COMMON,
  ProjectedObjectApiService,
  ProtectedCopyObjectApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory
} from 'app/shared';
import { MySQLBackupComponent } from 'app/shared/components/mysql-backup.component';
import {
  GlobalService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
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
  isString,
  isUndefined,
  last,
  map,
  omit,
  pick,
  set,
  size,
  toString,
  toUpper,
  values
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { CreateProtectModalComponent } from '../components/create-protect-modal/create-protect-modal.component';
import { SelectSlaComponent } from '../components/protect/select-sla/select-sla.component';
import { ReplicaAdvancedParameterComponent } from '../components/replica-advanced-parameter/replica-advanced-parameter.component';
import {
  ApplicationType,
  FCVmInNormalStatus,
  HCSHostInNormalStatus,
  MESSAGE_BOX_ACTION,
  PolicyAction,
  PolicyType,
  ResourceOperationType,
  ResourceType,
  SearchResource
} from '../consts';
import { PROTECTION_CONFIG } from '../consts/protect-config';
import { DataMap } from './../consts/data-map.config';
import { BackupMessageService } from './backup-message.service';
import { BatchOperateService } from './batch-operate.service';
import { SystemTimeService } from './system-time.service';

export interface ModalOption {
  data: any;
  width?: number;
  header?: string;
  originData?: any;
  onOK?: () => void;
  beforeOpen?: () => void;
  afterClose?: () => void;
  restoreWidth?: (data: any) => any;
}

@Injectable({
  providedIn: 'root'
})
export class ProtectService {
  mysqlBackupComponent = MySQLBackupComponent;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private globalService: GlobalService,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private systemTimeService: SystemTimeService,
    private batchOperateService: BatchOperateService,
    private backupMessageService: BackupMessageService,
    private warningMessageService: WarningMessageService,
    private detectFilesystemService: IODETECTFILESYSTEMService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedCopyObjectApiService: ProtectedCopyObjectApiService
  ) {}

  emitSearchStore() {
    if (this.router.url === '/search') {
      this.globalService.emitStore({
        action: SearchResource.Refresh,
        state: ''
      });
    }
  }

  openProtectModal(
    type,
    action: ProtectResourceAction,
    option: ModalOption,
    unitType?
  ) {
    const protectionConfig: any = cloneDeep(PROTECTION_CONFIG[type].steps);
    if (!isEmpty(unitType)) {
      switch (unitType) {
        case DataMap.Resource_Type.virtualMachine.value:
          protectionConfig.push(
            last(PROTECTION_CONFIG[ProtectResourceCategory.vmware].steps)
          );
          break;
        case DataMap.Resource_Type.FusionCompute.value:
          protectionConfig.push(
            last(
              PROTECTION_CONFIG[
                DataMap.Resource_Type.fusionComputeVirtualMachine.value
              ].steps
            )
          );
          break;
        case DataMap.Resource_Type.openStackCloudServer.value:
          protectionConfig.push(
            last(
              PROTECTION_CONFIG[
                DataMap.Resource_Type.openStackCloudServer.value
              ].steps
            )
          );
          break;
        case DataMap.Resource_Type.HCSCloudHost.value:
          protectionConfig.push(
            last(PROTECTION_CONFIG[DataMap.Resource_Type.CloudHost.value].steps)
          );
          break;
      }
    }
    this.openWindow(type, action, option, protectionConfig);
  }

  openWindow(type, action, option, protectionConfig) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'resource-protect-modal',
      lvType: 'drawer',
      lvHeader:
        option.header || action === ProtectResourceAction.Create
          ? this.i18n.get('common_protect_label')
          : this.i18n.get('common_resource_protection_modify_label'),
      lvWidth: MODAL_COMMON.largeModal,
      lvContent: CreateProtectModalComponent,
      lvComponentParams: {
        type,
        option: assign(option, {
          data: cloneDeep(option.data),
          originData: cloneDeep(option.data)
        }),
        protectionConfig,
        action
      },
      lvOkDisabled: action === ProtectResourceAction.Create,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent();
        const modalIns = modal.getInstance();
        const arr$ = {};
        let advancedParamsComponent;
        each(content.componentArr, (component, key) => {
          if (
            [ResourceType.PROJECT, ResourceType.HOST].includes(
              component.resourceSubType
            )
          ) {
            return;
          }
          if (component.valid$) {
            arr$[key] = component.valid$;
          }
          if (
            component instanceof FilesetAdvancedParameterComponent ||
            component instanceof NasSharedAdvancedParameterComponent ||
            component instanceof ObjectAdvancedParameterComponent ||
            component instanceof OracleAdvancedParameterComponent
          ) {
            advancedParamsComponent = component;
          }
        });
        const combined: any = combineLatest(values(arr$));
        combined.subscribe(res => {
          modalIns.lvOkDisabled = !!size(
            filter(res, item => {
              return !item;
            })
          );
          let params = {};
          each(content.componentArr, component => {
            if (isFunction(component.onOK)) {
              assign(params, component.onOK());
            }
          });
          if (
            includes(
              [ProtectResourceCategory.oracles, ProtectResourceCategory.oracle],
              type
            )
          ) {
            if (
              find(
                get(params, 'slaObject.policy_list'),
                (item: any) => item.action === PolicyAction.DIFFERENCE
              ) ||
              (find(
                get(params, 'slaObject.policy_list'),
                (item: any) => item.action === PolicyType.REPLICATION
              ) &&
                params['subType'] === DataMap.Resource_Type.oracleCluster.value)
            ) {
              // oracle不能同时使用差异备份和存储快照备份
              if (
                advancedParamsComponent.formGroup.get('storage_snapshot_flag')
                  .value
              ) {
                advancedParamsComponent.formGroup
                  .get('storage_snapshot_flag')
                  .setValue(false);
              }
              advancedParamsComponent.unsupportStorage = true;
            } else {
              advancedParamsComponent.unsupportStorage = false;
            }
          }
          if (
            includes(
              [
                ProtectResourceCategory.fileset,
                ProtectResourceCategory.NASShare,
                ProtectResourceCategory.filesets,
                DataMap.Resource_Type.ObjectSet.value
              ],
              type
            )
          ) {
            // 对象集合如果自动索引则不能开启多节点并行备份
            if (
              find(
                get(params, 'slaObject.policy_list'),
                (item: any) => !!item?.ext_parameters?.auto_index
              ) &&
              includes([DataMap.Resource_Type.ObjectSet.value], type)
            ) {
              if (
                !!advancedParamsComponent.formGroup.value.multiNodeBackupSwitch
              ) {
                advancedParamsComponent.formGroup
                  .get('multiNodeBackupSwitch')
                  .setValue(false);
              }
              advancedParamsComponent.isAutoIndex = true;
            } else {
              advancedParamsComponent.isAutoIndex = false;
            }

            if (
              find(
                get(params, 'slaObject.policy_list'),
                (item: any) => item.action === PolicyAction.PERMANENT
              )
            ) {
              if (!!advancedParamsComponent.formGroup.value.smallFile) {
                advancedParamsComponent.formGroup
                  .get('smallFile')
                  .setValue(false);
              }
              advancedParamsComponent.disableSmallFile = true;
            } else if (
              find(
                get(params, 'slaObject.policy_list'),
                (item: any) => item.action === PolicyAction.INCREMENT
              )
            ) {
              if (!advancedParamsComponent.formGroup.value.smallFile) {
                advancedParamsComponent.formGroup
                  .get('smallFile')
                  .setValue(true);
              }
              advancedParamsComponent.disableSmallFile = true;
            } else {
              advancedParamsComponent.disableSmallFile = false;
            }
            modalIns.lvOkDisabled = !!size(
              filter(res, item => {
                return !item;
              })
            );
          }
        });
        // 高级参数为非必填项，需手打触发订阅
        each(content.componentArr, component => {
          if (
            component instanceof AdvancedEmailComponent ||
            component instanceof AdvancedExchangeComponent ||
            component instanceof ActiveDirectoryAdvancedComponent ||
            component instanceof OracleAdvancedParameterComponent ||
            component instanceof FilesetAdvancedParameterComponent ||
            component instanceof SelectHostFilesetsListComponent ||
            component instanceof OracleSelectDatabaseListComponent ||
            component instanceof VMSelectObjectsComponent ||
            component instanceof VMwareAdvancedComponent ||
            component instanceof SelectNasSharedListComponent ||
            (component instanceof NasSharedAdvancedParameterComponent &&
              type === ProtectResourceCategory.NASShare) ||
            component instanceof SelectGaussdbTListComponent ||
            component instanceof SelectInstanceDatabaseComponent ||
            component instanceof MysqlAdvancedParameterComponent ||
            component instanceof SelectDoradoListComponent ||
            component instanceof SelectFilesetsListComponent ||
            component instanceof SelectBackupSetListComponent ||
            component instanceof HDFSAdvancedParameterComponent ||
            component instanceof SelectSQLServerListComponent ||
            component instanceof SelectStatefulSetComponent ||
            component instanceof FusionAdvancedParameterComponent ||
            component instanceof SelectClickhouseComponent ||
            component instanceof SelectClickhouseTablesetComponent ||
            component instanceof CloudStackAdvancedParameterComponent ||
            component instanceof SelectPoComponent ||
            component instanceof AdvancedComponent ||
            component instanceof AdvancedParameterComponent ||
            component instanceof AdvancedParamComponent ||
            component instanceof LearningConfigComponent ||
            component instanceof ProtectionAdvanceComponent ||
            component instanceof VolumeAdvancedParameterComponent ||
            component instanceof ObjectAdvancedParameterComponent ||
            component instanceof ProtectionObjectComponent ||
            component instanceof ProtectionAdvancedComponent ||
            component instanceof ApsAdvanceParameterComponent ||
            component instanceof TDSQLAdvancedParameterComponent ||
            component instanceof TDSQLDistributedAdvancedParameterComponent ||
            component instanceof ReplicaAdvancedParameterComponent
          ) {
            if (!isUndefined(component.valid$)) {
              component.valid$.next(true);
            }
          }

          if (component instanceof SelectDWSListComponent) {
            if (!isUndefined(component.valid$)) {
              component.valid$.next(action === ProtectResourceAction.Modify);
            }
          }

          if (
            component instanceof FusionComputeSelectDatabaseListComponent &&
            [
              DataMap.Resource_Type.FusionComputeVM.value,
              DataMap.Resource_Type.FusionComputeCluster.value
            ].includes(type)
          ) {
            if (!isUndefined(component.valid$)) {
              component.valid$.next(true);
            }
          }

          if (
            component instanceof HCSSelectDatabaseListComponent &&
            [
              DataMap.Resource_Type.CloudHost.value,
              DataMap.Resource_Type.Project.value
            ].includes(type)
          ) {
            if (isFunction(component.setValid)) {
              component.setValid();
            }
          }

          if (
            component instanceof ApsProtectSelectComponent &&
            [
              DataMap.Resource_Type.APSCloudServer.value,
              DataMap.Resource_Type.APSResourceSet.value,
              DataMap.Resource_Type.APSZone.value
            ].includes(type)
          ) {
            if (!isUndefined(component.valid$)) {
              component.valid$.next(true);
            }
          }
          // 修改时sla需要手动触发
          if (
            action === ProtectResourceAction.Modify &&
            component instanceof SelectSlaComponent
          ) {
            if (!isUndefined(component.valid$)) {
              component.valid$.next(true);
            }
            component.getOriginalSlaInfo();
          }
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent();
          let params = {};
          each(content.componentArr, component => {
            if (isFunction(component.onOK)) {
              assign(params, component.onOK());
            }
          });
          const protectedObject = get(params, 'protectedObject');
          // 特传三个esn和target字段
          if (protectedObject) {
            const extInfo = pick(get(protectedObject, 'extParameters'), [
              'last_backup_esn',
              'priority_backup_esn',
              'first_backup_esn',
              'first_backup_target',
              'priority_backup_target',
              'last_backup_target'
            ]);
            const extraParams: object = assign(
              {},
              get(params, 'ext_parameters'),
              extInfo
            );
            assign(params, {
              ext_parameters: { ...extraParams }
            });
          }
          this.systemTimeService.getSystemTime().subscribe(
            sysTime => {
              const slaObject = params['slaObject'] || {};
              const startTimeArr = [];
              if (isArray(slaObject.policy_list)) {
                each(slaObject.policy_list, item => {
                  if (
                    item &&
                    item.type === PolicyType.BACKUP &&
                    includes(
                      [
                        PolicyAction.FULL,
                        PolicyAction.DIFFERENCE,
                        PolicyAction.INCREMENT,
                        PolicyAction.PERMANENT
                      ],
                      item.action
                    ) &&
                    item.schedule &&
                    item.schedule.start_time
                  ) {
                    startTimeArr.push(Date.parse(item.schedule.start_time));
                  }
                });
              }
              const startTime: number =
                startTimeArr.length > 0
                  ? Math.min.apply(null, startTimeArr)
                  : undefined;
              const systemTime: number = sysTime
                ? Date.parse(sysTime.time.replace(/-/g, '/'))
                : undefined;
              if (
                startTime < systemTime &&
                action === ProtectResourceAction.Create &&
                (params['linkStatus'] ===
                  DataMap.resource_LinkStatus_Special.normal.value ||
                  !includes(
                    [
                      DataMap.Resource_Type.PostgreSQLInstance.value,
                      DataMap.Resource_Type.PostgreSQLClusterInstance.value,
                      DataMap.Resource_Type.tidbCluster.value,
                      DataMap.Resource_Type.tidbDatabase.value,
                      DataMap.Resource_Type.tidbTable.value,
                      DataMap.Resource_Type.tdsqlInstance.value,
                      DataMap.Resource_Type.tdsqlDistributedInstance.value,
                      DataMap.Resource_Type.ExchangeSingle.value,
                      DataMap.Resource_Type.ExchangeGroup.value
                    ],
                    params['subType']
                  ))
              ) {
                if (
                  type === ApplicationType.MySQL &&
                  !(
                    size(slaObject.policy_list) === 1 &&
                    get(first(slaObject.policy_list), 'action') ===
                      PolicyAction.LOG
                  )
                ) {
                  this.drawModalService.create({
                    ...MODAL_COMMON.generateDrawerOptions(),
                    lvModalKey: 'backupMessage',
                    ...{
                      lvType: 'dialog',
                      lvDialogIcon: 'lv-icon-popup-danger-48',
                      lvHeader: this.i18n.get('common_alarms_info_label'),
                      lvContent: this.mysqlBackupComponent,
                      lvWidth: MODAL_COMMON.normalWidth,
                      lvComponentParams: {
                        askManualBackup: true,
                        manualBackup: false
                      },
                      lvOkType: 'primary',
                      lvCancelType: 'default',
                      lvOkDisabled: true,
                      lvFocusButtonId: 'cancel',
                      lvCloseButtonDisplay: true,
                      lvAfterOpen: modal => {
                        const content = modal.getContentComponent() as MySQLBackupComponent;
                        content.valid$.subscribe(res => {
                          modal.lvOkDisabled = !res;
                        });
                      },
                      lvCancel: modal => resolve(false),
                      lvOk: modal => {
                        const component = modal.getContentComponent() as MySQLBackupComponent;
                        if (component.status) {
                          params = {
                            ...params,
                            post_action: toUpper(PolicyType.BACKUP)
                          };
                        }
                        this.onOK(type, action, params, option).subscribe({
                          next: () => {
                            resolve(true);
                            this.drawModalService.destroyModal(
                              'resource-protect-modal'
                            );
                            this.emitSearchStore();
                            if (
                              !isUndefined(option.onOK) &&
                              isFunction(option.onOK)
                            ) {
                              option.onOK();
                            }
                          },
                          error: err => resolve(false)
                        });
                      },
                      lvAfterClose: resolve(false)
                    }
                  });
                } else {
                  this.backupMessageService.create({
                    content: this.i18n.get('protection_protect_late_tip_label'),
                    onOK: modal => {
                      const component = modal.getContentComponent();
                      if (component.status) {
                        params = {
                          ...params,
                          post_action: toUpper(PolicyType.BACKUP)
                        };
                      }
                      this.onOK(type, action, params, option).subscribe({
                        next: () => {
                          resolve(true);
                          this.drawModalService.destroyModal(
                            'resource-protect-modal'
                          );
                          this.emitSearchStore();
                          if (
                            !isUndefined(option.onOK) &&
                            isFunction(option.onOK)
                          ) {
                            option.onOK();
                          }
                        },
                        error: err => resolve(false)
                      });
                    },
                    onCancel: () => resolve(false),
                    lvAfterClose: () => resolve(false)
                  });
                }
              } else {
                if (
                  type === ApplicationType.MySQL &&
                  !(
                    size(slaObject.policy_list) === 1 &&
                    get(first(slaObject.policy_list), 'action') ===
                      PolicyAction.LOG
                  )
                ) {
                  this.drawModalService.create({
                    ...MODAL_COMMON.generateDrawerOptions(),
                    lvModalKey: 'backupMessage',
                    ...{
                      lvType: 'dialog',
                      lvDialogIcon: 'lv-icon-popup-danger-48',
                      lvHeader: this.i18n.get('common_alarms_info_label'),
                      lvContent: this.mysqlBackupComponent,
                      lvWidth: MODAL_COMMON.normalWidth,
                      lvComponentParams: {
                        askManualBackup: false,
                        manualBackup: false
                      },
                      lvOkType: 'primary',
                      lvCancelType: 'default',
                      lvOkDisabled: true,
                      lvFocusButtonId: 'cancel',
                      lvCloseButtonDisplay: true,
                      lvAfterOpen: modal => {
                        const content = modal.getContentComponent() as MySQLBackupComponent;
                        content.valid$.subscribe(res => {
                          modal.lvOkDisabled = !res;
                        });
                      },
                      lvCancel: modal => resolve(false),
                      lvOk: modal => {
                        this.onOK(type, action, params, option).subscribe({
                          next: () => {
                            resolve(true);
                            this.drawModalService.destroyModal(
                              'resource-protect-modal'
                            );
                            this.emitSearchStore();
                            if (
                              !isUndefined(option.onOK) &&
                              isFunction(option.onOK)
                            ) {
                              option.onOK();
                            }
                          },
                          error: err => resolve(false)
                        });
                      },
                      lvAfterClose: resolve(false)
                    }
                  });
                } else {
                  this.onOK(type, action, params, option).subscribe({
                    next: () => {
                      resolve(true);
                      this.emitSearchStore();
                      if (
                        !isUndefined(option.onOK) &&
                        isFunction(option.onOK)
                      ) {
                        option.onOK();
                      }
                    },
                    error: err => resolve(false)
                  });
                }
              }
            },
            err => resolve(false)
          );
        });
      },
      restoreWidth: params => {
        if (!isFunction(option.restoreWidth)) {
          return;
        }
        option.restoreWidth(params.option.data);
      }
    });
  }

  onOK(
    type: ProtectResourceCategory,
    action: ProtectResourceAction,
    params: any,
    option: ModalOption
  ): Observable<void> {
    switch (action) {
      case ProtectResourceAction.Create:
        return this.createProtect(type, params, option);
      case ProtectResourceAction.Modify:
        return this.modifyProtect(type, params, option);
      default:
        break;
    }
  }

  asyncBatchCreateProtect(bodys, type?): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.batchOperateService.selfGetResults(
        item => {
          return this.isCyberEngine
            ? this.projectedObjectApiService.batchCreateV1ProtectedObjectsCyberBatchPost(
                {
                  body: item.body,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              )
            : type && type === DataMap.Resource_Type.vmGroup.value
            ? this.protectedResourceApiService.CreateResourceGroupProtectedObject(
                {
                  CreateResourceGroupProtectedObjectRequestBody: <any>(
                    omit(item.body, 'name')
                  ),
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              )
            : this.projectedObjectApiService.batchCreateV1ProtectedObjectsBatchPost(
                {
                  body: item.body,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
        },
        map(cloneDeep(bodys), item => {
          return {
            body: item,
            name: item.name,
            isAsyn: type === DataMap.Resource_Type.vmGroup.value ? false : true
          };
        }),
        () => {
          observer.next();
          observer.complete();
        },
        '',
        true
      );
    });
  }

  asyncBatchModifyProtect(bodys): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.batchOperateService.selfGetResults(
        item => {
          return this.isCyberEngine
            ? this.projectedObjectApiService.modifyV1ProtectedObjectsCyberPut({
                body: item.body,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              })
            : this.projectedObjectApiService.modifyV1ProtectedObjectsPut({
                body: item.body,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
        },
        cloneDeep(bodys),
        () => {
          observer.next();
          observer.complete();
        },
        '',
        true
      );
    });
  }

  createProtect(
    type: string | number,
    params: any,
    option: ModalOption
  ): Observable<void> {
    switch (type) {
      case ProtectResourceCategory.fileset:
      case ProtectResourceCategory.filesets:
      case ProtectResourceCategory.NASShare:
      case ProtectResourceCategory.NASFileSystem:
      case ProtectResourceCategory.HDFS:
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.ndmp.value:
      case DataMap.Resource_Type.LocalFileSystem.value:
      case DataMap.Resource_Type.LocalLun.value:
      case ProtectResourceCategory.HBase:
        return this.protectFilesets(params, option);
      case ProtectResourceCategory.GaussDBDWS:
      case ProtectResourceCategory.oracle:
      case ProtectResourceCategory.mysql:
      case ProtectResourceCategory.db2:
      case ProtectResourceCategory.SQLServer:
      case ProtectResourceCategory.GaussdbForOpengauss:
      case ProtectResourceCategory.oracles:
      case ProtectResourceCategory.mysqls:
      case ProtectResourceCategory.OpenGauss:
      case ProtectResourceCategory.db2s:
      case ProtectResourceCategory.SQLServers:
      case ProtectResourceCategory.GaussDBs:
      case ProtectResourceCategory.ClickHouse:
      case ProtectResourceCategory.ClickHouseDatabase:
      case ProtectResourceCategory.ClickHouseTableset:
      case DataMap.Resource_Type.GaussDB_T.value:
      case DataMap.Resource_Type.MySQL.value:
      case DataMap.Resource_Type.generalDatabase.value:
      case DataMap.Resource_Type.KingBase.value:
      case DataMap.Resource_Type.PostgreSQLInstance.value:
      case DataMap.Resource_Type.PostgreSQL.value:
      case DataMap.Resource_Type.Redis.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.SQLServerDatabase.value:
      case ProtectResourceCategory.Dameng:
      case DataMap.Resource_Type.MongoDB.value:
      case ProtectResourceCategory.GeneralDB:
      case DataMap.Resource_Type.OceanBaseCluster.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
      case DataMap.Resource_Type.tidbCluster.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.ActiveDirectory.value:
      case DataMap.Resource_Type.ObjectSet.value:
      case DataMap.Resource_Type.Exchange.value:
      case DataMap.Resource_Type.ExchangeDataBase.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
      case DataMap.Resource_Type.tdsqlInstance.value:
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        return this.protectDatabases(params, option);
      case DataMap.Resource_Type.CloudHost.value:
      case DataMap.Resource_Type.Project.value:
      case DataMap.Resource_Type.openStackProject.value:
      case DataMap.Resource_Type.openStackCloudServer.value:
      case DataMap.Resource_Type.APSCloudServer.value:
      case DataMap.Resource_Type.APSResourceSet.value:
      case DataMap.Resource_Type.APSZone.value:
        return this.protectCloudAndVirtual(type, params, option);
      case DataMap.Resource_Type.fusionComputeVirtualMachine.value:
      case DataMap.Resource_Type.fusionComputeCNA.value:
      case DataMap.Resource_Type.fusionComputeCluster.value:
        return this.protectFC(params, option);
      case DataMap.Resource_Type.KubernetesStatefulset.value:
      case DataMap.Resource_Type.KubernetesNamespace.value:
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
      case DataMap.Resource_Type.kubernetesDatasetCommon.value:
        return this.protectKubernents(params, option);
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.hyperVHost.value:
      case DataMap.Resource_Type.hyperVVm.value:
      case DataMap.Resource_Type.nutanixVm.value:
      case DataMap.Resource_Type.nutanixHost.value:
      case DataMap.Resource_Type.nutanixCluster.value:
        return this.protectVirtual(params, option);
      case ProtectResourceCategory.host:
      case ProtectResourceCategory.hosts:
        return this.protectHosts(params, option);
      case ProtectResourceCategory.vmware:
      case ProtectResourceCategory.vmwares:
      case ProtectResourceCategory.esix:
      case ProtectResourceCategory.cluster:
        return (isArray(option.data) ? option.data[0] : option.data).resType ===
          ResourceType.HYPERV
          ? this.protectHyperv(params, option)
          : this.protectHostCluster(params, option, type);
      case ProtectResourceCategory.Replica:
        return this.protectReplica(params, option);
      case DataMap.Resource_Type.vmGroup.value:
        return this.protectVmGroup(params, option, type);
    }
  }

  modifyProtect(
    type: string | number,
    params: any,
    option: ModalOption
  ): Observable<void> {
    switch (type) {
      case ProtectResourceCategory.fileset:
      case ProtectResourceCategory.filesets:
      case ProtectResourceCategory.NASShare:
      case ProtectResourceCategory.NASFileSystem:
      case ProtectResourceCategory.HDFS:
      case ProtectResourceCategory.HBase:
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.ndmp.value:
      case DataMap.Resource_Type.LocalFileSystem.value:
      case DataMap.Resource_Type.LocalLun.value:
        return this.modifyFilesetProtect(params, option);
      case ProtectResourceCategory.host:
        return this.modifyHostProtect(params, option);
      case ProtectResourceCategory.GaussDBDWS:
      case ProtectResourceCategory.oracle:
      case ProtectResourceCategory.GaussdbForOpengauss:
      case ProtectResourceCategory.LightCloudGaussDB:
      case ProtectResourceCategory.mysql:
      case DataMap.Resource_Type.MySQL.value:
      case DataMap.Resource_Type.generalDatabase.value:
      case DataMap.Resource_Type.Redis.value:
      case DataMap.Resource_Type.PostgreSQL.value:
      case DataMap.Resource_Type.KingBase.value:
      case ProtectResourceCategory.db2:
      case ProtectResourceCategory.SQLServer:
      case ProtectResourceCategory.GaussDB:
      case ProtectResourceCategory.ClickHouse:
      case ProtectResourceCategory.ClickHouseDatabase:
      case ProtectResourceCategory.ClickHouseTableset:
      case DataMap.Resource_Type.GaussDB_T.value:
      case ProtectResourceCategory.Dameng:
      case ProtectResourceCategory.OpenGauss:
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.SQLServerDatabase.value:
      case DataMap.Resource_Type.MongoDB.value:
      case ProtectResourceCategory.GeneralDB:
      case DataMap.Resource_Type.OceanBaseCluster.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
      case DataMap.Resource_Type.tidbCluster.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.ActiveDirectory.value:
      case DataMap.Resource_Type.ObjectSet.value:
      case DataMap.Resource_Type.Exchange.value:
      case DataMap.Resource_Type.ExchangeDataBase.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
      case DataMap.Resource_Type.tdsqlInstance.value:
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        return this.modifyDatabaseProtect(params, option);
      case DataMap.Resource_Type.CloudHost.value:
      case DataMap.Resource_Type.Project.value:
      case DataMap.Resource_Type.openStackProject.value:
      case DataMap.Resource_Type.openStackCloudServer.value:
      case DataMap.Resource_Type.APSCloudServer.value:
      case DataMap.Resource_Type.APSResourceSet.value:
      case DataMap.Resource_Type.APSZone.value:
        return this.modifyCloudAndVirtual(params, option);
      case DataMap.Resource_Type.fusionComputeVirtualMachine.value:
      case DataMap.Resource_Type.fusionComputeCNA.value:
      case DataMap.Resource_Type.fusionComputeCluster.value:
        return this.modifyFCProtect(params, option);
      case DataMap.Resource_Type.KubernetesStatefulset.value:
      case DataMap.Resource_Type.KubernetesNamespace.value:
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
      case DataMap.Resource_Type.kubernetesDatasetCommon.value:
        return this.modifyKubernentsProtect(params, option);
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.hyperVHost.value:
      case DataMap.Resource_Type.hyperVVm.value:
      case DataMap.Resource_Type.nutanixVm.value:
      case DataMap.Resource_Type.nutanixHost.value:
      case DataMap.Resource_Type.nutanixCluster.value:
        return this.modifyVirtual(params, option);
      case ProtectResourceCategory.vmware:
      case ProtectResourceCategory.vmwares:
        return option.data.resType === ResourceType.HYPERV
          ? this.modifyHypervProtect(params, option)
          : this.modifyVmProtect(params, option);
      case ProtectResourceCategory.esix:
      case ProtectResourceCategory.cluster:
        return option.data.resType === ResourceType.HYPERV
          ? this.modifyHypervProtect(params, option)
          : this.modifyHostClusterProtect(params, option);
      case ProtectResourceCategory.Replica:
        return this.modifyProtectReplica(params, option);
      case DataMap.Resource_Type.vmGroup.value:
        return this.modifyVmGroup(params, option);
    }
  }

  removeProtection(
    resourceIds: Array<string>,
    resourceNames: Array<string>
  ): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_delete_protect_label', [
          toString(resourceNames)
        ]),
        onOK: () => {
          this.projectedObjectApiService
            .deleteV1ProtectedObjectsDelete({
              body: {
                resource_ids: resourceIds
              }
            })
            .subscribe({
              next: () => {
                this.emitSearchStore();
                observer.next();
                observer.complete();
              },
              error: ex => {
                observer.error(ex);
                observer.complete();
              }
            });
        }
      });
    });
  }

  removeGroupProtection(resources: any): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_delete_protect_label', [
          map(resources, 'name').join(',')
        ]),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResourceGroupProtectedObject(
                {
                  resourceGroupId: item.uuid,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
            },
            cloneDeep(resources),
            () => {
              observer.next();
              observer.complete();
            }
          );
        }
      });
    });
  }

  activeProtection(resourceIds: Array<string>): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.projectedObjectApiService
        .activeV1ProtectedObjectsStatusActionActivatePut({
          body: {
            resource_ids: resourceIds
          }
        })
        .subscribe({
          next: () => {
            this.emitSearchStore();
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  deactiveProtection(resourceIds: Array<string>, resourceNames: Array<string>) {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('protection_deactivate_resource_tip_label', [
          toString(resourceNames)
        ]),
        onOK: () => {
          this.projectedObjectApiService
            .deactivateV1ProtectedObjectsStatusActionDeactivatePut({
              body: {
                resource_ids: resourceIds
              }
            })
            .subscribe({
              next: () => {
                this.emitSearchStore();
                observer.next();
                observer.complete();
              },
              error: ex => {
                observer.error(ex);
                observer.complete();
              }
            });
        }
      });
    });
  }

  activeGroupProtection(
    resourceIds: Array<string>,
    type?: string | number
  ): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        activateResourceGroupReq: {
          resource_ids: resourceIds,
          is_resource_group: true
        }
      };
      this.protectedResourceApiService.ActivateResourceGroup(params).subscribe({
        next: () => {
          this.emitSearchStore();
          observer.next();
          observer.complete();
        },
        error: ex => {
          observer.error(ex);
          observer.complete();
        }
      });
    });
  }

  deactiveGroupProtection(
    resourceIds: Array<string>,
    resourceNames: Array<string>,
    type?: string | number
  ) {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('protection_deactivate_resource_tip_label', [
          toString(resourceNames)
        ]),
        onOK: () => {
          const params = {
            deactivateResourceGroupReq: {
              resource_ids: resourceIds,
              is_resource_group: true
            }
          };
          this.protectedResourceApiService
            .DeactivateResourceGroup(params)
            .subscribe({
              next: () => {
                this.emitSearchStore();
                observer.next();
                observer.complete();
              },
              error: ex => {
                observer.error(ex);
                observer.complete();
              }
            });
        }
      });
    });
  }

  protectFilesets(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      // 实时侦测
      if (this.isCyberEngine && params.isRealDetection === true) {
        const bodys = [];
        const pickKeys = [
          'deviceId',
          'fsId',
          'fsName',
          'fsUserId',
          'id',
          'vstoreId'
        ];
        if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
          each(params.selectedList, item => {
            bodys.push({
              name: item.fsName,
              params: {
                policyName: params.slaObject?.name,
                policyId: params.sla_id,
                protectionFsInfo: {
                  ...pick(item, pickKeys),
                  fsUserId: item.userId
                }
              }
            });
          });
        } else {
          bodys.push({
            name: params.fsName,
            params: {
              policyName: params.slaObject?.name,
              policyId: params.sla_id,
              protectionFsInfo: {
                ...pick(params, pickKeys),
                fsUserId: params.userId
              }
            }
          });
        }
        this.messageBox.confirm({
          lvDialogIcon: 'lv-icon-popup-info-48',
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvModalKey: 'active-protectionwarn-modal',
          lvWidth: 350,
          lvHeight: 240,
          lvContent: this.i18n.get('explore_active_detection_tip_label'),
          lvOkDisabled: false,
          lvOk: () => {
            this.batchOperateService.selfGetResults(
              item => {
                return this.detectFilesystemService.createProtectedObject({
                  protectionCreationRequest: item.params,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                });
              },
              bodys,
              () => {
                observer.next();
                observer.complete();
              },
              '',
              true
            );
          },
          lvCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === MESSAGE_BOX_ACTION.close) {
              observer.error(null);
              observer.complete();
            }
          }
        });
        return;
      }
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: includes(
            [DataMap.Resource_Type.LocalFileSystem.value],
            params.sub_type
          )
            ? this.isCyberEngine
              ? {
                  share_type: DataMap.Shared_Mode.nfs.label,
                  file_system_ids: !size(params.selectedList)
                    ? [params.extendInfo.fileSystemId]
                    : [
                        (isArray(params.selectedList)
                          ? find(params.selectedList, {
                              uuid: item.resource_id
                            })
                          : params?.selectedList
                        )?.extendInfo?.fileSystemId
                      ],
                  ...params.ext_parameters
                }
              : {
                  share_type: DataMap.Shared_Mode.nfs.label,
                  file_system_ids: !size(params.selectedList)
                    ? [params.extendInfo.fileSystemId]
                    : [
                        (isArray(params.selectedList)
                          ? find(params.selectedList, {
                              uuid: item.resource_id
                            })
                          : params?.selectedList
                        )?.extendInfo?.fileSystemId
                      ]
                }
            : params.ext_parameters || {}
        };

        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }
        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyFilesetProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      // 批量修改保护
      if (
        params.sub_type === DataMap.Resource_Type.fileset.value &&
        isArray(params.selectedList) &&
        params.selectedList.length > 1
      ) {
        const datas = [];
        each(params.selectedList, item => {
          const body = {
            resource_id: item.uuid,
            sla_id: params.sla_id || item.sla_id,
            ext_parameters:
              {
                ...params.ext_parameters,
                small_file_aggregation:
                  item.protectedObject?.extParameters?.small_file_aggregation,
                aggregation_file_max_size:
                  item.protectedObject?.extParameters
                    ?.aggregation_file_max_size,
                aggregation_file_size:
                  item.protectedObject?.extParameters?.aggregation_file_size
              } || {}
          };
          datas.push({ body, name: item.name, isAsyn: true });
        });

        this.asyncBatchModifyProtect(datas).subscribe(res => {
          observer.next();
          observer.complete();
        });
        return;
      }

      // 实时侦测
      if (this.isCyberEngine && params.isRealDetection === true) {
        const pickKeys = [
          'deviceId',
          'fsId',
          'fsName',
          'fsUserId',
          'id',
          'vstoreId'
        ];
        this.detectFilesystemService
          .modifyProtectedObject({
            protectionModifyReq: {
              policyName: params.slaObject?.name,
              policyId: params.sla_id,
              protectionFsInfo: <any>{
                ...pick(params, pickKeys),
                fsUserId: params.userId
              }
            }
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: ex => {
              observer.error(ex);
              observer.complete();
            }
          });
        return;
      }
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: includes(
          [DataMap.Resource_Type.LocalFileSystem.value],
          params.sub_type
        )
          ? this.isCyberEngine
            ? {
                share_type: DataMap.Shared_Mode.nfs.label,
                file_system_ids:
                  !size(params.selectedList) || !isArray(params.selectedList)
                    ? [params.extendInfo.fileSystemId]
                    : map(params.selectedList, 'extendInfo.fileSystemId'),
                ...params.ext_parameters
              }
            : {
                share_type: DataMap.Shared_Mode.nfs.label,
                file_system_ids:
                  !size(params.selectedList) || !isArray(params.selectedList)
                    ? [params.extendInfo.fileSystemId]
                    : map(params.selectedList, 'extendInfo.fileSystemId')
              }
          : params.ext_parameters || {}
      };
      if (
        includes(
          [
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.NASShare.value
          ],
          params.sub_type
        )
      ) {
        if (
          params?.ext_parameters?.small_file_aggregation !==
          option.originData?.protectedObject?.extParameters
            ?.small_file_aggregation
        ) {
          this.messageService.error(
            this.i18n.get('protect_fileset_aggregation_error_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'fileset_aggregation_error_key'
            }
          );
          observer.error(null);
          return;
        } else {
          params.ext_parameters.aggregation_file_max_size =
            option.originData.protectedObject?.extParameters?.aggregation_file_max_size;
          params.ext_parameters.aggregation_file_size =
            option.originData.protectedObject?.extParameters?.aggregation_file_size;
        }
      }

      if (this.isCyberEngine) {
        this.projectedObjectApiService
          .modifyV1ProtectedObjectsCyberPut({ body })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: ex => {
              observer.error(ex);
              observer.complete();
            }
          });
      } else {
        this.projectedObjectApiService
          .modifyV1ProtectedObjectsPut({ body })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: ex => {
              observer.error(ex);
              observer.complete();
            }
          });
      }
    });
  }

  protectFC(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid,
            status: item.status
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid,
          status: params.status
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: { ...params.ext_parameters }
        };

        if (params.type === DataMap.Resource_Type.FusionComputeVM.value) {
          const diskInfo =
            find(params.selectedList, { uuid: item.resource_id }) || params;
          assign(obj.ext_parameters, {
            disk_info:
              !diskInfo.enableSelectAll && !isEmpty(diskInfo.diskInfo)
                ? diskInfo?.diskInfo.map(cur => {
                    return JSON.stringify(cur);
                  })
                : []
          });
          if (FCVmInNormalStatus.includes(item.status)) {
            obj['status'] = 'FCVmStatusInNormal';
          }
        }
        // 是否执行一次备份
        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyFCProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: params.ext_parameters || {}
      };

      if (params.type === DataMap.Resource_Type.FusionComputeVM.value) {
        assign(body.ext_parameters, {
          disk_info:
            !params.enableSelectAll && !isEmpty(params.diskInfo)
              ? params.diskInfo.map(cur => {
                  if (isString(cur)) {
                    return cur;
                  }
                  return JSON.stringify(cur);
                })
              : []
        });
      }

      if (FCVmInNormalStatus.includes(params.status)) {
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
      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectCloudAndVirtual(
    type,
    params: any,
    option: ModalOption
  ): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid,
            status: item.status
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid,
          status: params.status
        });
      }

      const datas = [];
      each(idArr, item => {
        let resources;
        if (
          includes(
            [
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.openStackProject.value,
              DataMap.Resource_Type.APSCloudServer.value
            ],
            type
          )
        ) {
          resources = [omit(item, 'name', 'status')];
        } else {
          resources = [omit(item, 'name')];
        }
        const obj = {
          name: item.name,
          resources: resources,
          sla_id: params.sla_id,
          ext_parameters: { ...params.ext_parameters }
        };

        if (params.type === DataMap.Resource_Type.CloudHost.value) {
          // HCS
          const vmInfo =
            find(params.selectedList, { uuid: item.resource_id }) || params;
          assign(obj.ext_parameters, {
            disk_info:
              !vmInfo.enableSelectAll && !isEmpty(vmInfo.diskInfo)
                ? vmInfo?.diskInfo
                : []
          });

          if (HCSHostInNormalStatus.includes(item.status)) {
            obj['status'] = 'HCSCloudHostInNormal';
          }
        }

        if (
          includes(
            [
              DataMap.Resource_Type.openStackCloudServer.value,
              DataMap.Resource_Type.APSCloudServer.value
            ],
            params.subType
          )
        ) {
          // openstack,apsarastack
          const diskInfo =
            find(params.selectedList, { uuid: item.resource_id }) || params;
          assign(obj.ext_parameters, {
            disk_info:
              !diskInfo.enableSelectAll && !isEmpty(diskInfo.diskInfo)
                ? diskInfo?.diskInfo
                : [],
            all_disk: diskInfo.enableSelectAll ?? true
          });
        }

        if (
          includes(
            [
              DataMap.Resource_Type.APSZone.value,
              DataMap.Resource_Type.APSResourceSet.value
            ],
            params.subType
          )
        ) {
          assign(obj.ext_parameters, {
            all_disk: true
          });
        }

        // 是否执行一次备份
        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyCloudAndVirtual(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: params.ext_parameters || {}
      };

      if (params.type === DataMap.Resource_Type.CloudHost.value) {
        assign(body.ext_parameters, {
          disk_info:
            !params.enableSelectAll && !isEmpty(params.diskInfo)
              ? params.diskInfo
              : []
        });
      }

      if (HCSHostInNormalStatus.includes(params.status)) {
        this.messageService.error(
          this.i18n.get('protect_hcs_host_innormal_status_label'),
          {
            lvShowCloseButton: true,
            lvMessageKey: 'hcs_host_innormal_key'
          }
        );
        observer.error(this.i18n.get('protect_hcs_host_innormal_status_label'));
        return;
      }

      if (
        includes(
          [
            DataMap.Resource_Type.openStackCloudServer.value,
            DataMap.Resource_Type.APSCloudServer.value
          ],
          params.subType
        )
      ) {
        assign(body.ext_parameters, {
          disk_info:
            !params.enableSelectAll && !isEmpty(params.diskInfo)
              ? params.diskInfo
              : [],
          all_disk: params.enableSelectAll ?? true
        });
      }

      if (
        includes(
          [
            DataMap.Resource_Type.APSZone.value,
            DataMap.Resource_Type.APSResourceSet.value
          ],
          params.subType
        )
      ) {
        assign(body.ext_parameters, {
          all_disk: true
        });
      }

      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectKubernents(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: { ...params.ext_parameters }
        };
        // statusfulset
        if (
          params.subType === DataMap.Resource_Type.KubernetesStatefulset.value
        ) {
          const statefuleset =
            find(params.selectedList, { uuid: item.resource_id }) || params;
          assign(obj.ext_parameters, {
            volume_names:
              !statefuleset.enableSelectAll && !isEmpty(statefuleset.volumes)
                ? statefuleset.volumes
                : JSON.parse(get(statefuleset, ['extendInfo', 'sts']))
                    ?.volumeNames
          });
        }
        // namespace
        if (
          params.subType === DataMap.Resource_Type.KubernetesNamespace.value
        ) {
          assign(obj, {
            filters: [
              {
                filter_by: 'SLOT',
                type: 'DISK',
                rule: 'ALL',
                mode: 'INCLUDE',
                values: ['*']
              }
            ]
          });
        }
        // 是否执行一次备份
        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyKubernentsProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: params.ext_parameters || {}
      };

      if (
        params.subType === DataMap.Resource_Type.KubernetesStatefulset.value
      ) {
        assign(body.ext_parameters, {
          volume_names:
            !params.enableSelectAll && !isEmpty(params.volumes)
              ? params.volumes
              : JSON.parse(get(params, ['extendInfo', 'sts']))?.volumeNames
        });
      }

      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectVirtual(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: params.ext_parameters || {}
        };

        if (
          includes(
            [
              DataMap.Resource_Type.hyperVVm.value,
              DataMap.Resource_Type.cNwareVm.value,
              DataMap.Resource_Type.nutanixVm.value
            ],
            params.subType
          )
        ) {
          const diskInfo =
            find(params.selectedList, { uuid: item.resource_id }) || params;
          const { enableSelectAll, diskInfo: disks } = diskInfo;
          let disk_info = [];
          if (!enableSelectAll && !isEmpty(disks)) {
            disk_info = disks;
            if (params.subType === DataMap.Resource_Type.cNwareVm.value) {
              disk_info = map(disks, JSON.stringify);
            }
          }
          assign(obj.ext_parameters, {
            disk_info,
            all_disk: !!enableSelectAll
          });
        }

        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyVirtual(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: params.ext_parameters || {}
      };
      const { enableSelectAll, diskInfo: disks } = params;
      let disk_info = [];
      if (!enableSelectAll && !isEmpty(disks)) {
        if (params.subType === DataMap.Resource_Type.hyperVVm.value) {
          disk_info = disks;
        } else {
          disk_info = map(disks, JSON.stringify);
        }
      }
      assign(body.ext_parameters, {
        disk_info,
        all_disk: !!enableSelectAll
      });
      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectDatabases(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            name: item.name,
            resource_id: item.uuid
          });
        });
      } else {
        idArr.push({
          name: params.name,
          resource_id: params.uuid
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: params.ext_parameters || {}
        };

        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  protectVmGroup(
    params: any,
    option: ModalOption,
    type: string
  ): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const idArr = [];
      if (isArray(params.selectedList) && !isEmpty(params.selectedList)) {
        each(params.selectedList, item => {
          idArr.push({
            resourceGroupId: item.uuid,
            name: item.name
          });
        });
      } else {
        idArr.push({
          resourceGroupId: params.uuid,
          name: params.name
        });
      }

      const datas = [];
      each(idArr, item => {
        const obj = {
          resourceGroupId: item.resourceGroupId,
          slaId: params.sla_id,
          extParams: cloneDeep(params.ext_parameters) || {},
          name: item.name,
          type: DataMap.Resource_Type.vmGroup.value
        };
        // 批量操作时，每次下发的请求都要携带资源uuid
        assign(obj.extParams, {
          disk_info: [],
          all_disk: true
        });

        if (
          includes(
            [DataMap.Resource_Type.virtualMachine.value],
            params.sourceSubType
          )
        ) {
          set(obj, 'extParams.concurrent_requests_uuid', item.resourceGroupId);
        }

        // 是否执行一次备份
        if (params.post_action) {
          assign(obj, { postAction: params.post_action });
        }
        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas, type).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyVmGroup(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resourceGroupId: params.uuid,
        slaId: params.sla_id,
        extParams: params.ext_parameters || {},
        type: DataMap.Resource_Type.vmGroup.value
      };
      assign(body.extParams, {
        disk_info: [],
        all_disk: true
      });
      // 虚拟机组保护都会走这，注意判断类型vm、cnware、fc
      if (params.sourceSubType === DataMap.Resource_Type.virtualMachine.value) {
        set(body, 'extParams.concurrent_requests_uuid', params.uuid);
      }

      this.batchOperateService.selfGetResults(
        item => {
          return this.protectedResourceApiService.UpdateResourceGroupProtectedObject(
            {
              UpdateResourceGroupProtectedObjectRequestBody: <any>(
                omit(item.body, 'name')
              ),
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            }
          );
        },
        map(cloneDeep([body]), (item: any) => {
          return {
            body: item,
            name: item.name,
            isAsyn: false
          };
        }),
        () => {
          observer.next();
          observer.complete();
        },
        '',
        true
      );
    });
  }

  modifyDatabaseProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext_parameter = !isArray(option.originData)
        ? get(
            option,
            'originData?.protectedObject.extParameters.multiNodeBackupSwitch',
            false
          )
        : false;
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: params.ext_parameters || {}
      };
      if (
        params.subType === DataMap.Resource_Type.ObjectSet.value &&
        ext_parameter &&
        !params.ext_parameters?.multiNodeBackupSwitch
      ) {
        // 该功能暂时被裁减
        this.warningMessageService.create({
          content: this.i18n.get('protection_object_modify_protect_tip_label'),
          header: this.i18n.get('common_warn_label'),
          onOK: () => {
            this.projectedObjectApiService
              .modifyV1ProtectedObjectsPut({ body })
              .subscribe({
                next: () => {
                  observer.next();
                  observer.complete();
                },
                error: ex => {
                  observer.error(ex);
                  observer.complete();
                }
              });
          },
          onCancel: () => {
            observer.error('');
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      } else {
        this.projectedObjectApiService
          .modifyV1ProtectedObjectsPut({ body })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: ex => {
              observer.error(ex);
              observer.complete();
            }
          });
      }
    });
  }

  protectHosts(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      const resourceList = [];
      if (params.selectedList) {
        each(params.selectedList, item => {
          resourceList.push({
            name: item.name,
            resource_id: item.uuid,
            filters: [
              {
                filter_by: 'ID',
                type: 'VOLUME',
                rule: 'ALL',
                mode: 'INCLUDE',
                values: map(item.disks, 'path')
              }
            ]
          });
        });
      }

      const datas = [];
      each(resourceList, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: ext
        };

        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyHostProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const paths = [];
      if (params.selectedList) {
        each(first(params.selectedList)['disks'], item => {
          paths.push(item.path);
        });
      }
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: assign({ paths }, params.ext_parameters)
      };

      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  modifyVmProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      if (params.selectedList) {
        if (!isEmpty(first(params.selectedList)['disksInfo'])) {
          const diskList = [];
          each(first(params.selectedList)['disksInfo'], item => {
            if (!isEmpty(item.selection)) {
              each(item.selection, d => {
                diskList.push(d.uuid);
              });
            }
          });
          assign(ext, {
            all_disk:
              diskList.length === params.selectedList[0].allDisks.length,
            disk_info: diskList
          });
        } else {
          const extParam = params.selectedList[0]?.ext_parameters || {};
          assign(ext, {
            all_disk: get(extParam, 'all_disk', []),
            disk_info: get(extParam, 'disk_info', []),
            first_backup_target: get(extParam, 'first_backup_target', null),
            last_backup_target: get(extParam, 'last_backup_target', null),
            priority_backup_target: get(
              extParam,
              'priority_backup_target',
              null
            ),
            first_backup_esn: get(extParam, 'first_backup_esn', null),
            last_backup_esn: get(extParam, 'last_backup_esn', null),
            priority_backup_esn: get(extParam, 'priority_backup_esn', null)
          });
        }
      }
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: ext
      };

      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectHostCluster(params: any, option: ModalOption, type): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      const resourceList = [];
      if (params.selectedList) {
        each(params.selectedList, item => {
          if (item['diskError']) {
            return;
          }
          const diskValues = this.getDiskValues(item);
          resourceList.push({
            name: item.name,
            resource_id: item.uuid,
            filters: [
              {
                filter_by: includes(
                  [
                    ProtectResourceCategory.vmware,
                    ProtectResourceCategory.vmwares
                  ],
                  type
                )
                  ? 'ID'
                  : 'SLOT',
                type: 'DISK',
                rule: 'ALL',
                mode: 'INCLUDE',
                values:
                  item.allDisks && item.allDisks.length === diskValues.length
                    ? ['*']
                    : diskValues
              }
            ]
          });
        });
      }

      const datas = [];
      each(resourceList, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: cloneDeep(ext)
        };
        if (
          includes(
            [ProtectResourceCategory.esix, ProtectResourceCategory.cluster],
            type
          )
        ) {
          set(obj, 'ext_parameters.concurrent_requests_uuid', item.resource_id);
        }
        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  getDiskValues(item) {
    let diskValues = [];
    if (isEmpty(item['disksInfo'])) {
      if (item?.allUsableDisks?.length === item?.allDisks?.length) {
        diskValues = ['*'];
      } else {
        // 考虑uuid不存在的磁盘
        each(item.allUsableDisks, selection => {
          diskValues.push(selection.uuid);
        });
      }
    }
    each(item.disksInfo, disk => {
      each(disk.selection, selection => {
        diskValues.push(selection.uuid);
      });
    });
    return diskValues;
  }

  modifyHostClusterProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      if (params.selectedList) {
        let diskValues = [];
        if (isEmpty(params.selectedList[0].disksInfo)) {
          diskValues = params.selectedList[0].ext_parameters.disk_info;
        }
        each(params.selectedList[0].disksInfo, disk => {
          each(disk.selection, selection => {
            diskValues.push(selection.slot);
          });
        });
        ext['disk_filters'] = [
          {
            filter_by: 'SLOT',
            type: 'DISK',
            rule: 'ALL',
            mode: 'INCLUDE',
            values:
              diskValues.length === params.selectedList[0].allUsableDisks.length
                ? ['*']
                : diskValues
          }
        ];
      }
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: ext
      };
      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectHyperv(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      const resourceList = [];
      if (params.selectedList) {
        each(params.selectedList, item => {
          resourceList.push({
            name: item.name,
            resource_id: item.uuid,
            filters: []
          });
        });
      }

      const datas = [];
      each(resourceList, item => {
        const obj = {
          name: item.name,
          resources: [omit(item, 'name')],
          sla_id: params.sla_id,
          ext_parameters: ext
        };

        if (params.post_action) {
          assign(obj, { post_action: params.post_action });
        }

        datas.push(obj);
      });

      this.asyncBatchCreateProtect(datas).subscribe(res => {
        observer.next();
        observer.complete();
      });
    });
  }

  modifyHypervProtect(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const ext = cloneDeep(params.ext_parameters);
      const body = {
        resource_id: params.uuid,
        sla_id: params.sla_id,
        ext_parameters: ext
      };
      this.projectedObjectApiService
        .modifyV1ProtectedObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  protectReplica(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.resource_id,
        sla_id: params.sla_id
      };
      const ext = cloneDeep(params.ext_parameters);
      if (!isEmpty(ext)) {
        assign(body, {
          ext_parameters: ext
        });
      }
      this.protectedCopyObjectApiService
        .createV1ProtectedCopyObjectsPost({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  modifyProtectReplica(params: any, option: ModalOption): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const body = {
        resource_id: params.resource_id,
        sla_id: params.sla_id
      };
      const ext = cloneDeep(params.ext_parameters);
      if (!isEmpty(ext)) {
        assign(body, {
          ext_parameters: ext
        });
      }
      this.protectedCopyObjectApiService
        .modifyV1ProtectedCopyObjectsPut({ body })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: ex => {
            observer.error(ex);
            observer.complete();
          }
        });
    });
  }

  activeDetectionProtection(data): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.messageBox.confirm({
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvHeader: this.i18n.get('common_alarms_info_label'),
        lvModalKey: 'active-protectionwarn-modal',
        lvWidth: 450,
        lvHeight: 280,
        lvContent: this.i18n.get('explore_active_detection_tip_label'),
        lvOkDisabled: false,
        lvOk: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.detectFilesystemService.activateProtectedObjects({
                protectionActivateReq: {
                  protectionFsInfo: {
                    ...pick(item, [
                      'deviceId',
                      'fsId',
                      'fsName',
                      'fsUserId',
                      'id',
                      'vstoreId'
                    ]),
                    fsUserId: item.userId
                  }
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            map(cloneDeep(data), item => {
              return assign(item, {
                name: item.fsName
              });
            }),
            () => {
              observer.next();
              observer.complete();
            },
            '',
            true
          );
        }
      });
    });
  }

  warnModal(
    observer: Observer<void>,
    data,
    type: ResourceOperationType,
    content: string
  ) {
    this.messageBox.confirm({
      lvDialogIcon: 'lv-icon-popup-danger-48',
      lvHeader: this.i18n.get('common_danger_label'),
      lvModalKey: 'detection-protectionwarn-modal',
      lvWidth: 450,
      lvHeight: 280,
      lvContent: WarnModalComponent,
      lvOkDisabled: false,
      lvComponentParams: {
        data,
        type,
        content
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as WarnModalComponent;
        content.onOK().subscribe(() => {
          observer.next();
          observer.complete();
        });
      }
    });
  }

  deactiveDetectionProtection(data): Observable<void> {
    return new Observable<void>((observer: Observer<void>) =>
      this.warnModal(
        observer,
        data,
        ResourceOperationType.protection,
        this.i18n.get('explore_deactive_detection_warn_label', [
          map(data, 'fsName').join(',')
        ])
      )
    );
  }

  removeDetectionProtection(data): Observable<void> {
    return new Observable<void>((observer: Observer<void>) =>
      this.warnModal(
        observer,
        data,
        ResourceOperationType.deletion,
        this.i18n.get('explore_remove_detection_warn_label', [
          map(data, 'fsName').join(',')
        ])
      )
    );
  }
}
