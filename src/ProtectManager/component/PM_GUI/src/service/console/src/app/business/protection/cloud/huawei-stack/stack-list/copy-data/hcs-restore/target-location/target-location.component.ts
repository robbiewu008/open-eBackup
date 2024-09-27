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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  ProtectedResourceApiService,
  ResourceType,
  RestoreV2LocationType,
  VmFileReplaceStrategy,
  I18NService,
  DataMap,
  CookieService,
  ClientManagerApiService,
  MultiCluster
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  eq,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  map,
  reject
} from 'lodash';

@Component({
  selector: 'aui-target-location',
  templateUrl: './target-location.component.html',
  styleUrls: ['./target-location.component.less']
})
export class TargetLocationComponent implements OnInit {
  rowCopy;
  params;
  position;
  existEnv;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  restoreToNewLocationOnly = false;
  dataMap = DataMap;
  resourceProp;

  targetCloudPlatformOptions;
  cloudPlatformTenantOptions;
  regionsOptions;
  projectsOptions;
  cloudHostOptions;

  originTargetCloudPlatformOptions;
  originCloudPlatformTenantOptions;
  originRegionsOptions;
  originProjectsOptions;
  originCloudHostOptions;
  verifyStatus;
  copyVerifyDisableLabel: string;
  isWorkspace = false;

  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');

  proxyOptions = [];

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private cookieService: CookieService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initDeployType();
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      (!this.existEnv && !this.isHcsUser);

