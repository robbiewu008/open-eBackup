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
  FormGroup,
  FormBuilder,
  FormControl,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  DataMapService,
  I18NService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  ResourceType,
  KerberosAPIService,
  GlobalService
} from 'app/shared';
import { MessageService, ModalRef } from '@iux/live';
import {
  each,
  isNumber,
  map,
  isUndefined,
  isFunction,
  trim,
  isEmpty,
  uniq,
  find,
  isNil,
  get,
  filter
} from 'lodash';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
@Component({
  selector: 'app-add-node',
  templateUrl: './add-node.component.html',
  styleUrls: ['./add-node.component.less']
})
export class AddNodeComponent implements OnInit {
  addItem: any;
  okLoading = false;
  hostsOptions = []; // 主机名称
  formGroup: FormGroup;
  kerberosOptions = []; // kerberos选项数据
  tableData: TableData;
  data; // 修改时用的
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  // 路径校验
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label'),
    pathError: this.i18n.get('common_path_error_label')
  };

  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  // 认证方式
  authMethodOptions = this.dataMapService
    .toArray('clickHouse_Auth_Method_Type')
    .filter(v => (v.isLeaf = true));

  keberosOptions: any; // keberos认证方式
  dataMap = DataMap;

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private appUtilsService: AppUtilsService,
    private modal: ModalRef,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private kerberosApi: KerberosAPIService,
    private globalService: GlobalService,
    public message: MessageService
  ) {}

  ngOnInit() {
    this.getFooter();
    this.initForm();
    this.getHostOptions();
    this.getKerberos();
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        environment: {
          osType: [['=='], 'linux']
        },
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.ClickHouse.value}Plugin`]
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
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            port: tmp.port,
            endpoint: tmp.endpoint,
            isLeaf: true
          });
        });
        this.hostsOptions = hostArray;
      }
    );
  }

  getKerberos(callback?) {
    this.kerberosApi
      .queryAllKerberosUsingGET({
        startPage: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2]
      })
      .subscribe(res => {
        this.kerberosOptions = map(res.items, item => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
        if (!isUndefined(callback) && isFunction(callback)) {
          callback();
        }
      });
  }
  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  initForm() {
    this.formGroup = this.fb.group({
      client: new FormControl(!isEmpty(this.data) ? this.data.clientPath : '', {
        validators: [this.baseUtilService.VALID.required(), this.validPath()]
      }),

      business_ip: new FormControl(!isEmpty(this.data) ? this.data.ip : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      }),
      port: new FormControl(!isEmpty(this.data) ? this.data.port : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),

      host: new FormControl(!isEmpty(this.data) ? this.data.hostUuid : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      auth_method: new FormControl(
        !isEmpty(this.data) ? this.data.authType : [],
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      database_username: new FormControl(
        !isEmpty(this.data) ? this.data.authKey : []
      ),
      database_password: new FormControl(
        !isEmpty(this.data) ? this.data.authPwd : []
      ),
      kerberosId: new FormControl(
        !isEmpty(this.data) ? this.data.kerberosId : []
      )
    });

    this.watch();
  }

  watch() {
    this.formGroup.get('auth_method').valueChanges.subscribe(res => {
      if (res === this.dataMap.clickHouse_Auth_Method_Type.kerber.value) {
        this.formGroup
          .get('kerberosId')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('database_password').clearValidators();
        this.formGroup.get('database_username').clearValidators();
      } else if (
        res === this.dataMap.clickHouse_Auth_Method_Type.database.value
      ) {
        this.formGroup
          .get('database_password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('database_username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup.get('kerberosId').clearValidators();
      } else {
        this.formGroup.get('database_password').clearValidators();
        this.formGroup.get('database_username').clearValidators();
        this.formGroup.get('kerberosId').clearValidators();
      }
      this.formGroup.get('database_password').updateValueAndValidity();
      this.formGroup.get('database_username').updateValueAndValidity();
      this.formGroup.get('kerberosId').updateValueAndValidity();
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });
      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }

      if (
        find(paths, path => {
          return !CommonConsts.REGEX.templatLinuxPath.test(path);
        }) ||
        find(paths, path => {
          return path.length > 1024;
        })
      ) {
        return { pathError: { value: control.value } };
      }
    };
  }

  // 创建
  createKerberos() {
    const kerberosComponent = new KerberosComponent(
      this.i18n,
      this.drawModalService
    );
    kerberosComponent.create(undefined, kerberosId => {
      this.getKerberos(() => {
        this.formGroup.get('kerberosId').setValue(kerberosId);
      });
    });
  }

  getParams() {
    const originType = {}; // 区分修改和注册的场景，修改时需要传入uuid
    if (this.data) {
      originType['uuid'] = this.data.clusterParams.child.uuid;
    }
    const originExtendInfo = {};
    // kerber认证方式
    if (
      this.formGroup.value.auth_method ===
      this.dataMap.clickHouse_Auth_Method_Type.kerber.value
    ) {
      originExtendInfo['kerberosId'] = this.formGroup.value.kerberosId;
    }

    const authExtendInfo = {};

    // 数据库认证方式
    if (
      this.formGroup.value.auth_method ===
      this.dataMap.clickHouse_Auth_Method_Type.database.value
    ) {
      authExtendInfo['authKey'] = this.formGroup.value.database_username;
      authExtendInfo['authPwd'] = this.formGroup.value.database_password;
    }

    // 添加项
    this.addItem = {
      hostname: find(this.hostsOptions, { value: this.formGroup.value.host })
        .name,
      ip: find(this.hostsOptions, { value: this.formGroup.value.host })
        .endpoint,
      status: 1,
      clientPath: this.formGroup.value.client,
      businessIP: this.formGroup.value.business_ip,
      port: this.formGroup.value.port,
      clusterParams: {
        child: {
          uuid: isNil(this.data) ? null : this.data.clusterParams.child?.uuid,
          type: ResourceType.NODE,
          subType: DataMap.Resource_Type.ClickHouse.value,
          name:
            this.formGroup.value.business_ip.split('.').join('_') +
            `_${this.formGroup.value.port}`, // 采用IP+Port
          environment: {
            uuid: this.formGroup.value.host,
            rootUuid: this.formGroup.value.host,
            endpoint: find(this.hostsOptions, {
              value: this.formGroup.value.host
            }).endpoint,
            name: find(this.hostsOptions, { value: this.formGroup.value.host })
              .name,
            port:
              '' +
              find(this.hostsOptions, { value: this.formGroup.value.host }).port
          },
          auth: {
            authType: this.formGroup.value.auth_method,
            ...originExtendInfo,
            ...authExtendInfo,
            extendInfo: {
              ...originExtendInfo,
              ...authExtendInfo
            }
          },
          extendInfo: {
            port: this.formGroup.value.port,
            ip: this.formGroup.value.business_ip,
            resourceType: 'node',
            type: 'node',
            clientPath: this.formGroup.value.client,
            ...originExtendInfo
          },
          dependencies: {
            agents: [
              {
                uuid: this.formGroup.value.host,
                endpoint: find(this.hostsOptions, {
                  value: this.formGroup.value.host
                }).endpoint,
                name: find(this.hostsOptions, {
                  value: this.formGroup.value.host
                }).name
              }
            ]
          }
        }
      }
    };

    return {
      name: find(this.hostsOptions, { value: this.formGroup.value.host }).name,
      type: ResourceType.CLUSTER,
      subType: DataMap.Resource_Type.ClickHouse.value,
      extendInfo: {
        type: 'cluster',
        resourceType: 'cluster'
      },
      dependencies: {
        children: [
          {
            ...originType,
            type: ResourceType.NODE,
            subType: DataMap.Resource_Type.ClickHouse.value,
            name:
              this.formGroup.value.business_ip.split('.').join('_') +
              `_${this.formGroup.value.port}`, // 采用IP+Port
            auth: {
              authType: this.formGroup.value.auth_method,
              ...originExtendInfo,
              ...authExtendInfo,
              extendInfo: {
                ...originExtendInfo,
                ...authExtendInfo
              }
            },
            extendInfo: {
              port: this.formGroup.value.port,
              ip: this.formGroup.value.business_ip,
              type: 'node',
              resourceType: 'node',
              clientPath: this.formGroup.value.client,
              ...originExtendInfo,
              ...authExtendInfo
            },
            dependencies: {
              agents: [
                {
                  uuid: this.formGroup.value.host,
                  endpoint: find(this.hostsOptions, {
                    value: this.formGroup.value.host
                  }).endpoint,
                  name: find(this.hostsOptions, {
                    value: this.formGroup.value.host
                  }).name
                }
              ]
            }
          }
        ]
      }
    };
  }

  test(cb) {
    const params = this.getParams();
    if (this.tableData?.data?.length) {
      let flag = false;
      if (!this.data) {
        // 注册场景
        for (const item of this.tableData.data) {
          if (
            item.ip === this.formGroup.value.business_ip &&
            item.port === this.formGroup.value.port
          ) {
            flag = true;
            break;
          }
        }
      } else {
        // 修改场景
        for (const item of this.tableData.data) {
          if (item.ip === this.data.ip && item.port === this.data.port) {
            continue;
          } else {
            if (
              item.ip === this.formGroup.value.business_ip &&
              item.port === this.formGroup.value.port
            ) {
              flag = true;
              break;
            }
          }
        }
      }

      if (flag) {
        this.message.error(this.i18n.get('common_node_exist_label'), {
          lvMessageKey: 'errorKey',
          lvShowCloseButton: true
        });
        this.okLoading = false;
        return;
      }
    }

    this.protectedResourceApiService
      .CheckResource({
        checkResourceRequestBody: params as any,
        akOperationTips: false,
        akLoading: false
      })
      .subscribe(
        (res: any) => {
          if (res.length) {
            const returnRes = JSON.parse(res);
            const idx = returnRes.findIndex(item => item.code !== 0);
            if (idx !== -1) {
              this.message.error(this.i18n.get(returnRes[idx].code), {
                lvMessageKey: 'errorKey',
                lvShowCloseButton: true
              });
            } else {
              this.message.success(
                this.i18n.get('common_operate_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              cb();
            }
          } else {
            cb();
          }
          this.okLoading = false;
        },
        () => {
          this.okLoading = false;
        }
      );
  }

  ok() {
    this.okLoading = true;
    this.test(() => {
      this.okLoading = false;
      if (this.formGroup.invalid) {
        return;
      }
      const newItem = {
        uuid: this.addItem?.clusterParams.child.dependencies.agents[0].uuid,
        hostname: this.addItem?.clusterParams.child.dependencies.agents[0].name,
        endpoint: this.addItem?.clusterParams.child.dependencies.agents[0]
          .endpoint,
        status: '27', // 默认设置为在线
        clientPath: this.addItem?.clusterParams.child.extendInfo.clientPath,
        ip: this.addItem?.clusterParams.child.extendInfo.ip,
        port: this.addItem?.clusterParams.child.extendInfo.port,
        authType: this.addItem?.clusterParams.child.auth.authType,
        authKey: this.addItem?.clusterParams.child.auth.authKey,
        authPwd: this.addItem?.clusterParams.child.auth.authPwd,
        hostUuid: this.addItem?.clusterParams.child.dependencies.agents[0].uuid,
        kerberosId: this.addItem?.clusterParams.child.auth.extendInfo
          .kerberosId,
        clusterParams: this.addItem?.clusterParams
      };
      this.modal.close(); // 关闭注册页面

      // 修改的时候，需要先把原数据删除，再新增
      if (this.data) {
        const idx = this.tableData.data.findIndex(
          item =>
            item.endpoint === this.data.endpoint && item.ip === this.data.ip
        );
        this.tableData.data.splice(idx, 1);
      }
      // 注册的状态
      if (isEmpty(this.tableData)) {
        this.tableData = {
          data: [newItem],
          total: 1
        };
      } else {
        this.tableData.data = [newItem, ...this.tableData.data];
        this.tableData.total++;
      }

      this.globalService.emitStore({
        action: 'insertClickHouseCluster',
        state: this.tableData
      });
    });
  }
}
