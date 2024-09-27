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
  FormGroup,
  ValidationErrors,
  ValidatorFn
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
  MODAL_COMMON,
  MultiCluster,
  ClientManagerApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  find,
  first,
  get,
  isEmpty,
  isFunction,
  isNumber,
  isUndefined,
  last,
  map,
  set,
  size,
  toNumber,
  toString as _toString,
  trim,
  uniq,
  isString,
  includes
} from 'lodash';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  @Input() data;
  formGroup: FormGroup;
  clusterAuthType = DataMap.HDFS_Clusters_Auth_Type;
  isTest = false;
  enableBtn = false;
  testLoading = false;
  okLoading = false;
  uploadPlaceholder = '';
  selectHiveSiteFile;
  selectHdfsSiteFile;
  selectCoreSiteFile;
  selectHiveClientFile;
  selectCertFile;
  hiveFilters = [];
  hdfsFilters = [];
  coreFilters = [];
  hiveClientFilters = [];
  certFilters = [];

  kerberosOptions = [];
  proxyHostOptions = [];

  authOptions = this.dataMapService
    .toArray('HDFS_Clusters_Auth_Type')
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
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256]),
    invalidName: this.i18n.get('protection_hive_server_two_tip_label'),
    invalidZKMode: this.i18n.get('protection_zk_mode_tip_label'),
    samePathError: this.i18n.get('protection_same_path_error_label')
  };
  versionErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_hive_version_tips_label')
  };
  passwordErrorTip = {
    ...this.baseUtilService.pwdErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  ipErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.ipErrorTip
  );
  @ViewChild('modalFooter', { static: true }) modalFooter: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private kerberosApi: KerberosAPIService,
    private drawModalService: DrawModalService,
    private exceptionService: ExceptionService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initFooter();
    this.initForm();
    this.initFilters();
    this.getAgents();
    this.getKerberos();
    this.uploadPlaceholder = this.data?.uuid
      ? this.i18n.get('protection_big_data_upload_placeholder_label')
      : this.i18n.get('protection_upload_placeholder_label');
  }

  initFooter() {
    this.modal.setProperty({ lvFooter: this.modalFooter });
  }

  initFilters() {
    this.hiveFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectHiveSiteFile = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectHiveSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.hdfsFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectHdfsSiteFile = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectHdfsSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.coreFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey2',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.selectCoreSiteFile = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectCoreSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.hiveClientFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['properties'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['properties']),
              {
                lvMessageKey: 'formatErrorKey2',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 100 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['100KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectHiveClientFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.certFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['jks'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['jks']),
              {
                lvMessageKey: 'formatErrorKey2',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectCertFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
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
        case 'hive':
          this.selectHiveSiteFile = '';
          break;
        case 'hdfs':
          this.selectHdfsSiteFile = '';
          break;
        case 'core':
          this.selectCoreSiteFile = '';
          break;
        case 'client':
          this.selectHiveClientFile = '';
          break;
        case 'cert':
          this.selectCertFile = '';
          break;
        default:
          break;
      }
    }
    this.enableBtnFn();
  }

  enableBtnFn() {
    let highAvailability;

    if (
      this.formGroup.value.serverLink.split(',').length > 1 &&
      !this.formGroup.value.zookeeperNamespace
    ) {
      this.message.error(this.i18n.get('protection_zk_mode_tip_label'));
      highAvailability = false;
    } else {
      highAvailability = true;
    }

    const validCert =
      this.formGroup.value.loginMode === this.clusterAuthType.system.value
        ? true
        : !(this.formGroup?.value?.cert && isEmpty(this.selectCertFile));

    this.enableBtn =
      !this.formGroup.invalid &&
      !isEmpty(this.selectHiveSiteFile) &&
      !isEmpty(this.selectHdfsSiteFile) &&
      !isEmpty(this.selectCoreSiteFile) &&
      highAvailability &&
      validCert;

    if (this.data.uuid) {
      this.enableBtn = !this.formGroup.invalid && highAvailability && validCert;
    }
  }

  initForm() {
    this.formGroup = this.fb.group(
      {
        name: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name()
          ]
        }),
        serverLink: new FormControl('', {
          validators: [this.baseUtilService.VALID.required(), this.validPath()]
        }),
        serverPrincipal: new FormControl('', {
          validators: [this.baseUtilService.VALID.maxLength(256)]
        }),
        zookeeperNamespace: new FormControl(''),
        version: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.validVersion()
          ]
        }),
        loginMode: new FormControl('', {
          validators: [this.baseUtilService.VALID.required()]
        }),
        kerberosId: new FormControl(''),
        username: new FormControl('hdfs'),
        agents: new FormControl([], {
          validators: [this.baseUtilService.VALID.minLength(1)]
        }),
        cert: new FormControl(true),
        certPassword: new FormControl(''),
        confirmCertPassword: new FormControl(''),
        useSecProtocols: new FormControl(true)
      },
      {
        validators: this.validatorForm
      }
    );

    this.formGroup.get('cert').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.get('useSecProtocols').setValue(true);
        this.formGroup
          .get('certPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]);
        this.formGroup
          .get('confirmCertPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]);
      } else {
        this.formGroup.get('certPassword').clearValidators();
        this.formGroup.get('confirmCertPassword').clearValidators();
      }
    });

    this.formGroup.get('loginMode').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }
      if (res === DataMap.HDFS_Clusters_Auth_Type.system.value) {
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
          ]);
        this.formGroup.get('kerberosId').clearValidators();
        this.formGroup.get('serverPrincipal').clearValidators();
      } else {
        this.formGroup
          .get('kerberosId')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('serverPrincipal')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('username').clearValidators();
      }
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('kerberosId').updateValueAndValidity();
      this.formGroup.get('serverPrincipal').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(res => this.enableBtnFn());

    if (this.data.uuid) {
      this.formGroup.patchValue({
        name: this.data.name,
        serverLink: this.data.extendInfo.hiveServerUrl,
        serverPrincipal: this.data.extendInfo?.hiveServerPrincipal,
        zookeeperNamespace: this.data.extendInfo?.zookeeperNameSpace,
        version: this.data.extendInfo?.hiveVersion,
        loginMode: this.data.extendInfo.kerberosId
          ? DataMap.HDFS_Clusters_Auth_Type.kerberos.value
          : DataMap.HDFS_Clusters_Auth_Type.system.value,
        kerberosId: this.data.extendInfo.kerberosId,
        username: this.data.auth.authKey,
        agents: this.data.extendInfo?.agents.split(';')
      });
      this.formGroup
        .get('cert')
        .setValue(this.data.extendInfo?.enableCert === 'true');
      this.formGroup
        .get('useSecProtocols')
        .setValue(this.data.extendInfo?.useSecProtocols === 'true');
    }
  }

  validatorForm: ValidatorFn = (
    control: AbstractControl
  ): ValidationErrors | null => {
    const password = trim(control.get('certPassword').value);
    const configPassword = trim(control.get('confirmCertPassword').value);
    if (control.get('cert').value) {
      if (password && configPassword) {
        if (password !== configPassword) {
          control.get('certPassword').setErrors({ diffPwd: true });
          control.get('confirmCertPassword').setErrors({ diffPwd: true });
        } else {
          control.get('certPassword').setErrors(null);
          control.get('confirmCertPassword').setErrors(null);
        }
      }
    } else {
      control.get('certPassword').setErrors(null);
      control.get('confirmCertPassword').setErrors(null);
    }

    return null;
  };

  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      label: `${item.name}(${item.endpoint})`,
      value: item.rootUuid || item.parentUuid,
      isLeaf: true
    });
  }

  isRegisterAgent(uuid) {
    return includes(this.data?.extendInfo?.agents.split(';'), uuid);
  }

  getAgents(recordsTemp?, startPage?) {
    this.clientManagerApiService
      .queryAgentListInfoUsingGET({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          pluginType: `${DataMap.Resource_Type.HiveBackupSet.value}Plugin`
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
          const agentArr = [];
          each(recordsTemp, item => {
            if (
              item.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value ||
              this.isRegisterAgent(item.rootUuid)
            ) {
              this.addData(agentArr, item);
            }
          });
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
      subType: DataMap.Resource_Type.Hive.value,
      extendInfo: {
        hiveServerUrl: this.formGroup.value.serverLink,
        hiveServerPrincipal: this.formGroup.value.serverPrincipal,
        zookeeperNameSpace: this.formGroup.value.zookeeperNamespace,
        hdfsSite: this.selectHdfsSiteFile || this.data.extendInfo.hdfsSite,
        coreSite: this.selectCoreSiteFile || this.data.extendInfo.coreSite,
        hiveSite: this.selectHiveSiteFile || this.data.extendInfo.hiveSite,
        hiveClient: this.selectHiveClientFile,
        hiveVersion: this.formGroup.value.version,
        kerberosId: this.formGroup.value.kerberosId,
        agents: this.formGroup.value.agents.join(';'),
        enableCert:
          this.formGroup.value.loginMode === this.clusterAuthType.system.value
            ? false
            : this.formGroup.value.cert,
        useSecProtocols:
          this.formGroup.value.loginMode === this.clusterAuthType.system.value
            ? false
            : this.formGroup.value.useSecProtocols
      },
      auth: {
        authType: this.formGroup.value.loginMode,
        authKey: this.formGroup.value.username,
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
    if (this.formGroup.value.loginMode === this.clusterAuthType.system.value) {
      delete params.extendInfo.kerberosId;
    } else if (
      this.formGroup.value.loginMode === this.clusterAuthType.kerberos.value
    ) {
      delete params.auth.authKey;
    }

    if (this.formGroup.value.cert) {
      set(params, 'auth.extendInfo', {
        sslTrustStore: this.selectCertFile,
        trustStorePassword: this.formGroup.value.certPassword
      });
    }
    return params;
  }

  test() {
    const params = this.getParams();

    this.protectedEnvironmentApiService
      .CheckEnvironment({
        checkEnvironmentRequestBody: params,
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
      let errorMessage = err.error?.errorMessage;

      if (errorMessage === 'The hive cluster partially exist.') {
        this.okLoading = true;
        this.messageBox.confirm({
          lvDialogIcon: 'lv-icon-popup-warning-48',
          lvWidth: MODAL_COMMON.largeWidth,
          lvContent: this.i18n.get(
            'protection_hive_cluster_partially_exist_tips_label',
            [...err.error?.parameters]
          ),
          lvOk: () => {
            assign(params.extendInfo, { acceptRisk: true });
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
          },
          lvCancel: () => {
            this.okLoading = false;
          }
        });
      } else {
        const error = isString(err.error) ? JSON.parse(err.error) : err.error;
        this.exceptionService.alertMsg(error);
      }
    };

    if (this.data.uuid) {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.data.uuid,
          UpdateProtectedEnvironmentRequestBody: params,
          akDoException: false
        })
        .subscribe(successCallback, errorCallback);
    } else {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params,
          akDoException: false
        })
        .subscribe(successCallback, errorCallback);
    }
  }

  cancle() {
    this.modal.close();
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      let invaildInfo;
      if (!trim(control.value)) {
        return null;
      }

      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });

      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }

      each(paths, item => {
        const path = item.split(':');
        const reg = /^([1-9])([0-9]*)$/;
        const port: string = last(path);

        if (item.length > 256) {
          invaildInfo = { invalidMaxLength: { value: control.value } };
        } else if (path.length !== 2) {
          invaildInfo = { invalidName: { value: control.value } };
        } else if (!reg.test(port) || toNumber(port) > 65535) {
          invaildInfo = { invalidName: { value: control.value } };
        }
      });

      return invaildInfo ? invaildInfo : null;
    };
  }

  validVersion(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      const value = control.value;
      const reg = /^\d+\.\d+\.\d+$/;
      if (!reg.test(value)) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }
}