    this.initForm();
    if (this.isHcsUser) {
      this.getHcsCloudHost();
    } else {
      if (
        this.formGroup.value.restoreLocation === this.restoreLocationType.ORIGIN
      ) {
        this.getCloudHost();
      } else {
        this.getTargetCloudPlatform();
      }
    }
    this.getProxyOptions();
    this.initCopyVerifyDisableLabel();
  }
  initCopyVerifyDisableLabel() {
    if (eq(this.verifyStatus, this.CopyDataVerifyStatus.noGenerate.value)) {
      this.copyVerifyDisableLabel = 'common_generate_verify_file_disable_label';
    }
    if (eq(this.verifyStatus, this.CopyDataVerifyStatus.Invalid.value)) {
      this.copyVerifyDisableLabel = 'common_invalid_verify_file_disable_label';
    }
  }

  initDeployType() {
    const resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.isHcsUser =
      this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE &&
      resourceProperties.environment_sub_type ===
        DataMap.Job_Target_Type.hcsEnvOp.value;
  }

  initForm() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.isWorkspace = this.resourceProp.extendInfo?.isWorkspace === '1';
    const originData = this.resourceProp.path.split('/');
    if (!isEmpty(this.params?.newPosition)) {
      this.targetCloudPlatformOptions = this.params.newPosition?.targetCloudPlatformOptions;
      this.cloudPlatformTenantOptions = this.params.newPosition?.cloudPlatformTenantOptions;
      this.regionsOptions = this.params.newPosition?.regionsOptions;
      this.projectsOptions = this.params.newPosition?.projectsOptions;
      this.cloudHostOptions = this.params.newPosition?.cloudHostOptions;
    }
    defer(() => {
      this.originTargetCloudPlatformOptions = [
        {
          value: originData[0],
          label: originData[0],
          isLeaf: true
        }
      ];
      this.originCloudPlatformTenantOptions = [
        {
          value: originData[1],
          label: originData[1],
          isLeaf: true
        }
      ];
      this.originRegionsOptions = [
        {
          value: originData[2],
          label: originData[2],
          isLeaf: true
        }
      ];
      this.originProjectsOptions = [
        {
          value: originData[3],
          label: originData[3],
          isLeaf: true
        }
      ];
      this.originCloudHostOptions = [
        {
          value: this.resourceProp.uuid,
          label: this.resourceProp.name,
          isLeaf: true
        }
      ];
    });

    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(this.position),
      originTargetCloudPlatform: new FormControl(originData[0]),
      originCloudPlatformTenant: new FormControl(originData[1]),
      originRegions: new FormControl(originData[2]),
      originProjects: new FormControl(originData[3]),
      originCloudHost: new FormControl(this.resourceProp.uuid),

      targetCloudPlatform: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.targetCloudPlatform?.uuid
          : ''
      ),
      cloudPlatformTenant: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.cloudPlatformTenant?.uuid
          : ''
      ),
      regions: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.regions?.uuid
          : ''
      ),
      projects: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.projects?.uuid
          : ''
      ),
      cloudHost: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.cloudHost?.uuid
          : ''
      ),
      proxyHost: new FormControl(
        !isEmpty(this.params) ? this.params?.agents : []
      ),
      restoreAutoPowerOn: new FormControl(
        isEmpty(this.params) ? false : this.params?.restoreAutoPowerOn === '1'
      ),
      copyVerify: new FormControl(
        isEmpty(this.params) ? false : this.params?.copyVerify === 'true'
      )
    });

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        if (!this.isHcsUser) {
          this.formGroup.get('targetCloudPlatform').clearValidators();
          this.formGroup.get('cloudPlatformTenant').clearValidators();
          this.formGroup.get('regions').clearValidators();
          this.formGroup.get('projects').clearValidators();
          this.originCloudHostOptions = [
            {
              value: this.resourceProp.uuid,
              label: this.resourceProp.name,
              isLeaf: true
            }
          ];
        }
        this.formGroup.get('cloudHost').clearValidators();
        this.formGroup.get('originCloudHost').setValue(this.resourceProp.uuid);
        this.formGroup.get('originCloudHost').updateValueAndValidity();

        this.getCloudHost();
      } else {
        this.formGroup
          .get('cloudHost')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (!this.isHcsUser) {
          this.formGroup
            .get('targetCloudPlatform')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('cloudPlatformTenant')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('regions')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('projects')
            .setValidators([this.baseUtilService.VALID.required()]);
        }

        this.getTargetCloudPlatform();
      }
      this.formGroup
        .get('cloudHost')
        .updateValueAndValidity({ emitEvent: false });
      if (!this.isHcsUser) {
        this.formGroup
          .get('targetCloudPlatform')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('cloudPlatformTenant')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('regions')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('projects')
          .updateValueAndValidity({ emitEvent: false });
      }
    });

    this.formGroup.get('targetCloudPlatform').valueChanges.subscribe(res => {
      this.formGroup
        .get('cloudPlatformTenant')
        .setValue('', { emitEvent: false });
      this.formGroup.get('regions').setValue('', { emitEvent: false });
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.cloudPlatformTenantOptions = [];
      this.regionsOptions = [];
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getCloudPlatform());
    });

    this.formGroup.get('cloudPlatformTenant').valueChanges.subscribe(res => {
      this.formGroup.get('regions').setValue('', { emitEvent: false });
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.regionsOptions = [];
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getRegion());
    });

    this.formGroup.get('regions').valueChanges.subscribe(res => {
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getProject());
    });

    this.formGroup.get('projects').valueChanges.subscribe(res => {
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.cloudHostOptions = [];
      defer(() => this.getCloudHost());
    });

    if (this.restoreToNewLocationOnly) {
      defer(() => {
        this.formGroup.patchValue({
          restoreLocation: RestoreV2LocationType.NEW
        });
      });
    }
  }
  getTargetCloudPlatform(recordsTemp?: any[], startPage?: number) {
    if (this.isHcsUser) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: ResourceType.HCS_CONTAINER
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
          this.targetCloudPlatformOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getTargetCloudPlatform(recordsTemp, startPage);
      });
  }
  getCloudPlatform(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          visible: '1',
          parentUuid: this.formGroup.value.targetCloudPlatform,
          subType: DataMap.Resource_Type.HCSTenant.value
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
          this.cloudPlatformTenantOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getCloudPlatform(recordsTemp, startPage);
      });
  }

  getRegion(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          parentUuid: this.formGroup.value.cloudPlatformTenant,
          subType: DataMap.Resource_Type.HCSRegion.value
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
          this.regionsOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getRegion(recordsTemp, startPage);
      });
  }

  getProject(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          parentUuid: this.formGroup.value.regions,
          subType: DataMap.Resource_Type.HCSProject.value
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
          this.projectsOptions = map(recordsTemp, item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getProject(recordsTemp, startPage);
      });
  }

  getHcsCloudHost() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'CloudHost',
        path: [
          ['~~'],
          decodeURI(
            get(window, 'parent.hcsData.ProjectName', '') ||
              this.cookieService.get('projectName')
          )
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.originCloudHostOptions = map(
          filter(resource, item => item.uuid === this.resourceProp.uuid),
          item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          }
        );
        this.cloudHostOptions = map(
          this.restoreToNewLocationOnly
            ? resource
            : reject(resource, v => v.uuid === this.resourceProp.uuid),
          item => {
            return assign(item, {
              value: item.uuid,
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          }
        );
      }
    );
  }

  getCloudHost(recordsTemp?: any[], startPage?: number) {
    if (this.isHcsUser) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.HCSCloudHost.value,
          parentUuid:
            this.formGroup.value.restoreLocation ===
            this.restoreLocationType.ORIGIN
              ? this.resourceProp.parent_uuid
              : this.formGroup.value.projects
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
          // 软删除不能作为恢复目标位置
          recordsTemp = reject(
            recordsTemp,
            item =>
              item.extendInfo?.status ===
              DataMap.HCS_Host_LinkStatus.softDelete.value
          );
          if (
            !find(recordsTemp, item => item.uuid === this.resourceProp.uuid) &&
            this.formGroup.value.originCloudHost === this.resourceProp.uuid
          ) {
            this.formGroup.get('originCloudHost').setValue('');
          }
          if (
            this.formGroup.value.restoreLocation ===
            this.restoreLocationType.ORIGIN
          ) {
            this.originCloudHostOptions = map(recordsTemp, item => {
              return assign(item, {
                value: item.uuid,
                key: item.uuid,
                label: item.name,
                isLeaf: true
              });
            });
          } else {
            this.cloudHostOptions = map(
              this.restoreToNewLocationOnly
                ? recordsTemp
                : reject(recordsTemp, v => v.uuid === this.resourceProp.uuid),
              item => {
                return assign(item, {
                  value: item.uuid,
                  key: item.uuid,
                  label: item.name,
                  isLeaf: true
                });
              }
            );
          }
          return;
        }
        this.getCloudHost(recordsTemp, startPage);
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `HCScontainerPlugin`,
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        // 多集群场景下HCS的注册 保护 恢复场景 过滤内置代理主机
        if (MultiCluster.isMulti) {
          resource = reject(
            resource,
            item =>
              item.extendInfo.scenario === DataMap.proxyHostType.builtin.value
          );
        }
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getTargetParams() {
    const item =
      this.formGroup.value.restoreLocation === this.restoreLocationType.ORIGIN
        ? find(this.originCloudHostOptions, {
            uuid: this.formGroup.value.originCloudHost
          })
        : find(this.cloudHostOptions, {
            uuid: this.formGroup.value.cloudHost
          });

    const params = {
      cloudHost: {
        ...item
      },
      agents: this.formGroup.value.proxyHost,
      restoreAutoPowerOn: this.formGroup.value.restoreAutoPowerOn ? '1' : '0',
      copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false'
    };

    if (this.formGroup.value.restoreLocation === this.restoreLocationType.NEW) {
      params['newPosition'] = {
        targetCloudPlatformOptions: this.targetCloudPlatformOptions,
        cloudPlatformTenantOptions: this.cloudPlatformTenantOptions,
        regionsOptions: this.regionsOptions,
        projectsOptions: this.projectsOptions,
        cloudHostOptions: this.cloudHostOptions,
        targetCloudPlatform: find(this.targetCloudPlatformOptions, {
          uuid: this.formGroup.value.targetCloudPlatform
        }),
        cloudPlatformTenant: find(this.cloudPlatformTenantOptions, {
          uuid: this.formGroup.value.cloudPlatformTenant
        }),
        regions: find(this.regionsOptions, {
          uuid: this.formGroup.value.regions
        }),
        projects: find(this.projectsOptions, {
          uuid: this.formGroup.value.projects
        }),
        cloudHost: find(this.cloudHostOptions, {
          uuid: this.formGroup.value.cloudHost
        })
      };
    }
    return params;
  }
}
