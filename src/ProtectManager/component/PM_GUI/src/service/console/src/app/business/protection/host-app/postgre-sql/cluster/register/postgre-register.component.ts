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
  BaseUtilService,
  I18NService,
  DataMapService,
  CommonConsts,
  DataMap,
  ResourceType,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  MultiCluster,
  getMultiHostOps,
  Scene,
  Features,
  ClientManagerApiService
} from 'app/shared';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  isNumber,
  each,
  map,
  isEmpty,
  differenceBy,
  filter,
  includes,
  assign,
  set
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { distinctUntilChanged } from 'rxjs/operators';

@Component({
  selector: 'aui-postgre-register',
  templateUrl: './postgre-register.component.html',
  styleUrls: ['./postgre-register.component.less']
})
export class PostgreRegisterComponent implements OnInit {
  data;
  formGroup: FormGroup;
  dataMap = DataMap;
  isTest = false;
  proxyOptions = [];
  isSupport = true;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label'),
    invalidMaxLength: this.i18n.get('common_host_max_number_label', [3])
  };
  commonNodeLabel = this.i18n.get('protection_statefulset_node_label');
  clusterNodeLabel = this.i18n.get('protection_cluster_node_label');
  nodeLabel = this.commonNodeLabel;
  typeOptions = this.dataMapService
    .toArray('PostgreSql_Cluster_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  deployTypeOptions = this.dataMapService
    .toArray('PostgreSqlDeployType')
    .map(item => {
      item.isLeaf = true;
      return item;
    });

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.listenForm();
    this.updateData();
    this.getProxyOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  // 判断当前版本是否支持添加存储资源
  isSupportFunc(agent) {
    const params = {
      hostUuidsAndIps: agent,
      applicationType: 'PgSql',
      scene: Scene.Register,
      buttonNames: [Features.ClusterType]
    };
    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        this.isSupport = res?.ClusterType;
        if (res?.ClusterType) {
          this.formGroup
            .get('installDeployType')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('installDeployType').clearValidators();
        }
        this.formGroup.get('installDeployType').updateValueAndValidity();
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(
        {
          value: !isEmpty(this.data) ? this.data.name : '',
          disabled: !!this.data
        },
        {
          validators: [
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      type: new FormControl(!isEmpty(this.data) ? this.data.clusterType : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      installDeployType: new FormControl(
        !isEmpty(this.data)
          ? this.data?.installDeployType
          : DataMap.PostgreSqlDeployType.Pgpool.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      agents: new FormControl(
        !isEmpty(this.data) ? map(this.data.dependencies.agents, 'uuid') : [],
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(2),
            this.baseUtilService.VALID.maxLength(3)
          ]
        }
      ),
      virtual_ip: new FormControl(
        !isEmpty(this.data) ? this.data.endpoint : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ipv4()
          ]
        }
      ),
      clupServerNode: new FormControl([])
    });
  }

  listenForm() {
    this.formGroup.get('installDeployType').valueChanges.subscribe(res => {
      // CLup 需要改节点为集群节点 需要新增CLup Server节点
      if (res === DataMap.PostgreSqlDeployType.CLup.value) {
        this.nodeLabel = this.clusterNodeLabel;
        this.formGroup
          .get('clupServerNode')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.nodeLabel = this.commonNodeLabel;
        this.formGroup.get('clupServerNode').clearValidators();
      }
      this.formGroup.get('clupServerNode').updateValueAndValidity();
    });

    this.formGroup
      .get('agents')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        this.formGroup.get('installDeployType').setValue('');
        if (isEmpty(res)) {
          this.isSupport = true;
        } else {
          this.isSupportFunc(res);
        }
      });
  }

  updateData() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe((res: any) => {
        const data = {
          name: res.name,
          type: res.extendInfo?.clusterType,
          agents: map(res.dependencies.agents || [], 'uuid'),
          installDeployType:
            res.extendInfo?.installDeployType ||
            DataMap.PostgreSqlDeployType.Pgpool.value
        };
        if (
          data.installDeployType === DataMap.PostgreSqlDeployType.CLup.value
        ) {
          assign(data, {
            clupServerNode: map(res.dependencies.clupServers || [], 'uuid')
          });
        }
        this.formGroup.patchValue(data);
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.PostgreSQLInstance.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (isEmpty(this.data)) {
          if (MultiCluster.isMulti) {
            resource = getMultiHostOps(resource);
          } else {
            resource = filter(
              resource,
              item =>
                item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          }
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getParams() {
    let reduceAgents = [];
    if (this.data) {
      reduceAgents = differenceBy(
        this.data.dependencies.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    const params = {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      endpoint: this.formGroup.value.virtual_ip,
      subType: DataMap.Resource_Type.PostgreSQLCluster.value,
      extendInfo: {
        clusterType: this.formGroup.value.type,
        installDeployType:
          this.formGroup.value.installDeployType ||
          DataMap.PostgreSqlDeployType.Pgpool.value
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return { uuid: item };
        }),
        '-agents': !isEmpty(this.data)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
    if (
      this.formGroup.get('installDeployType').value ===
      DataMap.PostgreSqlDeployType.CLup.value
    ) {
      set(
        params,
        'dependencies.clupServers',
        map(this.formGroup.get('clupServerNode').value, item => ({
          uuid: item
        }))
      );
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.data) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.data.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
        return;
      }
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params
        })
        .subscribe({
          next: res => {
            cacheGuideResource(res);
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
