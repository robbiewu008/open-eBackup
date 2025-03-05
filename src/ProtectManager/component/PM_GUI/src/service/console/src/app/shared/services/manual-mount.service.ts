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
import { CommonModule } from '@angular/common';
import { Injectable, NgModule } from '@angular/core';
import { LiveMountOptionsComponent as CnwareLiveMountOptionsComponent } from 'app/business/explore/live-mounts/cnware/live-mount-options/live-mount-options.component';
import { LiveMountOptionsModule as CnwareLiveMountOptionsModule } from 'app/business/explore/live-mounts/cnware/live-mount-options/live-mount-options.module';
import { LiveMountOptionsComponent as FilesetLiveMountOptionsComponent } from 'app/business/explore/live-mounts/fileset/live-mount-options/live-mount-options.component';
import { LiveMountOptionsModule as FilesetLiveMountOptionsModule } from 'app/business/explore/live-mounts/fileset/live-mount-options/live-mount-options.module';
import { LiveMountOptionsComponent as NasSharedLiveMountOptionsComponent } from 'app/business/explore/live-mounts/nas-shared/live-mount-options/live-mount-options.component';
import { LiveMountOptionsModule as NasSharedLiveMountOptionsModule } from 'app/business/explore/live-mounts/nas-shared/live-mount-options/live-mount-options.module';
import { LiveMountOptionsComponent as OracleLiveMountOptionsComponent } from 'app/business/explore/live-mounts/oracle/live-mount-options/live-mount-options.component';
import { LiveMountOptionsModule as OracleLiveMountOptionsModule } from 'app/business/explore/live-mounts/oracle/live-mount-options/live-mount-options.module';
import { LiveMountOptionsComponent as VMwareLiveMountOptionsComponent } from 'app/business/explore/live-mounts/vmware/live-mount-options/live-mount-options.component';
import { LiveMountOptionsModule as VMwareLiveMountOptionsModule } from 'app/business/explore/live-mounts/vmware/live-mount-options/live-mount-options.module';
import { assign, first, includes, isFunction } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';
import {
  DataMap,
  GlobalService,
  I18NService,
  LiveMountAction,
  LiveMountApiService,
  MODAL_COMMON,
  ResourceType,
  VirtualResourceService,
  WarningMessageService
} from '..';
import { AppUtilsService } from './app-utils.service';
import { DrawModalService } from './draw-modal.service';

export interface Params {
  item: any;
  resType: ResourceType; // 主类型
  onOk: () => void;
  warning?: true;
}

@Injectable({
  providedIn: 'root'
})
export class ManualMountService {
  constructor(
    public appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private globalService: GlobalService,
    private drawModalService: DrawModalService,
    private liveMountApiService: LiveMountApiService,
    private virtualResourceService: VirtualResourceService,
    private warningMessageService: WarningMessageService
  ) {}

