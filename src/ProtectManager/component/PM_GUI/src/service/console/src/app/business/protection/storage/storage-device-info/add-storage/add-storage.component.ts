import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  ProductStoragesApiService,
  ProtectedEnvironmentApiService,
  DataMapService,
  ProtectedResourceApiService,
  WarningMessageService,
  CommonConsts,
  AgentsSubType,
  ClientManagerApiService
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNil,
  isNumber,
  map,
  pick
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-storage',
  templateUrl: './add-storage.component.html',
  styleUrls: ['./add-storage.component.less']
})
export class AddStorageComponent implements OnInit {
  item;
  formGroup: FormGroup;
  DataMap = DataMap;
  isFocusPassword = false;
  deviceStorageType = DataMap.Device_Storage_Type;

  fcCertFilters = [];
  selectFcSiteFile;
  revocationListFilters = [];
  selectRevocationList = '';

  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';

  typeOptions = this.dataMapService
    .toArray('Device_Storage_Type')
    .filter(v => (v.isLeaf = true))
    .filter(item => {
      return item.value !== DataMap.Device_Storage_Type.Other.value;
    });
  typeValues = map(this.dataMapService.toArray('Device_Storage_Type'), 'value');
  hostOptions = [];
  agentData = [];
  exterAgent = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x9000.value,
      DataMap.Deploy_Type.e6000.value
    ],
    this.i18n.get('deploy_type')
  );
  isAgentExternal = false;
  externalAgentLists = [];
  poxyOptions = [];

  deviceNameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.nameErrorTip
  );
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  usernameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );
  passwordErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private message: MessageService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private productStoragesApiService: ProductStoragesApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public warningMessageService: WarningMessageService,
    private clientManagerApiService: ClientManagerApiService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
    this.updateData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(
        DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value,
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      ),
      equipment_name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ],
        updateOn: 'change'
      }),
      fqdn: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      }),
      port: new FormControl('8088', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      verify_status: new FormControl(true),
      proxyHost: new FormControl([])
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.listenType(res);
    });

    this.formGroup.statusChanges.subscribe(formGroupStatus => {
      const modalIns = this.modal.getInstance();
      defer(() => {
        modalIns.lvOkDisabled = this.formGroup.value.verify_status
          ? !(formGroupStatus === 'VALID' && !!this.certName)
          : formGroupStatus !== 'VALID';
      });
    });

    this.formGroup.get('verify_status')?.valueChanges.subscribe(() => {
      this.selectFcSiteFile = ''; // 清空
      this.selectRevocationList = '';
    });
  }

  initFilters() {
    this.fcCertFilters = [
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
            this.selectFcSiteFile = '';
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
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
            return validFiles;
          }

          const reader = new FileReader();
          this.certName = first(files)?.name;
          this.certSize = first(files)?.fileSize;
          reader.onloadend = () => {
            this.selectFcSiteFile = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              !this.selectFcSiteFile || this.formGroup.invalid;
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
            return validFiles;
          }
          if (files[0].size > 5 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['5KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );

            this.selectRevocationList = ''; // 清空
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
          this.crlName = first(files)?.name;
          this.crlSize = first(files)?.fileSize;
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              !this.selectFcSiteFile || this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  certChange(files) {
    if (isEmpty(files)) {
      this.selectFcSiteFile = '';
      const modalIns = this.modal.getInstance();
      modalIns.lvOkDisabled = !this.selectFcSiteFile || this.formGroup.invalid;
    }
  }

  revocationListChange(files) {
    if (isEmpty(files)) {
      this.selectRevocationList = '';
    }
  }

  updateData() {
    if (!this.item) {
      return;
    }
    // 类型为ndmp-sever时，需要调详情接口获取所选代理主机信息
    if (this.item.subType === this.deviceStorageType.ndmp.value) {
      this.protectedResourceApiService
        .ShowResource({ resourceId: this.item.uuid })
        .subscribe(res => {
          this.agentData = get(res, 'dependencies.agents', []);
        });
    }

    const item = {
      type: this.item.subType,
      equipment_name: this.item.name,
      fqdn: this.item.endpoint,
      port: +this.item.port,
      username: this.item.username,
      verify_status: !!+this.item.extendInfo.verifyStatus
    };
    this.formGroup.patchValue(item);
    defer(() => {
      this.modal.getInstance().lvOkDisabled =
        !this.selectFcSiteFile || this.formGroup.invalid;
      if (this.item.name) {
        this.formGroup.get('equipment_name').disable();
      } else {
        this.formGroup.get('equipment_name').enable();
      }
    });
  }

  listenType(type) {
    if (![this.deviceStorageType.Other.value].includes(type)) {
      this.formGroup
        .get('fqdn')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]);
      this.formGroup
        .get('port')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]);
      this.formGroup
        .get('username')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]);
      this.formGroup
        .get('password')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]);
    } else {
      this.formGroup.get('fqdn').clearValidators();
      this.formGroup.get('port').clearValidators();
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
    }

    if (this.deviceStorageType.NetApp.value === type) {
      this.formGroup.get('port').setValue('443');
    } else if (this.deviceStorageType.Other.value === type) {
      this.formGroup.get('port').setValue('');
    } else if (type === this.deviceStorageType.ndmp.value) {
      this.formGroup.get('port').setValue('10000');
    } else {
      this.formGroup.get('port').setValue('8088');
    }
    // 设备类型为NDMP Server时，增加代理主机
    if (type === this.deviceStorageType.ndmp.value) {
      this.getProxyOptions();
      this.formGroup
        .get('proxyHost')
        .setValidators([this.baseUtilService.VALID.required()]);
    } else {
      this.formGroup.get('proxyHost').clearValidators();
    }
    this.formGroup.get('proxyHost').updateValueAndValidity();
    this.formGroup.get('fqdn').updateValueAndValidity();
    this.formGroup.get('port').updateValueAndValidity();
    this.formGroup.get('username').updateValueAndValidity();
    this.formGroup.get('password').updateValueAndValidity();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: AgentsSubType.Ndmp,
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      })
    };

    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            value: item.rootUuid || item.parentUuid,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
        if (this.item && this.agentData) {
          this.formGroup.get('proxyHost').setValue(map(this.agentData, 'uuid'));
        }
      }
    );
  }

  getParams() {
    const deleteAgents = [];
    if (this.item && this.agentData) {
      each(this.agentData, item => {
        if (!find(this.formGroup.value.proxyHost, val => val === item.uuid)) {
          deleteAgents.push({
            uuid: item.uuid
          });
        }
      });
    }
    const params = {
      name: !this.item ? this.formGroup.value.equipment_name : this.item.name,
      type: 'StorageEquipment',
      subType: this.formGroup.value.type,
      endpoint: this.formGroup.value.fqdn,
      port: this.formGroup.value.port,
      extendInfo: {
        verifyStatus:
          this.formGroup.value.type === this.deviceStorageType.Other.value
            ? '0'
            : this.formGroup.value.verify_status
            ? '1'
            : '0',
        snapConsistency: '1'
      },
      auth: {
        authType: 2,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          enableCert: String(+this.formGroup.value.verify_status),
          certification: this.selectFcSiteFile,
          revocationlist: this.selectRevocationList
        }
      }
    };
    if (
      [this.deviceStorageType.ndmp.value].includes(this.formGroup.value.type)
    ) {
      assign(params, {
        dependencies: {
          agents: map(this.formGroup.get('proxyHost').value, item => {
            return {
              uuid: item
            };
          }),
          '-agents': deleteAgents
        }
      });
    }
    return params;
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let body = this.getParams() as any;

      if (
        [this.deviceStorageType.Other.value].includes(this.formGroup.value.type)
      ) {
        body = pick(body, ['name', 'subType', 'type', 'extendInfo']);
      }

      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: body
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

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      let body = this.getParams() as any;

      if (
        [this.deviceStorageType.Other.value].includes(this.formGroup.value.type)
      ) {
        body = pick(body, ['name', 'subType', 'type', 'extendInfo']);
      }

      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.item.uuid,
          UpdateProtectedEnvironmentRequestBody: body
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
    });
  }

  onOK(): Observable<void> {
    return this.item ? this.modify() : this.create();
  }
}
