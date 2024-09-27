import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  MessageService,
  ModalRef,
  UploadFile,
  UploadFileStatusEnum
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  getMultiHostOps
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  first,
  get,
  isEmpty,
  map,
  size,
  trim,
  toString,
  differenceBy,
  filter,
  includes,
  defer
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-openstack',
  templateUrl: './register-openstack.component.html',
  styleUrls: ['./register-openstack.component.less']
})
export class RegisterOpenstackComponent implements OnInit {
  item: any;
  formGroup: FormGroup;
  dataMap = DataMap;
  originNameList = [];

  proxyOptions = [];

  certFiles = [];
  crlFiles = [];
  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';
  certFilters = [];
  revocationListFilters = [];
  selectCertFile = '';
  selectRevocationList = '';

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  keystoneErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  domainErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_openstack_username_valid_label'),
    invalidMaxLength: this.i18n.get('protection_openstack_username_valid_label')
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getProxyOptions();
    this.initFilters();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.globalResourceType.openStackContainer.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        // 过滤1.2.1版本
        resource = this.baseUtilService.rejectAgentsByVersion(
          resource,
          '1.2.1'
        );
        // 创建不展示离线主机
        const hostArray = [];
        if (isEmpty(this.item)) {
          if (MultiCluster.isMulti) {
            resource = getMultiHostOps(resource);
          } else {
            resource = filter(resource, val => {
              return (
                val.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
              );
            });
          }
        }
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
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
        this.proxyOptions = hostArray;
        if (!isEmpty(this.item)) {
          this.formGroup
            .get('agents')
            .setValue(
              filter(this.formGroup.value.agents, item =>
                includes(map(hostArray, 'uuid'), item)
              )
            );
        }
      }
    );
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.value.cert
      ? this.formGroup.invalid || isEmpty(this.certName)
      : this.formGroup.invalid;
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.item?.name || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.validIsSameName()
        ]
      }),
      keystone: new FormControl(this.item?.endpoint || '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      username: new FormControl(this.item?.auth?.authKey || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.openstackUserName),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      password: new FormControl(this.item?.auth?.authPwd || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      domain: new FormControl(this.item?.extendInfo?.domainName || 'Default', {
        validators: [this.baseUtilService.VALID.maxLength(255)]
      }),
      agents: new FormControl(
        !isEmpty(this.item)
          ? this.item?.dependencies?.agents.map(item => item.uuid)
          : [],
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      openstackInterface: new FormControl(
        this.item?.extendInfo?.isRegister === 'ture'
      ),
      cert: new FormControl(
        !isEmpty(this.item) ? this.item?.extendInfo?.enableCert === '1' : true
      )
    });
    this.updateCertInfo();
    this.listenForm();
  }

  listenForm() {
    this.formGroup.valueChanges.subscribe(() => this.disableOkBtn());
  }

  updateCertInfo() {
    if (!isEmpty(this.item) && this.item?.extendInfo?.enableCert === '1') {
      defer(() => {
        if (this.item?.extendInfo?.certName) {
          this.certFiles = [
            {
              key: 'cert_key',
              name: this.item?.extendInfo?.certName,
              fileSize: this.item?.extendInfo?.certSize,
              status: UploadFileStatusEnum.SUCCESS
            }
          ];
          this.certName = this.item?.extendInfo?.certName;
          this.certSize = this.item?.extendInfo?.certSize;
        }
        if (this.item?.extendInfo?.crlName) {
          this.crlFiles = [
            {
              key: 'crl_key',
              name: this.item?.extendInfo?.crlName,
              fileSize: this.item?.extendInfo?.crlSize,
              status: UploadFileStatusEnum.SUCCESS
            }
          ];
          this.crlName = this.item?.extendInfo?.crlName;
          this.crlSize = this.item?.extendInfo?.crlSize;
        }
        this.disableOkBtn();
      });
    }
  }

  initFilters() {
    this.certFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectCertFile = '';
            this.disableOkBtn();
            return '';
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );

            this.selectCertFile = '';
            this.disableOkBtn();
            return '';
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectCertFile = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            this.disableOkBtn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
    this.revocationListFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crl'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['crl']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectRevocationList = '';
            this.disableOkBtn();
            return '';
          }
          if (files[0].size > 5 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['5KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectRevocationList = '';
            this.disableOkBtn();
            return '';
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            this.disableOkBtn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  certFilesChange(files) {
    if (size(files) === 0) {
      each(['certName', 'certSize'], key => (this[key] = ''));
      this.selectCertFile = '';
      this.disableOkBtn();
    } else {
      this.certName = get(first(files), 'name');
      this.certSize = get(first(files), 'fileSize');
    }
  }

  crlFilesChange(files) {
    if (size(files) === 0) {
      each(['crlName', 'crlSize'], key => (this[key] = ''));
      this.selectRevocationList = '';
    } else {
      this.crlName = get(first(files), 'name');
      this.crlSize = get(first(files), 'fileSize');
    }
  }

  validIsSameName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originNameList.includes(trim(control.value))
        ? { invalidSameName: { value: control.value } }
        : null;
    };
  }

  getParams() {
    let reduceAgents = [];
    if (this.item) {
      reduceAgents = differenceBy(
        this.item?.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    return {
      name: this.formGroup.value.name,
      type: ResourceType.OpenStack,
      subType: ResourceType.OPENSTACK_CONTAINER,
      endpoint: this.formGroup.value.keystone,
      extendInfo: {
        isRegister: toString(this.formGroup.value.openstackInterface),
        enableCert: this.formGroup.value.cert ? '1' : '0',
        certName: this.formGroup.value.cert ? this.certName : '',
        certSize: this.formGroup.value.cert ? this.certSize : '',
        crlName: this.formGroup.value.cert ? this.crlName : '',
        crlSize: this.formGroup.value.cert ? this.crlSize : ''
      },
      auth: {
        authType: '1',
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          certification: this.formGroup.value.cert ? this.selectCertFile : '',
          revocationList: this.formGroup.value.cert
            ? this.selectRevocationList
            : '',
          enableCert: this.formGroup.value.cert ? '1' : '0'
        }
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return {
            uuid: item
          };
        }),
        '-agents': !isEmpty(this.item)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  onOK(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      if (!isEmpty(this.item)) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: this.getParams(),
            envId: this.item.uuid
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }
}
