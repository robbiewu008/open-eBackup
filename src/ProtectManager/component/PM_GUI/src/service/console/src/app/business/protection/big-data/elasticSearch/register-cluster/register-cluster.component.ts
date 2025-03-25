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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import {
  MessageboxService,
  MessageService,
  ModalRef,
  UploadFile
} from '@iux/live';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  KerberosAPIService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  ExceptionService,
  MultiCluster,
  CookieService,
  getMultiHostOps
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  filter,
  first,
  get,
  includes,
  isEmpty,
  isFunction,
  isNil,
  isNumber,
  isUndefined,
  last,
  map,
  set,
  size,
  split,
  toNumber,
  toString as _toString,
  uniq
} from 'lodash';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  @Input() data;
  formGroup: FormGroup;
  clusterAuthType = DataMap.ElasticSearch_Clusters_Auth_Type;
  isTest = false;
  enableBtn = false;
  testLoading = false;
  okLoading = false;
  selectFile;
  fileFilters = [];
  certificationTips = this.i18n.get(
    'protection_es_kerberos_certificate_tips_label'
  );
  kerberosOptions = [];
  proxyHostOptions = [];

  authOptions = this.dataMapService
    .toArray('ElasticSearch_Clusters_Auth_Type')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    })
    .filter(item => item.value !== DataMap.HDFS_Clusters_Auth_Type.ldap.value);

  nameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });
  serverErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    samePathError: this.i18n.get('protection_same_path_error_label'),
    invalidName: this.i18n.get('common_invalid_inputtext_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [4096])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };

  ipErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.ipErrorTip
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  @ViewChild('modalFooter', { static: true }) modalFooter: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private kerberosApi: KerberosAPIService,
    private drawModalService: DrawModalService,
    private exceptionService: ExceptionService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initFooter();
    this.initForm();
    this.initFilter();
    this.getAgents();
    this.getKerberos();
  }

  initFooter() {
    this.modal.setProperty({ lvFooter: this.modalFooter });
  }

  enableBtnFn() {
    this.enableBtn = !this.formGroup.invalid;

    if (
      (this.formGroup.value.loginMode === this.clusterAuthType.xpack.value &&
        !!this.formGroup.value.username &&
        !this.formGroup.value.password) ||
      (this.formGroup.value.loginMode === this.clusterAuthType.xpack.value &&
        !this.formGroup.value.username &&
        !!this.formGroup.value.password)
    ) {
      this.enableBtn = false;
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      serverLink: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.vaildServerLink()
        ]
      }),
      loginMode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      kerberosId: new FormControl(''),
      username: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(256)]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(64)]
      }),
      agents: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      pathUser: new FormControl('omm', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      pathAttr: new FormControl('wheel', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      useSecProtocols: new FormControl(false),
      useSecCipherSuites: new FormControl(false)
    });

    this.formGroup.get('loginMode').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }
      // 统一清除校验
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
      if (res === this.clusterAuthType.xpack.value) {
        this.certificationTips = this.i18n.get(
          'protection_es_xpack_certificate_tips_label'
        );
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.name(CommonConsts.REGEX.name, false)
          ]);

        this.formGroup
          .get('password')
          .setValidators([this.baseUtilService.VALID.maxLength(64)]);
        this.formGroup.get('kerberosId').clearValidators();
      } else if (res === this.clusterAuthType.kerberos.value) {
        this.certificationTips = this.i18n.get(
          'protection_es_kerberos_certificate_tips_label'
        );
        this.formGroup
          .get('kerberosId')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('username').markAsTouched();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('kerberosId').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(res => this.enableBtnFn());

    if (this.data.uuid) {
      this.formGroup.patchValue({
        name: this.data.name,
        serverLink: this.data.extendInfo?.ElasticSearchAddress,
        loginMode: this.data.auth.authType,
        kerberosId: this.data.extendInfo?.kerberosId,
        username: isNil(this.data.auth.authKey)
          ? this.data.auth.authKey
          : this.data.auth.authKey.split('@')[0],
        agents: this.data.extendInfo?.agents.split(';'),
        pathUser: this.data.extendInfo?.ElasticSearchUser,
        pathAttr: this.data.extendInfo?.ElasticSearchGroup,
        useSecProtocols: this.data.extendInfo?.useSecProtocols === 'true',
        useSecCipherSuites: this.data.extendInfo?.useSecCipherSuites === 'true'
      });
    }
  }

  initFilter() {
    this.fileFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crt'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['crt']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            return validFiles;
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  filesChange(file, name) {
    if (!size(file)) {
      switch (name) {
        case 'cert':
          this.selectFile = '';
          break;
        default:
          break;
      }
    }
    this.enableBtnFn();
  }

  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      label: `${item.environment?.name}(${item.environment?.endpoint})`,
      value: item.rootUuid || item.parentUuid,
      isLeaf: true
    });
  }

  isEnableAgent(item) {
    return (
      item.environment?.linkStatus ===
        DataMap.resource_LinkStatus_Special.normal.value ||
      includes(this.data?.extendInfo?.agents?.split(';'), item.rootUuid)
    );
  }

  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: [
            `${DataMap.Resource_Type.ElasticsearchBackupSet.value}Plugin`
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
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          recordsTemp = filter(recordsTemp, item => this.isEnableAgent(item));
          const agentArr = [];
          if (MultiCluster.isMulti) {
            recordsTemp = getMultiHostOps(recordsTemp);
            each(recordsTemp, item => {
              this.addData(agentArr, item);
            });
          } else {
            each(recordsTemp, item => {
              this.addData(agentArr, item);
            });
          }
          this.proxyHostOptions = agentArr;
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
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
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.BIG_DATA,
      subType: DataMap.Resource_Type.Elasticsearch.value,
      extendInfo: {
        kerberosId: this.formGroup.value.kerberosId,
        agents: this.formGroup.value.agents.join(';'),
        ElasticSearchAddress: this.formGroup.value.serverLink,
        ElasticSearchUser: this.formGroup.value.pathUser,
        ElasticSearchGroup: this.formGroup.value.pathAttr,
        useSecProtocols: this.formGroup.value.useSecProtocols,
        useSecCipherSuites: this.formGroup.value.useSecCipherSuites
      },
      auth: {
        authType: this.formGroup.value.loginMode,
        extendInfo: {}
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return {
            uuid: item
          };
        })
      }
    };

    if (this.isHcsUser) {
      assign(params.extendInfo, {
        subNetFixedIpList: JSON.stringify(
          map(
            filter(this.proxyHostOptions, item =>
              includes(this.formGroup.value.agents, item.rootUuid)
            ),
            agent => agent.environment?.extendInfo?.subNetFixedIp
          )
        )
      });
    }

    if (size(this.selectFile)) {
      set(params, 'extendInfo.certificate', this.selectFile);
    }
    if (
      !size(this.selectFile) &&
      !!this.data.uuid &&
      this.formGroup.value.loginMode === this.clusterAuthType.xpack.value
    ) {
      // 没有证书，修改场景，默认塞空字符串
      set(params, 'extendInfo.certificate', '');
    }

    if (this.formGroup.value.loginMode === this.clusterAuthType.xpack.value) {
      delete params.extendInfo.kerberosId;
      if (this.formGroup.value.username && this.formGroup.value.password) {
        set(params, 'auth.authKey', this.formGroup.value.username);
        set(params, 'auth.authPwd', this.formGroup.value.password);
      }
    }
    return params;
  }

  test() {
    const params = this.getParams();

    this.protectedEnvironmentApiService
      .CheckEnvironment({
        checkEnvironmentRequestBody: params as any,
        akOperationTips: false
      })
      .subscribe(res => {
        const error = first(JSON.parse(res));
        if (get(error, 'code') !== 0) {
          this.exceptionService.alertMsg({
            errorCode: get(error, 'code'),
            errorMessage: get(error, 'message')
          });
          this.isTest = false;
        } else {
          this.message.success(this.i18n.get('common_operate_success_label'));
          this.isTest = true;
        }
      });
  }

  ok() {
    const params = this.getParams();
    this.okLoading = true;
    const successCallback = res => {
      this.okLoading = false;
      this.modal.close();
      if (this.data && isFunction(this.data.getCluster)) {
        this.data.getCluster();
      }
    };
    const errorCallback = err => {
      this.okLoading = false;
      let errorCode = err.error?.errorCode;

      if (errorCode === '1677931423') {
        this.okLoading = true;
        this.messageBox.confirm({
          lvDialogIcon: 'lv-icon-popup-warning-48',
          lvContent: this.i18n.get(errorCode),
          lvOk: () => {
            assign(params.extendInfo, { ecceptRisk: true });
            this.protectedEnvironmentApiService
              .RegisterProtectedEnviroment({
                RegisterProtectedEnviromentRequestBody: params
              })
              .subscribe(
                res => {
                  this.okLoading = false;
                  this.modal.close();
                  if (this.data && isFunction(this.data.getCluster)) {
                    this.data.getCluster();
                  }
                },
                () => {
                  this.okLoading = false;
                }
              );
          }
        });
      }
    };

    if (this.data.uuid) {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.data.uuid,
          UpdateProtectedEnvironmentRequestBody: params
        })
        .subscribe(successCallback, errorCallback);
    } else {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params
        })
        .subscribe(successCallback, errorCallback);
    }
  }

  cancle() {
    this.modal.close();
  }

  vaildServerLink() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      if (control.value?.length > 4096) {
        return { invalidMaxLength: { value: control.value } };
      }

      const links = split(control.value, ',')?.filter(item => {
        return !isEmpty(item);
      });
      let invaildInfo;

      if (links.length !== uniq(links).length) {
        return { samePathError: { value: control.value } };
      }

      each(links, link => {
        const path = link.split(':');
        const reg = /^([1-9])([0-9]*)$/;

        if (
          path.length !== 2 ||
          !CommonConsts.REGEX.ipv4.test(first(path)) ||
          !reg.test(last(path)) ||
          toNumber(last(path)) > 65535
        ) {
          invaildInfo = { invalidName: { value: control.value } };
        }
      });

      return invaildInfo ? invaildInfo : null;
    };
  }
}
