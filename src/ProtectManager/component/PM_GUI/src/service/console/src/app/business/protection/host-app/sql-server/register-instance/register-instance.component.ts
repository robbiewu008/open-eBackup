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
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  find,
  includes,
  isNumber,
  map,
  size,
  split,
  isEmpty
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-register-instance',
  templateUrl: './register-instance.component.html',
  styleUrls: ['./register-instance.component.less']
})
export class RegisterInstanceComponent implements OnInit {
  item;
  isSingle = true;
  optsConfig;
  isDBAuthMode = false;
  optItems = [];
  agentOptions = [];
  nodeOptions = [];
  clusterOptions = [];
  dataMap = DataMap;
  isMulti = MultiCluster.isMulti;
  authOptions = this.dataMapService.toArray('Database_Auth_Method');
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  isTest = true;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  @Input() getInstance;
  @Input() isModified;
  @Input() rowData;
  constructor(
    private modal: ModalRef,
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getFooter();
    this.getProxyOptions();
    this.getclusterOptions();
    each(this.authOptions, item => {
      assign(item, {
        isLeaf: true
      });
    });
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  showClusterGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl(
        {
          value: 'MSSQLSERVER',
          disabled: !!this.rowData
        },
        {
          validators: [
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      networkName: new FormControl(
        {
          value: '',
          disabled: !!this.rowData
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nameCombination,
              false
            ),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      agent: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      node: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      authMode: new FormControl(DataMap.Database_Auth_Method.os.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      dbUserName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      dbPassword: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.listenForm();

    if (this.rowData) {
      if (
        this.rowData.subType === DataMap.Resource_Type.SQLServerInstance.value
      ) {
        this.formGroup.patchValue({
          type: DataMap.Instance_Type.single.value,
          name: this.rowData.name,
          networkName: this.rowData.extendInfo?.networkName,
          agent: this.rowData.extendInfo?.hostId,
          authType:
            this.rowData.authType === 1
              ? DataMap.Database_Auth_Method.os.value
              : DataMap.Database_Auth_Method.db.value
        });
      } else {
        this.formGroup.patchValue({
          type: DataMap.Instance_Type.cluster.value,
          name: this.rowData.name,
          networkName: this.rowData.extendInfo?.networkName,
          cluster: this.rowData.parentUuid,
          node: map(
            JSON.parse(this.rowData.extendInfo?.agents),
            item => item.uuid
          ),
          authType:
            this.rowData.authType === 1
              ? DataMap.Database_Auth_Method.os.value
              : DataMap.Database_Auth_Method.db.value
        });
      }
    } else {
      this.formGroup.get('type').setValue(DataMap.Instance_Type.single.value);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.formGroup.patchValue({
        name: 'MSSQLSERVER',
        agent: '',
        cluster: '',
        node: [],
        authMode: DataMap.Database_Auth_Method.os.value,
        dbUserName: '',
        dbPassword: ''
      });

      if (res === DataMap.Instance_Type.cluster.value) {
        this.isSingle = false;
        this.formGroup.get('cluster').enable();
        this.formGroup.get('node').enable();
        this.formGroup.get('agent').disable();
      } else {
        this.isSingle = true;
        this.formGroup.get('cluster').disable();
        this.formGroup.get('node').disable();
        this.formGroup.get('agent').enable();
      }
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (!res || !size(this.agentOptions) || !size(this.clusterOptions)) {
        return;
      }

      this.formGroup.patchValue({
        node: []
      });
      const cluster = find(this.clusterOptions, item => item.uuid === res);
      this.nodeOptions = [
        ...filter(this.agentOptions, item =>
          includes([...split(cluster?.endpoint, ',')], item.endpoint)
        )
      ];
    });

    this.formGroup.get('authMode').valueChanges.subscribe(res => {
      if (res === DataMap.Database_Auth_Method.db.value) {
        this.isDBAuthMode = true;
        this.formGroup.get('dbUserName').enable();
        this.formGroup.get('dbPassword').enable();
      } else {
        this.isDBAuthMode = false;
        this.formGroup.get('dbUserName').disable();
        this.formGroup.get('dbPassword').disable();
      }
    });

    this.formGroup.get('node').valueChanges.subscribe(res => {
      if (size(res) >= 2) {
        each(this.nodeOptions, item => {
          if (!includes(res, item.uuid)) {
            assign(item, {
              disabled: true
            });
          }
        });
      } else {
        each(this.nodeOptions, item => {
          assign(item, {
            disabled: false
          });
        });
      }

      this.nodeOptions = [...this.nodeOptions];
    });
  }
  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      value: item.uuid,
      label: `${item.name}(${item.endpoint})`,
      isLeaf: true
    });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.SQLServerInstance.value}Plugin`]
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
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.osType === DataMap.Os_Type.windows.value &&
            includes(['1', '2', '3', '4'], tmp.linkStatus)
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
        this.agentOptions = hostArray;

        if (this.rowData) {
          const cluster = find(
            this.clusterOptions,
            item => item.uuid === this.rowData.parentUuid
          );
          this.nodeOptions = filter(this.agentOptions, item =>
            includes([...split(cluster?.endpoint, ',')], item?.endpoint)
          );
        }
      }
    );
  }

  getclusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.SQLServerCluster.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.clusterOptions = clusterArray;

        if (this.rowData) {
          const cluster = find(
            this.clusterOptions,
            item => item.uuid === this.rowData.parentUuid
          );
          this.nodeOptions = filter(this.agentOptions, item =>
            includes([...split(cluster?.endpoint, ',')], item?.endpoint)
          );
        }
        return;
      }
      this.getclusterOptions(recordsTemp, startPage);
    });
  }

  getParams() {
    return this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.SQLServerInstance.value,
          parentUuid: this.formGroup.value.agent,
          rootUuid: this.formGroup.value.agent,
          extendInfo: {
            hostId: this.formGroup.value.agent,
            networkName: this.formGroup.getRawValue().networkName
          },
          auth: {
            authType:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.os.value
                ? 1
                : 2,
            authKey: this.formGroup.value.dbUserName,
            authPwd: this.formGroup.value.dbPassword
          },
          dependencies: {
            agents: [{ uuid: this.formGroup.value.agent }]
          }
        }
      : {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.SQLServerClusterInstance.value,
          parentUuid: this.formGroup.value.cluster,
          extendInfo: {
            networkName: this.formGroup.getRawValue().networkName
          },
          auth: {
            authType:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.os.value
                ? 1
                : 2,
            authKey: this.formGroup.value.dbUserName,
            authPwd: this.formGroup.value.dbPassword
          },
          dependencies: {
            agents: map(this.formGroup.value.node, item => {
              return {
                uuid: item
              };
            })
          }
        };
  }

  onOK() {
    const params = this.getParams();
    if (this.rowData) {
      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.rowData.uuid,
          UpdateResourceRequestBody: params
        })
        .subscribe(res => {
          this.getInstance();
          this.modal.close();
        });
    } else {
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody: params as any
        })
        .subscribe(res => {
          cacheGuideResource(res);
          this.getInstance();
          this.modal.close();
        });
    }
  }

  test() {
    const params = this.getParams();

    this.protectedResourceApiService
      .CheckResource({
        checkResourceRequestBody: params as any
      })
      .subscribe(res => {
        this.isTest = true;
      });
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
