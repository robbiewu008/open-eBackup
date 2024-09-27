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
import { Component, OnInit } from '@angular/core';
import {
  DataMapService,
  I18NService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  ResourceType,
  KerberosAPIService,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { MessageService } from '@iux/live';
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
  get,
  filter,
  some
} from 'lodash';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { TableData } from 'app/shared/components/pro-table';
import { Observable, Observer, of } from 'rxjs';
import { catchError, mergeMap } from 'rxjs/operators';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
@Component({
  selector: 'aui-redis-cluster-node',
  templateUrl: './redis-cluster-node.component.html',
  styleUrls: ['./redis-cluster-node.component.less']
})
export class RedisClusterNodeComponent implements OnInit {
  okLoading = false;
  testLoading = false;
  hostsOptions = []; // 主机名称
  formGroup: FormGroup;
  kerberosOptions = []; // kerberos选项数据
  tableData: TableData;
  data;
  addData;
  _isEmpty = isEmpty;

  nameErrorTip = this.baseUtilService.nameErrorTip;
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
    .toArray('redis_Auth_Method_Type')
    .filter(v => (v.isLeaf = true));

  keberosOptions: any; // keberos认证方式
  dataMap = DataMap;

  constructor(
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private kerberosApi: KerberosAPIService,
    public message: MessageService
  ) {}

  ngOnInit() {
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
        subType: [`${DataMap.Resource_Type.Redis.value}Plugin`]
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
        resource = filter(
          resource,
          item => item.environment.osType === this.dataMap.Os_Type.linux.value
        );
        if (isEmpty(this.data)) {
          if (!MultiCluster.isMulti) {
            resource = filter(
              resource,
              item =>
                item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          } else {
            resource = getMultiHostOps(resource);
          }
        }
        each(resource, item => {
          const tmp = item.environment;
          if (tmp.osType === this.dataMap.Os_Type.linux.value) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              port: tmp.port,
              endpoint: tmp.endpoint,
              isLeaf: true
            });
          }
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
      kerberosId: new FormControl(
        !isEmpty(this.data) ? this.data.kerberosId : []
      ),
      ssl: new FormControl(!isEmpty(this.data) ? this.data.sslEnable : true)
    });

    this.watch();
  }

  watch() {
    this.formGroup.get('auth_method').valueChanges.subscribe(res => {
      if (res === this.dataMap.redis_Auth_Method_Type.kerber.value) {
        this.formGroup
          .get('kerberosId')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('kerberosId').clearValidators();
      }
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
    const originExtendInfo = {};
    // kerber认证方式
    if (
      this.formGroup.value.auth_method ===
      this.dataMap.redis_Auth_Method_Type.kerber.value
    ) {
      originExtendInfo['kerberosId'] = this.formGroup.value.kerberosId;
    }

    return {
      name: find(this.hostsOptions, { value: this.formGroup.value.host }).name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.Redis.value,
      extendInfo: {
        type: 'cluster',
        resourceType: 'cluster'
      },
      dependencies: {
        children: [
          {
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.Redis.value,
            uuid: !isEmpty(this.data) ? this.data?.uuid : null,
            name:
              this.formGroup.value.business_ip.split('.').join('_') +
              `_${this.formGroup.value.port}`, // 采用IP+Port
            auth: {
              authType: this.formGroup.value.auth_method,
              extendInfo: {
                ...originExtendInfo
              }
            },
            extendInfo: {
              port: this.formGroup.value.port,
              ip: this.formGroup.value.business_ip,
              type: 'node',
              resourceType: 'node',
              clientPath: this.formGroup.value.client,
              sslEnable: this.formGroup.value.ssl,
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
        ]
      }
    };
  }

  onOK() {
    // 添加节点的时候需要校验当前结点是否已存在
    const existItem = this.tableData?.data.find(
      item =>
        item.ip === this.formGroup.value.business_ip &&
        item.port === this.formGroup.value.port &&
        !(
          this.formGroup.value.business_ip === this.data.ip &&
          this.formGroup.value.port === this.data.port
        )
    );
    if (!isEmpty(existItem) && !this.data.uuid) {
      this.message.error(this.i18n.get('common_node_exist_label'), {
        lvMessageKey: 'errorKey',
        lvShowCloseButton: true
      });
      return;
    }
    const params = this.getParams();

    return this.protectedResourceApiService
      .CheckResource({
        checkResourceRequestBody: params as any,
        akOperationTips: false
      })
      .pipe(
        catchError(res => {
          return of(true);
        }),
        mergeMap((res: any) => {
          if (res === true) {
            return of(null);
          }
          if (res.length) {
            const returnRes = JSON.parse(res);
            const idx = returnRes.findIndex(item => item.code !== 0);
            if (idx !== -1) {
              this.message.error(this.i18n.get(returnRes[idx].code), {
                lvMessageKey: 'errorKey',
                lvShowCloseButton: true
              });
              return of(null);
            } else {
              this.message.success(
                this.i18n.get('common_operate_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              return this.assembleData();
            }
          } else {
            return this.assembleData();
          }
        })
      );
  }

  assembleData() {
    return new Observable<void>((observer: Observer<void>) => {
      this.addData = {
        hostname: find(this.hostsOptions, { value: this.formGroup.value.host })
          .name,
        endpoint: find(this.hostsOptions, { value: this.formGroup.value.host })
          .endpoint,
        uuid: !isEmpty(this.data) ? this.data?.uuid : null,
        status: '27', // 默认设置为在线
        clientPath: this.formGroup.value.client,
        ip: this.formGroup.value.business_ip,
        port: this.formGroup.value.port,
        authType: this.formGroup.value.auth_method,
        hostUuid: this.formGroup.value.host,
        kerberosId: this.formGroup.value.kerberosId,
        sslEnable: this.formGroup.value.ssl
      };
      observer.next(this.addData);
      observer.complete();
    });
  }
}
