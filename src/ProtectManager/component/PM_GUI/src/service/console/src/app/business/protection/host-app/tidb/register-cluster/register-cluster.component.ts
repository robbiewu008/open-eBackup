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
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RouterUrl
} from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { defer, each, filter, isEmpty, size } from 'lodash';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  activeIndex = 0;
  nextBtnDisabled = true;
  isLoading = false;
  selectionPort = [];
  proxyOptions = [];
  proxyData = [];
  selectedHost = [];

  formGroup: FormGroup;
  nodeGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  nodeData;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  requiredLabel = this.i18n.get('common_required_label');
  requiredErrorTip = {
    required: this.requiredLabel
  };

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  agentTip = this.i18n.get('protection_tidb_register_cluster_proxy_info_label');

  @ViewChild('footerTpl', { static: true })
  footerTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  rowData;
  option;

  constructor(
    public modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig();
    this.getFooter();
    if (!this.rowData) {
      this.getProxyOptions();
    }
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.ProtectionHostAppHost);
  }

  initForm() {
    this.formGroup = this.fb.group({
      agent: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      name: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      path: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });
    this.nodeGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      location: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });
    if (this.rowData) {
      this.formGroup.get('name').setValue(this.rowData.name);
      this.formGroup.get('agent').setValue([this.rowData.extendInfo.tiupUuid]);
      this.formGroup.get('path').clearValidators();
      this.nodeGroup.get('name').setValue(this.rowData.auth.authKey);
      this.nodeGroup
        .get('location')
        .setValue(this.rowData.extendInfo.logBackupPath);
      this.getProxyOptions();
    }
    this.watchForm();
  }

  watchForm() {
    this.formGroup.statusChanges.subscribe(res => {
      defer(() => {
        this.disableBtn();
      });
    });
    this.nodeGroup.statusChanges.subscribe(res => {
      if (this.nodeGroup.valid && this.formGroup.valid) {
        this.hostChange();
      } else {
        this.nextBtnDisabled = true;
      }
    });
  }

  disableBtn() {
    if (this.formGroup.valid && this.activeIndex === 0) {
      this.nextBtnDisabled = false;
    } else {
      this.nextBtnDisabled = true;
    }
  }

  hostChange(e?) {
    let unchosen = filter(this.nodeData, item => {
      return !item.hostManagerIp;
    });
    if (unchosen.length !== 0) {
      this.nextBtnDisabled = true;
    } else if (this.nodeGroup.valid && this.formGroup.valid) {
      for (let item of this.nodeData) {
        if (item.status === 'up') {
          this.nextBtnDisabled = false;
        }
      }
    }
  }

  initConfig() {
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'cluster_name',
        columns: [
          {
            key: 'cluster_name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'version',
            name: this.i18n.get('common_version_label')
          }
        ],
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: this.rowData ? false : true,
          keepRadioLogic: true
        },
        colDisplayControl: false,
        selectionChange: data => {
          this.selectionPort = data;
          this.nextBtnDisabled = false;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.tidbCluster.value}Plugin`]
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
        if (MultiCluster.isMulti) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.endpoint,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
        this.proxyData = this.proxyOptions;
        if (this.rowData) {
          this.getTiup();
        }
      }
    );
  }

  getTiup() {
    let tmpConditions = {
      userName: 'admin',
      action_type: 'check_tiup',
      tiupPath: this.rowData
        ? this.rowData.extendInfo.tiupPath
        : this.formGroup.value.path,
      isCluster: true,
      agentIds: this.formGroup.value.agent
    };
    const params: any = {
      envId: this.formGroup.get('agent').value[0],
      pageSize: this.pageSize,
      pageNo: this.pageIndex,
      resourceSubType: this.dataMap.Resource_Type.tidbCluster.value,
      resourceType: this.dataMap.Resource_Type.tidbCluster.value,
      agentId: this.formGroup.get('agent').value[0],
      conditions: JSON.stringify(tmpConditions)
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!res.records.length) return;
        this.tableData = {
          data: this.rowData
            ? filter(
                JSON.parse(res.records[0]?.extendInfo?.cluster_list),
                item => {
                  return (
                    item.cluster_name === this.rowData.extendInfo.clusterName
                  );
                }
              )
            : JSON.parse(res.records[0]?.extendInfo?.cluster_list),
          total: size(res.records)
        };
        this.nextBtnDisabled = true;
        this.activeIndex++;
        this.formGroup.get('name').disable();
        if (this.rowData) {
          this.getNode();
        }
      });
  }

  getNode() {
    let test = {
      action_type: 'get_cluster_info',
      clusterName: this.rowData
        ? this.rowData.extendInfo.clusterName
        : this.selectionPort[0]?.cluster_name,
      tiupPath: this.rowData
        ? this.rowData.extendInfo.tiupPath
        : this.formGroup.value.path,
      isCluster: true,
      agentIds: this.formGroup.value.agent
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        pageSize: this.pageSize,
        pageNo: this.pageIndex,
        resourceType: this.dataMap.Resource_Type.tidbCluster.value,
        resourceSubType: this.dataMap.Resource_Type.tidbCluster.value,
        envId: this.formGroup.get('agent').value[0],
        agentId: this.formGroup.get('agent').value[0],
        conditions: JSON.stringify(test)
      })
      .subscribe(res => {
        this.nodeData = JSON.parse(res.records[0].extendInfo.cluster_info_list);
        if (this.rowData) {
          each(this.nodeData, item => {
            let tmp: any = filter(
              JSON.parse(this.rowData.extendInfo.clusterInfoList),
              temp => {
                return temp.id === item.id;
              }
            );
            item.hostManagerIp = tmp[0].hostManagerIp;
          });
          this.nextBtnDisabled = true;
        }
      });
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  previous() {
    if (this.activeIndex === 1) {
      this.formGroup.get('name').enable();
      this.dataTable.setSelections([]);
    }
    this.activeIndex--;
    this.nextBtnDisabled = false;
  }

  next() {
    if (this.activeIndex === 0) {
      this.getTiup();
    } else if (this.activeIndex === 1) {
      this.getNode();
      this.nextBtnDisabled = true;
      this.activeIndex++;
    }
  }

  finish() {
    this.isLoading = true;
    const params = this.getParams();
    if (this.rowData) {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.rowData.uuid,
          UpdateProtectedEnvironmentRequestBody: params
        })
        .subscribe({
          next: () => {
            this.option.refreshData();
            this.modal.close();
          },
          error: () => {
            this.isLoading = false;
            this.nextBtnDisabled = false;
          }
        });
    } else {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params
        })
        .subscribe({
          next: res => {
            cacheGuideResource(res);
            this.option.refreshData();
            this.modal.close();
          },
          error: () => {
            this.isLoading = false;
            this.nextBtnDisabled = false;
          }
        });
    }
  }

  getParams() {
    // 从代理主机集合中取出选中的代理主机
    const test1: any = filter(this.proxyOptions, item => {
      return this.formGroup.value.agent.includes(item.uuid);
    });
    this.nodeData.forEach(tmp => {
      let temp: any = filter(this.proxyOptions, item => {
        return item.endpoint === tmp.hostManagerIp;
      });
      tmp.hostManagerResourceUuid = temp[0].uuid;
    });
    const params: any = {
      name: this.formGroup.get('name').value,
      type: 'Database',
      subType: this.dataMap.Resource_Type.tidbCluster.value,
      extendInfo: {
        tiupUuid: this.formGroup.value.agent[0],
        clusterName: this.rowData
          ? this.rowData.extendInfo.clusterName
          : this.selectionPort[0].cluster_name,
        tiupPath: this.rowData
          ? this.rowData.extendInfo.tiupPath
          : this.formGroup.value.path,
        logBackupPath: this.nodeGroup.value.location,
        clusterInfoList: JSON.stringify(this.nodeData),
        version: this.rowData
          ? this.rowData.extendInfo.version
          : this.selectionPort[0].version,
        owner: this.rowData
          ? this.rowData.extendInfo.owner
          : this.selectionPort[0].owner,
        save_type: this.rowData ? 1 : 0
      },
      dependencies: {
        agents: test1.map(agent => {
          return {
            uuid: agent.uuid,
            endpoint: agent.endpoint
          };
        })
      },
      auth: {
        authType: 8,
        authKey: this.nodeGroup.get('name').value,
        authPwd: this.nodeGroup.get('password').value
      }
    };
    return params;
  }
}