  create(param: Params, drillCb?) {
    this.getLiveMountParams(param.item).subscribe(liveMountParams => {
      const liveMountComponent = this.getLiveMountComponent(param.item);
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvHeader: this.i18n.get('common_live_mount_label'),
        lvContent: liveMountComponent,
        lvWidth: includes(
          [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.MySQLClusterInstance.value
          ],
          param.item.resource_sub_type
        )
          ? MODAL_COMMON.xLargeWidth
          : MODAL_COMMON.largeWidth,
        lvComponentParams: { ...liveMountParams, isDrill: isFunction(drillCb) },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          this.afterOpen(modal, param.item);
        },
        lvOk: modal => {
          if (isFunction(drillCb)) {
            const component = modal.getContentComponent();
            const mountParams = component.getComponentData()?.requestParams;
            const targetLocation = isFunction(component.getTargetLocation)
              ? component.getTargetLocation()
              : '';

            return new Promise(resolve => {
              if (
                param.warning &&
                includes(
                  [
                    DataMap.Resource_Type.oracle.value,
                    DataMap.Resource_Type.oracleCluster.value
                  ],
                  param.item.resource_sub_type
                ) &&
                component.oracleOfflineWarnTip
              ) {
                this.warningMessageService.create({
                  content: component.oracleOfflineWarnTip,
                  onOK: () => {
                    drillCb(mountParams, targetLocation);
                    resolve(true);
                  },
                  onCancel: () => resolve(false),
                  lvAfterClose: result => {
                    if (result && result.trigger === 'close') {
                      resolve(false);
                    }
                  }
                });
              } else {
                drillCb(mountParams, targetLocation);
                resolve(true);
              }
            });
          }

          return new Promise(resolve => {
            const component = modal.getContentComponent();
            const componentData = component.getComponentData();
            if (
              includes(
                [
                  DataMap.Resource_Type.oracle.value,
                  DataMap.Resource_Type.oracleCluster.value
                ],
                param.item.resource_sub_type
              ) &&
              component.oracleOfflineWarnTip
            ) {
              this.warningMessageService.create({
                content: component.oracleOfflineWarnTip,
                onOK: () => {
                  this.executeLiveMount(componentData, resolve, param);
                },
                onCancel: () => resolve(false),
                lvAfterClose: result => {
                  if (result && result.trigger === 'close') {
                    resolve(false);
                  }
                }
              });
            } else {
              if (
                this.appUtilsService.isDistributed &&
                param.item.generated_by ===
                  DataMap.CopyData_generatedType.replicate.value &&
                [
                  DataMap.Resource_Type.MySQL.value,
                  DataMap.Resource_Type.MySQLInstance.value,
                  DataMap.Resource_Type.MySQLClusterInstance.value,
                  DataMap.Resource_Type.MySQLDatabase.value,
                  DataMap.Resource_Type.virtualMachine.value,
                  DataMap.Resource_Type.oracle.value,
                  DataMap.Resource_Type.oracleCluster.value
                ].includes(param.item.resource_sub_type)
              ) {
                this.warningMessageService.create({
                  header: this.i18n.get(
                    'explore_distributed_live_mount_header_label'
                  ),
                  width: MODAL_COMMON.normalWidth,
                  content: this.i18n.get(
                    'explore_distributed_live_mount_tip_label'
                  ),
                  onOK: () => {
                    this.executeLiveMount(componentData, resolve, param);
                  },
                  onCancel: () => resolve(false),
                  lvAfterClose: result => {
                    if (result && result.trigger === 'close') {
                      resolve(false);
                    }
                  }
                });
              } else {
                this.executeLiveMount(componentData, resolve, param);
              }
            }
          });
        }
      });
    });
  }

  private executeLiveMount(
    componentData: any,
    resolve: (value: unknown) => void,
    param: Params
  ) {
    this.liveMountApiService
      .createLiveMountUsingPOST({
        liveMountObject: componentData.requestParams
      })
      .subscribe(
        res => {
          resolve(true);
          param.onOk();
        },
        error => resolve(false)
      );
  }

  getVirtualResource(resource) {
    return this.virtualResourceService.queryResourcesV1VirtualResourceGet({
      pageSize: 10,
      pageNo: 0,
      conditions: JSON.stringify({
        uuid: resource.parent_uuid
      })
    });
  }

  afterOpen(modal, item) {
    const component = modal.getContentComponent();
    if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.tdsqlInstance.value,
          DataMap.Resource_Type.MySQLInstance.value
        ],
        item.resource_sub_type
      )
    ) {
      component.getTargetHostOptions();
      this.globalService.emitStore({
        action: LiveMountAction.SelectResource,
        state: item
      });
    } else if (
      includes(
        [
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.cNwareVm.value
        ],
        item.resource_sub_type
      )
    ) {
      component.initData();
    }
    const modalIns = modal.getInstance();
    if (
      includes(
        [DataMap.Resource_Type.cNwareVm.value],
        item.resource_sub_type
      ) ||
      (includes([DataMap.Resource_Type.volume.value], item.resource_sub_type) &&
        JSON.parse(item.resource_properties || '{}')?.environment_os_type ===
          DataMap.Os_Type.windows.value)
    ) {
      component.valid$.subscribe(res => {
        modalIns.lvOkDisabled = !res;
      });
    } else {
      component.formGroup.statusChanges.subscribe(res => {
        modalIns.lvOkDisabled = res !== 'VALID';
      });
    }
  }

  getLiveMountParams(item): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      let params: any = {};
      const resourceProperties = JSON.parse(item.resource_properties) as any;
      if (
        includes(
          [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value,
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.MySQLInstance.value
          ],
          item.resource_sub_type
        )
      ) {
        assign(params, {
          componentData: {
            rowCopy: item,
            requestParams: {
              copy_id: item.uuid,
              source_resource_id: item.resource_id
            },
            selectionResource: {
              resource_name: resourceProperties.name,
              environment_uuid: resourceProperties.environment_uuid,
              environment_is_cluster: resourceProperties.environment_is_cluster,
              resource_sub_type: item.resource_sub_type,
              resource_properties: item.resource_properties
            },
            childResourceType: [item.resource_sub_type]
          }
        });
        if (
          includes(
            [
              DataMap.Resource_Type.tdsqlInstance.value,
              DataMap.Resource_Type.MySQLInstance.value
            ],
            item.resource_sub_type
          )
        ) {
          assign(params.componentData, {
            childResourceType: [item.resource_sub_type]
          });
        }
        if (
          includes(
            [DataMap.Resource_Type.MySQLInstance.value],
            item.resource_sub_type
          )
        ) {
          assign(params, {
            componentData: {
              childResourceType: [DataMap.Resource_Type.MySQLInstance.value],
              ...params.componentData
            }
          });
        }
        observer.next(params);
        observer.complete();
      }

      if (
        item.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
      ) {
        this.getVirtualResource(resourceProperties)
          .pipe(
            map(res => {
              return first(res.items);
            })
          )
          .subscribe(res => {
            assign(params, {
              componentData: {
                requestParams: {
                  copy_id: item.uuid,
                  source_resource_id: item.resource_id
                },
                selectionResource: {
                  ...resourceProperties,
                  resource_id: item.resource_id,
                  original_location: !res ? resourceProperties.path : res.path
                },
                selectionCopy: item
              }
            });
            observer.next(params);
            observer.complete();
          });
      }

      if (
        includes(
          [
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value,
            DataMap.Resource_Type.cNwareVm.value
          ],
          item.resource_sub_type
        )
      ) {
        assign(params, {
          componentData: {
            requestParams: {
              copy_id: item.uuid,
              source_resource_id: item.resource_id
            },
            selectionResource: {
              ...resourceProperties,
              resource_id: item.resource_id
            },
            selectionCopy: item
          }
        });
        observer.next(params);
        observer.complete();
      }
    });
  }

  getLiveMountComponent(item) {
    let component;
    if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.tdsqlInstance.value,
          DataMap.Resource_Type.MySQLInstance.value
        ],
        item.resource_sub_type
      )
    ) {
      component = OracleLiveMountOptionsComponent;
    } else if (
      item.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
    ) {
      component = VMwareLiveMountOptionsComponent;
    } else if (
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value
        ],
        item.resource_sub_type
      )
    ) {
      component = NasSharedLiveMountOptionsComponent;
    } else if (
      includes(
        [
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.volume.value
        ],
        item.resource_sub_type
      )
    ) {
      component = FilesetLiveMountOptionsComponent;
    } else if (
      includes([DataMap.Resource_Type.cNwareVm.value], item.resource_sub_type)
    ) {
      component = CnwareLiveMountOptionsComponent;
    }
    return component;
  }
}

@NgModule({
  imports: [
    CommonModule,
    OracleLiveMountOptionsModule,
    VMwareLiveMountOptionsModule,
    FilesetLiveMountOptionsModule,
    NasSharedLiveMountOptionsModule,
    CnwareLiveMountOptionsModule
  ],
  providers: [ManualMountService]
})
export class ManualMountModule {}
