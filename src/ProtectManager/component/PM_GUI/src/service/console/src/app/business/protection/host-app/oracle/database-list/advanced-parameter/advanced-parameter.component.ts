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
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  PolicyAction,
  ProtectedResourceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  filter,
  find,
  includes,
  isArray,
  isEmpty,
  isNil,
  map,
  some,
  toNumber,
  trim
} from 'lodash';
import { Subject } from 'rxjs';
import { distinctUntilChanged } from 'rxjs/operators';

@Component({
  selector: 'aui-oracle-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  _isEmpty = isEmpty;
  osType;
  resourceData;
  resourceType;
  scriptTooltip;
  scriptPlaceholder;
  dataMap = DataMap;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  showProxyHost = false;
  isRequiredProxy = false;
  isWindows = false;
  isDetail = false; // 创建还是查看保护详情
  hiddenStorage = false; // 隐藏存储快照按钮
  unsupportStorage = false;
  protectedExtparams;
  protectedAgentsIpArr;
  hostOptions = [];
  newHostOptions = [];
  originalHostOptions = [];
  dateUnitOptions = this.dataMapService.toArray('deleteArchiveLogDuringUnit');
  unsupportedLabel = this.i18n.get(
    'protection_oracle_snapshot_storage_diff_label'
  );
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  proxyTipsLabel = this.i18n.get('protection_oracle_proxy_host_tip_label');
  scriptErrorTip = {
    ...this.baseUtilService.scriptNameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  concurrencyErrorTip = {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 8]),
    invalidInteger: this.i18n.get('common_valid_integer_label')
  };
  storageUsageErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 100]),
    invalidInteger: this.i18n.get('common_valid_integer_label')
  };
  deleteBeforeTimeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999])
  };
  deleteArchiveLogDisplayLabel = this.i18n.get(
    'protection_delete_all_archived_log_label'
  );
  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService,
    private appUtilsService: AppUtilsService,
    private globalService: GlobalService,
    private infoMessageService: InfoMessageService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.getCopyInfo();
    this.initForm();
    this.getProxyOptions();
    this.getHostOptions();
  }

  initDetailData(data) {
    // 如果定义了同名函数，会在resource-detail中被调用
    this.resourceData = data;
    this.isDetail = true;
  }

  getCopyInfo() {
    if (isArray(this.resourceData)) {
      this.osType = this.resourceData[0].environment?.osType;
    } else {
      this.osType = this.resourceData.environment?.osType;
      this.showProxyHost =
        this.resourceData.subType === DataMap.Resource_Type.oracleCluster.value;
    }
    this.isWindows = this.osType === DataMap.Os_Type.windows.value;
    this.hiddenStorage =
      this.osType === DataMap.Os_Type.aix.value ||
      this.appUtilsService.isHcsUser;
    this.scriptPlaceholder = this.i18n.get(
      this.isWindows
        ? 'common_script_windows_placeholder_label'
        : 'common_script_linux_placeholder_label'
    );
    this.scriptTooltip = this.i18n.get(
      this.isWindows
        ? 'common_script_agent_windows_position_label'
        : 'common_script_oracle_linux_help_label'
    );
    if (
      this.resourceData.subType === DataMap.Resource_Type.oracleCluster.value
    ) {
      this.unsupportedLabel = this.i18n.get(
        'protection_oracle_snapshot_storage_diff_replicate_label'
      );
    }
  }

  getProxyOptions() {
    if (!this.showProxyHost) {
      return;
    }
    return this.protectedResourceApiService
      .ShowResource({
        resourceId:
          this.resourceData.environment?.uuid || this.resourceData.rootUuid
      })
      .subscribe((res: any) => {
        this.hostOptions = map(res.dependencies?.agents, item => {
          return assign(item, {
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.formatIdToIp(
          this.formGroup.get('proxyHost').value,
          this.hostOptions
        );
        this.originalHostOptions = [...this.hostOptions];
      });
  }

  getHostOptions() {
    const extParams = {
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.oracle.value}Plugin`,
        linkStatus: [String(DataMap.resource_LinkStatus.normal.value)]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.clientManagerApiService.queryAgentListInfoUsingGET(param),
      resource => {
        const filterResource = filter(
          resource,
          item => item.osType === DataMap.Os_Type.linux.value
        );
        this.newHostOptions = map(filterResource, item => ({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.endpoint})`,
          isLeaf: true
        }));
        this.formatIdToIp(
          this.formGroup.get('proxyHost').value,
          this.newHostOptions
        );
        if (!!this.resourceData && this.formGroup.value.storage_snapshot_flag) {
          // 修改场景
          this.hostOptions = [...this.newHostOptions];
        }
      }
    );
  }

  initForm() {
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    this.globalService.getState('slaObjectUpdated').subscribe(res => {
      // 进入高级参数组件是无法获取到slaList的，因为slaObject是异步更新的
      // 查询完sla后select-sla会发送更新消息，然后这里就能取到所有的slaList
      if (res) {
        this.unsupportStorage = some(
          this.resourceData?.slaObject?.policy_list,
          item => item.action === PolicyAction.DIFFERENCE
        );
      }
    });
    this.protectedExtparams = extParameters;
    const getFormControl = (name: string, validators = []) => {
      return new FormControl(extParameters[name] || '', {
        validators: [
          ...validators,
          this.baseUtilService.VALID.maxLength(8192),
          this.isWindows
            ? this.baseUtilService.VALID.name(
                CommonConsts.REGEX.windowsScript,
                false
              )
            : this.baseUtilService.VALID.name(
                CommonConsts.REGEX.linuxScript,
                false
              )
        ],
        updateOn: 'change'
      });
    };
    this.formGroup = this.fb.group({
      delete_archived_log: new FormControl(
        extParameters.delete_archived_log ?? false
      ),
      delete_archived_log_group: new FormControl(0),
      custom_delete_log_time: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.rangeValue(1, 999),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.required()
        ]
      }),
      custom_delete_log_time_unit: new FormControl(
        this.dateUnitOptions[0].value
      ),
      proxyHost: new FormControl(
        !isEmpty(extParameters?.agents) ? extParameters?.agents.split(';') : []
      ),
      pre_script: getFormControl('pre_script'),
      post_script: getFormControl('post_script'),
      failed_script: getFormControl('failed_script'),
      storage_snapshot_flag: new FormControl(
        extParameters?.storage_snapshot_flag || false
      ),
      concurrency: new FormControl(3),
      max_storage_usage_ratio: new FormControl(80)
    });
    this.listenForm();
    if (
      !isNil(extParameters?.delete_before_time) &&
      extParameters.delete_before_time !== '0'
    ) {
      // 有值且不为零说明是开启了自定义删除时间
      this.formGroup.get('delete_archived_log_group').setValue(1); // radio 跳到自定义选项
      this.formGroup
        .get('custom_delete_log_time')
        .setValue(extParameters.delete_before_time); // 填充自定义
      this.formGroup
        .get('custom_delete_log_time_unit')
        .setValue(extParameters.delete_before_time_unit);
      this.deleteArchiveLogDisplayLabel = this.i18n.get(
        'protection_delete_archived_log_after_log_backup_display_label',
        [
          this.formGroup.get('custom_delete_log_time').value,
          this.dataMapService.getLabel(
            'deleteArchiveLogDuringUnit',
            this.formGroup.get('custom_delete_log_time_unit').value
          )
        ]
      );
    }
    // 存储快照开启时使用的是内置外置代理主机
    this.hostOptions = extParameters?.storage_snapshot_flag
      ? [...this.newHostOptions]
      : [...this.originalHostOptions];
    // 开启存储快照后使用的是snapshot_agents，此时agents字段为null
    if (extParameters?.storage_snapshot_flag) {
      this.formGroup.patchValue({
        proxyHost: !isEmpty(extParameters?.snapshot_agents)
          ? extParameters?.snapshot_agents.split(';')
          : [],
        concurrency: toNumber(extParameters.concurrent_requests || '3'),
        max_storage_usage_ratio: toNumber(
          extParameters?.max_storage_usage_ratio || '80'
        )
      });
      this.afterStorageSnapshotFlagChange(true);
      this.proxyTipsLabel = this.i18n.get(
        'protection_snapshot_backup_tips_label'
      );
      // windows主机在开启备份存储快照后 代理主机为必填
      if (this.isWindows) {
        this.isRequiredProxy = true;
        this.formGroup
          .get('proxyHost')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('proxyHost').updateValueAndValidity();
      }
    }
  }

  listenForm() {
    this.formGroup
      .get('storage_snapshot_flag')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        this.beforeStorageFlagChange(res);
      });
    this.formGroup
      .get('delete_archived_log_group')
      .valueChanges.subscribe(res => {
        if (!res) {
          this.formGroup.get('custom_delete_log_time').disable();
          this.formGroup.get('custom_delete_log_time_unit').disable();
        } else {
          this.formGroup.get('custom_delete_log_time').enable();
          this.formGroup.get('custom_delete_log_time_unit').enable();
        }
      });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  /**
   * 开启存储快照开关时需要弹出提示
   * 点击了确认才能执行开启逻辑，取消或关闭窗口都会执行关闭逻辑
   */
  beforeStorageFlagChange(res) {
    if (res) {
      this.infoMessageService.create({
        content: this.i18n.get(
          'protection_oracle_snapshot_storage_instance_confirm_tips_label'
        ),
        onOK: () => {
          this.storageFlagChange(true);
        },
        onCancel: () => {
          this.storageFlagChange(false);
          this.formGroup.get('storage_snapshot_flag').setValue(false);
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            this.formGroup.get('storage_snapshot_flag').setValue(false);
          }
        }
      });
    } else {
      this.storageFlagChange(false);
    }
  }

  storageFlagChange(res) {
    const proxyControl = this.formGroup.get('proxyHost');
    this.proxyTipsLabel = this.i18n.get(
      res
        ? 'protection_snapshot_backup_tips_label'
        : 'protection_oracle_proxy_host_tip_label'
    );
    // 开启存储快照，可以同时选择内置+外置代理
    this.hostOptions = res
      ? [...this.newHostOptions]
      : [...this.originalHostOptions];
    this.isRequiredProxy = this.isWindows && res;
    this.isRequiredProxy
      ? proxyControl.setValidators([this.baseUtilService.VALID.required()])
      : proxyControl.clearValidators();
    proxyControl.updateValueAndValidity();
    // 开启存储快照需要填并发数
    this.afterStorageSnapshotFlagChange(res);
  }

  private afterStorageSnapshotFlagChange(res) {
    if (res) {
      this.formGroup
        .get('concurrency')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ]);
      this.formGroup
        .get('max_storage_usage_ratio')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(0, 100)
        ]);
    } else {
      this.formGroup.get('max_storage_usage_ratio').clearValidators();
      this.formGroup.get('concurrency').clearValidators();
    }
    this.formGroup.get('concurrency').updateValueAndValidity();
    this.formGroup.get('max_storage_usage_ratio').updateValueAndValidity();
  }

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
  }

  formatIdToIp(agentArr, hostOptions) {
    // 选中的代理主机返回的是id,界面需要展示的是ip
    this.protectedAgentsIpArr = map(agentArr, item => {
      const target = find(hostOptions, { uuid: item });
      if (!!target) {
        // 如果保护时在线的代理主机，修改时离线或者不存则无法回显展示
        // 所以这里只返回找得到的代理主机
        return target?.endpoint;
      }
    });
  }

  onOK() {
    const resourceData = isArray(this.resourceData)
      ? this.resourceData[0]
      : this.resourceData;
    const agentParams = {
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.hostOptions, 'value'), item);
          })
          .join(';') || null,
      delete_archived_log: this.formGroup.value.delete_archived_log,
      storage_snapshot_flag: this.formGroup.value.storage_snapshot_flag
    };
    if (agentParams.delete_archived_log) {
      if (this.formGroup.get('delete_archived_log_group').value) {
        assign(agentParams, {
          delete_before_time_unit: this.formGroup.value
            .custom_delete_log_time_unit,
          delete_before_time: this.formGroup.value.custom_delete_log_time
        });
      } else {
        assign(agentParams, {
          delete_before_time_unit:
            DataMap.deleteArchiveLogDuringUnit.hour.value,
          delete_before_time: '0'
        });
      }
    }
    // 开启快照备份后原来的agents改为snapshot_agents agents字段传null
    if (this.formGroup.value.storage_snapshot_flag) {
      assign(agentParams, {
        snapshot_agents:
          this.formGroup.value.proxyHost
            ?.filter(item => {
              return includes(map(this.hostOptions, 'value'), item);
            })
            .join(';') || null,
        agents: null,
        concurrent_requests: String(this.formGroup.value.concurrency),
        max_storage_usage_ratio: String(
          this.formGroup.value.max_storage_usage_ratio
        )
      });
    }
    return assign(resourceData, {
      ext_parameters:
        isEmpty(trim(this.formGroup.value.pre_script)) &&
        isEmpty(trim(this.formGroup.value.post_script)) &&
        isEmpty(trim(this.formGroup.value.failed_script))
          ? { ...agentParams }
          : {
              ...agentParams,
              pre_script: trim(this.formGroup.value.pre_script),
              post_script: trim(this.formGroup.value.post_script),
              failed_script: trim(this.formGroup.value.failed_script)
            }
    });
  }
}
