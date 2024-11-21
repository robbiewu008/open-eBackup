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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageboxService } from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  includes,
  indexOf,
  isEmpty,
  isNumber,
  isString,
  map,
  pick,
  reject,
  some
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-kubernetes-restore',
  templateUrl: './kubernetes-restore.component.html',
  styleUrls: ['./kubernetes-restore.component.less']
})
export class KubernetesRestoreComponent implements OnInit {
  rowCopy;
  childResType;
  restoreType;

  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  resourceProp: any;
  properties: any;
  pvcTableData: any[] = [];
  originPvcs: any[];
  clusterOptions: any[];
  namespaceOptions: any[];
  statefulsetOptions: any[];

  originClusterOptions: any[];
  originNamespaceOptions: any[];
  originStatefulsetOptions: any[];

  restoreToNewLocationOnly: boolean = false;

  pageSizeOptions = CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  pvcValid$ = new Subject<boolean>();
  @ViewChild('page', { static: false }) page;
  @ViewChild('warnTpl', { static: true }) warnTpl: TemplateRef<any>;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };

  verifyStatus;
  copyVerifyDisableLabel: string;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;

  proxyOptions = [];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appService: AppService,
    private messageBox: MessageboxService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getCluster();
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
  }

  initCopyVerifyDisableLabel() {
    if (
      includes([this.CopyDataVerifyStatus.noGenerate.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_generate_verify_file_disable_label'
      );
    }
    if (
      includes([this.CopyDataVerifyStatus.Invalid.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_invalid_verify_file_disable_label'
      );
    }
  }

  avalibleSize(recovey, target): boolean {
    const unit = ['Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei'];
    if (isEmpty(recovey.size) || isEmpty(target.size)) {
      return true;
    }
    const recoveryUnit = indexOf(unit, recovey.size?.slice(-2));
    const targetUnit = indexOf(unit, target.size?.slice(-2));
    if (recoveryUnit === -1 || targetUnit === -1) {
      return true;
    }
    return (
      parseFloat(target.size) * 1024 ** targetUnit >=
      parseFloat(recovey.size) * 1024 ** recoveryUnit
    );
  }

  getTargetPvc(pv: any, pods: any[]): any[] {
    let pvcArr: any = [];
    each(pods, item => {
      pvcArr = [
        ...pvcArr,
        ...filter(item.pvs, p => p.volumeName === pv.volumeName)
      ];
    });
    return map(
      filter(pvcArr, val => {
        return this.avalibleSize(pv, val);
      }),
      item => {
        return assign(item, {
          label: item.pvcName,
          value: item.pvcName,
          isLeaf: true
        });
      }
    );
  }

  initForm() {
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.properties = JSON.parse(this.rowCopy.properties);
    this.verifyStatus = this.properties?.verifyStatus;
    // 复制副本
    if (
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cloudArchival.value,
          DataMap.CopyData_generatedType.tapeArchival.value,
          DataMap.CopyData_generatedType.cascadedReplication.value,
          DataMap.CopyData_generatedType.reverseReplication.value
        ],
        this.rowCopy.generated_by
      ) &&
      isEmpty(this.properties.volList)
    ) {
      assign(this.properties, {
        volList: this.properties.extendInfo?.volList
      });
    }
    if (isString(this.properties?.volList)) {
      assign(this.properties, {
        volList: JSON.parse(this.properties.volList)
      });
    }
    defer(() => {
      this.originClusterOptions = [
        {
          value: this.resourceProp?.environment_uuid,
          label: this.resourceProp?.environment_name,
          isLeaf: true
        }
      ];
      this.originNamespaceOptions = [
        {
          value: this.resourceProp?.parent_uuid,
          label: this.resourceProp?.parent_name,
          isLeaf: true
        }
      ];
      this.originStatefulsetOptions = [
        {
          value: this.resourceProp?.uuid,
          label: this.resourceProp?.name,
          isLeaf: true
        }
      ];
      this.getOriginalStatefulset();
    });
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originCluster: new FormControl(this.resourceProp?.environment_uuid),
      originNamespace: new FormControl(this.resourceProp?.parent_uuid),
      originStatefulset: new FormControl(this.resourceProp?.uuid),
      cluster: new FormControl(''),
      namespace: new FormControl(''),
      statefulset: new FormControl(''),
      proxyHost: new FormControl([]),
      preScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxScript,
            false
          ),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxScript,
            false
          ),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.linuxScript,
            false
          ),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      }),
      copyVerify: new FormControl(false)
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      each(this.pvcTableData, item => {
        item.targetPvc = '';
        item.targetPvcOptions = [];
      });
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('cluster').clearValidators();
        this.formGroup.get('namespace').clearValidators();
        this.formGroup.get('statefulset').clearValidators();
        this.getOriginalStatefulset();
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('namespace')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('statefulset')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (this.formGroup.value.statefulset) {
          this.getLocationPvc(
            find(this.statefulsetOptions, {
              value: this.formGroup.value.statefulset
            })
          );
        }
        this.validPvc();
      }
      this.formGroup
        .get('cluster')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup
        .get('namespace')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup
        .get('statefulset')
        .updateValueAndValidity({ emitEvent: false });
    });

    this.formGroup
      .get('cluster')
      .valueChanges.subscribe(res => defer(() => this.getNamespace()));

    this.formGroup
      .get('namespace')
      .valueChanges.subscribe(res => defer(() => this.getStatefulset()));

    this.formGroup.get('statefulset').valueChanges.subscribe(res =>
      defer(() => {
        const statefulset = find(this.statefulsetOptions, { value: res });
        this.getLocationPvc(statefulset);
      })
    );

    if (this.restoreToNewLocationOnly) {
      defer(() => {
        this.formGroup.patchValue({
          restoreTo: RestoreV2LocationType.NEW
        });
      });
    }
  }

  getPod(pod): string {
    return pod?.split('-').pop();
  }

  getLocationPvc(resource, isOriginal?: boolean) {
    const sts = JSON.parse(resource.extendInfo.sts);
    this.pvcTableData = map(this.properties.volList, item => {
      const vol = item.extendInfo ? JSON.parse(item.extendInfo) : {};
      const target = this.getTargetPvc(vol.pv, sts.pods);
      const findSamePod = find(target, pod => {
        return this.getPod(pod.value) === this.getPod(vol.pv?.pvcName);
      });
      return {
        name: vol.pv?.pvcName,
        id: item.uuid,
        targetPvc: isOriginal ? findSamePod?.value || target[0]?.value : '',
        originTargetPvc: findSamePod?.value,
        targetPvcOptions: target
      };
    });
  }

  validPvc() {
    this.pvcValid$.next(
      some(this.pvcTableData, item => {
        return !isEmpty(item.targetPvc);
      })
    );
  }

  pvcChange(item) {
    if (item.targetPvc) {
      if (this.getPod(item.targetPvc) !== this.getPod(item.name)) {
        this.messageBox.confirm({
          lvHeader: this.i18n.get('protection_confirm_title_label'),
          lvDialogIcon: 'lv-icon-popup-danger-48',
          lvContent: this.warnTpl,
          lvWidth: 450,
          lvOkType: 'primary',
          lvCancelType: 'default',
          lvOk: () => this.setOtherTargetPvc(item),
          lvCancel: () => {
            item.targetPvc = item.originTargetPvc;
            item.originTargetPvc = item.targetPvc;
            this.validPvc();
          }
        });
      } else {
        this.setOtherTargetPvc(item);
      }
    }
    this.validPvc();
  }

  setOtherTargetPvc(item) {
    each(this.pvcTableData, pod => {
      if (
        this.getPod(pod.name) === this.getPod(item.name) &&
        pod.id !== item.id &&
        pod.targetPvc
      ) {
        const findPod = find(pod.targetPvcOptions, v => {
          return this.getPod(v.value) === this.getPod(item.targetPvc);
        });
        pod.targetPvc = findPod?.value;
        pod.originTargetPvc = findPod?.value;
      }
    });
    this.validPvc();
  }

  getCluster(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.Kubernetes.value]
        })
      })
      .subscribe(res => {
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
          if (!this.restoreToNewLocationOnly) {
            recordsTemp = reject(recordsTemp, item => {
              return item.uuid === this.resourceProp.environment_uuid;
            });
          }
          this.clusterOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getCluster(recordsTemp, startPage);
      });
  }

  getNamespace(recordsTemp?: any[], startPage?: number) {
    if (!this.formGroup.value.cluster) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.KubernetesNamespace.value],
          path: [
            ['=~'],
            find(this.clusterOptions, { value: this.formGroup.value.cluster })
              ?.path
          ]
        })
      })
      .subscribe(res => {
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
          this.namespaceOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getNamespace(recordsTemp, startPage);
      });
  }

  getStatefulset(recordsTemp?: any[], startPage?: number) {
    if (!this.formGroup.value.namespace) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.KubernetesStatefulset.value],
          path: [
            ['=~'],
            find(this.namespaceOptions, {
              value: this.formGroup.value.namespace
            })?.path
          ]
        })
      })
      .subscribe(res => {
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
          recordsTemp = filter(recordsTemp, item => {
            return this.restoreToNewLocationOnly
              ? this.resourceProp.name?.split('-')[0] ===
                  item.name?.split('-')[0]
              : this.resourceProp.uuid !== item.uuid &&
                  this.resourceProp.name?.split('-')[0] ===
                    item.name?.split('-')[0];
          });
          this.statefulsetOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getStatefulset(recordsTemp, startPage);
      });
  }

  getOriginalStatefulset() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceProp?.environment_uuid
      })
      .subscribe((res: any) => {
        const onlineAgents = res?.dependencies?.agents?.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        if (!isEmpty(onlineAgents)) {
          const agentId = onlineAgents[0].uuid;
          this.getResourcesDetails(
            agentId,
            [this.resourceProp.parent_uuid],
            this.resourceProp.environment_uuid
          );
        }
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Kubernetes.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getResourcesDetails(
    agentId,
    resourceIds,
    envId,
    recordsTemp?: any[],
    startPage?: number
  ) {
    this.appService
      .ListResourcesDetails({
        agentId,
        resourceIds,
        envId,
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({ statefulSet: this.resourceProp.name })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) + 1 ||
          res.totalCount === 0
        ) {
          this.getLocationPvc(recordsTemp[0], true);
          if (this.restoreToNewLocationOnly) {
            each(this.pvcTableData, item => {
              item.targetPvc = '';
              item.targetPvcOptions = [];
            });
          }
          this.validPvc();
          return;
        }
        this.getResourcesDetails(
          agentId,
          resourceIds,
          envId,
          recordsTemp,
          startPage
        );
      });
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? `${
          find(this.originClusterOptions, {
            value: this.formGroup.value.originCluster
          })?.label
        }/${
          find(this.originNamespaceOptions, {
            value: this.formGroup.value.originNamespace
          })?.label
        }/${
          find(this.originStatefulsetOptions, {
            value: this.formGroup.value.originStatefulset
          })?.label
        }`
      : `${
          find(this.clusterOptions, {
            value: this.formGroup.value.cluster
          })?.label
        }/${
          find(this.namespaceOptions, {
            value: this.formGroup.value.namespace
          })?.label
        }/${
          find(this.statefulsetOptions, {
            value: this.formGroup.value.statefulset
          })?.label
        }`;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copyId: this.rowCopy.uuid,
        agents: this.formGroup.value.proxyHost || [],
        targetEnv:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProp?.environment_uuid
            : this.formGroup.value.cluster,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        subObjects: map(
          filter(this.pvcTableData, v => !isEmpty(v.targetPvc)),
          item => {
            if (item.targetPvc) {
              return JSON.stringify({
                uuid: item.id,
                name: item.name,
                extendInfo: {
                  pv: JSON.stringify(
                    pick(
                      find(item.targetPvcOptions, { value: item.targetPvc }),
                      [
                        'lunName',
                        'name',
                        'pvcName',
                        'size',
                        'storageUrl',
                        'volumeName'
                      ]
                    )
                  )
                }
              });
            }
          }
        ),
        targetObject:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProp?.uuid
            : this.formGroup.value.statefulset,
        extendInfo: {
          pre_script: this.formGroup.value.preScript || null,
          post_script: this.formGroup.value.postScript || null,
          failed_script: this.formGroup.value.executeScript || null,
          copyVerify: this.formGroup.value.copyVerify
        }
      };

      if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
        assign(params.extendInfo, {
          force_recovery: true
        });
      }
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
