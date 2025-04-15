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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl } from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  EnvironmentsService,
  I18NService,
  ProtectResourceCategory,
  ResourceType
} from 'app/shared';
import { ProtectFilterComponent } from 'app/shared/components/protect-filter/protect-filter.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  find,
  includes,
  isArray,
  isEmpty,
  isNumber,
  map,
  reject
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced',
  templateUrl: './advanced.component.html',
  styleUrls: ['./advanced.component.less']
})
export class AdvancedComponent implements OnInit {
  resourceData;
  dataMap = DataMap;
  showProxyHost;
  virtualType;
  isVirtualMachine;
  selectedTitle;
  formGroup;
  valid$ = new Subject<boolean>();
  protectResourceCategory = ProtectResourceCategory;
  proxyHost = [];
  originHost = [];
  resourceType: string;
  concurrentErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 99999999])
  };
  extParams;
  isSingleHostClusterProtect = false;
  belongTag = [];

  isGroupRule = false;

  slaOverwriteHelp: string;
  slaApplyAllHelp: string;

  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;
  constructor(
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public environmentsApiService: EnvironmentsService,
    public message: MessageService,
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.showProxyHost && this.getProxyHost();
  }

  initData(data: any, type: any) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.isSingleHostClusterProtect =
      (!isArray(data) || (isArray(data) && data.length === 1)) &&
      includes(
        [ProtectResourceCategory.esix, ProtectResourceCategory.cluster],
        type
      );
    this.belongTag = [
      {
        label:
          type === ProtectResourceCategory.cluster
            ? this.i18n.get('protection_belong_cluster_label')
            : this.i18n.get('protection_belong_host_label')
      }
    ];
    this.virtualType =
      type === DataMap.Resource_Type.vmGroup.value
        ? ProtectResourceCategory.vmware
        : type;
    this.resourceType = this.resourceData.sub_type;
    if (type === DataMap.Resource_Type.vmGroup.value) {
      this.resourceType = type;
    }
    this.isVirtualMachine = [
      ProtectResourceCategory.vmware,
      ProtectResourceCategory.vmwares
    ].includes(this.virtualType);
    if (
      this.resourceData['resType'] === ResourceType.VM ||
      type === DataMap.Resource_Type.vmGroup.value
    ) {
      this.showProxyHost = true;
    }

    this.selectedTitle = this.i18n.get(
      this.virtualType === ProtectResourceCategory.cluster
        ? 'protection_cluster_label'
        : 'common_host_label'
    );

    this.isGroupRule =
      type === DataMap.Resource_Type.vmGroup.value &&
      this.resourceData.groupType === DataMap.vmGroupType.rule.value;

    if (this.isGroupRule) {
      this.slaOverwriteHelp = this.i18n.get(
        'protection_overwrite_policy_group_help_label'
      );
    } else if (type === ProtectResourceCategory.cluster) {
      this.slaOverwriteHelp = this.i18n.get(
        'protection_overwrite_policy_cluster_help_label'
      );
      this.slaApplyAllHelp = this.i18n.get('protection_cluster_sla_help_label');
    } else {
      this.slaOverwriteHelp = this.i18n.get(
        'protection_overwrite_policy_host_help_label'
      );
      this.slaApplyAllHelp = this.i18n.get('protection_host_sla_help_label');
    }
  }

  initForm() {
    let ext;
    // 虚拟机组和其他vm资源不同，ext_parameter多了一层protectObject，所以需要将其中的内容特取重新赋值
    if (
      this.resourceType === DataMap.Resource_Type.vmGroup.value &&
      !isEmpty(this.resourceData.protectedObject)
    ) {
      assign(this.resourceData, {
        ext_parameters: cloneDeep(
          this.resourceData.protectedObject.extParameters
        )
      });
    }
    if (!isEmpty(this.resourceData.ext_parameters)) {
      ext = cloneDeep(this.resourceData.ext_parameters);
      if (!isEmpty(ext.resource_filters)) {
        defer(() =>
          this.ProtectFilterComponent.setFilter(ext.resource_filters)
        );
      }
      if (!isEmpty(ext.resource_tag_filters)) {
        defer(() =>
          this.ProtectFilterComponent.setTagFilter(ext.resource_tag_filters)
        );
      }
    }
    this.extParams = ext;
    this.formGroup = this.fb.group({
      proxyHost: new FormControl([]),
      preScript: new FormControl(ext && ext.pre_script, {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.scriptName, false)
        ],
        updateOn: 'change'
      }),
      postScript: new FormControl(ext && ext.post_script, {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.scriptName, false)
        ],
        updateOn: 'change'
      }),
      concurrent: new FormControl(
        ext?.concurrent_requests && ext?.concurrent_requests !== '0'
          ? ext?.concurrent_requests
          : '',
        {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 99999999)
          ]
        }
      ),
      slaOverwrite: new FormControl(ext ? ext.overwrite : false),
      slaPolicy: new FormControl(
        ext ? ext.binding_policy : ['APPLY_TO_ALL', 'APPLY_TO_NEW']
      )
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(res === 'VALID');
    });
  }

  getVmwareAgents() {
    const conditions = {
      type: ResourceType.HOST,
      subType: [DataMap.Resource_Type.VMBackupAgent.value]
    };
    if (!this.resourceData.sla_id) {
      assign(conditions, {
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      });
    }
    const extParams = {
      uuid: this.resourceData.uuid,
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.clientManagerApiService.queryVMwareAgentListInfoUsingGET(params),
      resource => {
        // 修改过滤其它的离线主机
        const recordsTemp = reject(
          resource,
          item =>
            item.linkStatus !==
              DataMap.resource_LinkStatus_Special.normal.value &&
            this.resourceData.ext_parameters?.proxy_id !== item.uuid &&
            !includes(
              map(this.resourceData.ext_parameters?.host_list, 'proxy_id'),
              item.uuid
            )
        );
        const hostArr = [];
        each(recordsTemp, item => {
          hostArr.push({
            key: item.uuid,
            value: item.uuid,
            label: !isEmpty(item.endpoint)
              ? `${item.name}(${item.endpoint})`
              : item.name,
            link_status: Number(item.linkStatus),
            isLeaf: true,
            extendInfo: item.extendInfo
          });
        });
        hostArr.sort(a => {
          return a.extendInfo?.is_belongs_to_host_or_cluster === 'true'
            ? -1
            : 1;
        });
        this.proxyHost = hostArr;
        this.originHost = recordsTemp;
        if (this.resourceData?.ext_parameters?.host_list) {
          this.formGroup
            .get('proxyHost')
            .setValue(
              map(this.resourceData.ext_parameters.host_list, 'proxy_id')
            );
        }
        // 兼容升级场景
        if (
          this.resourceData?.ext_parameters?.proxy_id &&
          isEmpty(this.resourceData?.ext_parameters?.host_list)
        ) {
          this.formGroup
            .get('proxyHost')
            .setValue([this.resourceData.ext_parameters?.proxy_id]);
        }
      }
    );
  }

  getProxyHost(recordsTemp?, startPage?) {
    if (this.isSingleHostClusterProtect) {
      this.getVmwareAgents();
      return;
    }
    const conditions = {
      type: ResourceType.HOST,
      sub_type: DataMap.Resource_Type.VMBackupAgent.value
    };
    // 创建保护去掉离线主机
    if (!this.resourceData.sla_id) {
      assign(conditions, {
        link_status: [DataMap.resource_LinkStatus.normal.value]
      });
    }
    this.environmentsApiService
      .queryResourcesV1EnvironmentsGet({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify(conditions)
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.items];
        if (
          startPage === Math.ceil(res.total / CommonConsts.PAGE_SIZE) ||
          res.total === 0
        ) {
          // 修改过滤其它的离线主机
          recordsTemp = reject(
            recordsTemp,
            item =>
              item.link_status !== DataMap.resource_LinkStatus.normal.value &&
              this.resourceData.ext_parameters?.proxy_id !== item.uuid &&
              !includes(
                map(this.resourceData.ext_parameters?.host_list, 'proxy_id'),
                item.uuid
              )
          );
          const hostArr = [];
          each(recordsTemp, item => {
            hostArr.push({
              key: item.uuid,
              value: item.uuid,
              label: !isEmpty(item.endpoint)
                ? `${item.name}(${item.endpoint})`
                : item.name,
              link_status: item.link_status,
              isLeaf: true
            });
          });
          this.proxyHost = hostArr;
          this.originHost = recordsTemp;
          if (this.resourceData?.ext_parameters?.host_list) {
            this.formGroup
              .get('proxyHost')
              .setValue(
                map(this.resourceData.ext_parameters.host_list, 'proxy_id')
              );
          }
          // 兼容升级场景
          if (
            this.resourceData?.ext_parameters?.proxy_id &&
            isEmpty(this.resourceData?.ext_parameters?.host_list)
          ) {
            this.formGroup
              .get('proxyHost')
              .setValue([this.resourceData.ext_parameters?.proxy_id]);
          }
          return;
        }
        this.getProxyHost(recordsTemp, startPage);
      });
  }

  onOK() {
    const ext = {
      pre_script: this.formGroup.value.preScript,
      post_script: this.formGroup.value.postScript
    };
    if (this.showProxyHost) {
      assign(ext, {
        host_list: map(this.formGroup.value.proxyHost, item => {
          const host = find(this.originHost, { uuid: item });
          return {
            proxy_id: item || '',
            host: host?.endpoint || '',
            name: host?.name || '',
            port: host?.port || ''
          };
        })
      });
    }

    each(
      [
        'backup_res_auto_index',
        'archive_res_auto_index',
        'enable_security_archive'
      ],
      key => {
        if (this.formGroup.get(key)) {
          assign(ext, {
            [key]: this.formGroup.get(key).value
          });
        }
      }
    );

    if (!this.isVirtualMachine) {
      const vmFilters = this.ProtectFilterComponent.getAllFilters();
      const vmTagFilters = this.ProtectFilterComponent.getAllTagFilters();
      assign(ext, {
        resource_filters: !isEmpty(vmFilters) ? vmFilters : null,
        resource_tag_filters: !isEmpty(vmTagFilters) ? vmTagFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy,
        concurrent_requests: this.formGroup.value.concurrent
          ? `${this.formGroup.value.concurrent}`
          : '0',
        concurrent_requests_uuid: this.resourceData.uuid
      });
    }

    if (this.resourceType === DataMap.Resource_Type.vmGroup.value) {
      assign(ext, {
        concurrent_requests: this.formGroup.value.concurrent
          ? `${this.formGroup.value.concurrent}`
          : '0'
      });
    }

    if (this.isGroupRule) {
      assign(ext, {
        overwrite: this.formGroup.value.slaOverwrite
      });
    }
    return {
      ext_parameters: ext
    };
  }
}
