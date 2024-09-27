import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import {
  AgentsSubType,
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  KerberosAPIService,
  LANGUAGE,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
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
  isNumber,
  isUndefined,
  omit,
  size,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-nas-share',
  templateUrl: './register-nas-share.component.html',
  styleUrls: ['./register-nas-share.component.less']
})
export class RegisterNasShareComponent implements OnInit {
  item;
  items = [];
  formGroup: FormGroup;
  isMulti = MultiCluster.isMulti;
  dataMap = DataMap;
  authOptions = [];
  filterParams = [];
  kerberosOptions = [];
  poxyOptions = [];
  equipmentOptions = [{ value: 1, label: 'test', isLeaf: true }];
  osType = DataMap.Os_Type.linux.value;
  isEn = this.i18n.language === LANGUAGE.EN;
  exterAgent = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  hostOptions = [];
  sharedModeOptions = this.dataMapService
    .toArray('Shared_Mode')
    .filter(v => (v.isLeaf = true));

  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  nasNameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [512]),
      invalidName: this.i18n.get('protection_invalid_nasshare_nfs_name_label'),
      invalidSpecialPath: this.i18n.get('protection_no_special_path_label')
    }
  );
  ipNameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256]),
      invalidName: this.i18n.get('protection_no_slash_error_label')
    }
  );
  domainNameErrorTip = {
    invalidName: this.i18n.get('common_invalid_input_label')
  };
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
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
    }
  );
  @ViewChild('resourceFilter', { static: false }) filterComponent;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private kerberosApi: KerberosAPIService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getKerberos();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      nas_name: new FormControl(
        { value: '', disabled: this.item && this.item.uuid },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(512),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.nasShareNfsName),
            this.validNfsShareName()
          ]
        }
      ),
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      share_mode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      encryption: new FormControl(false),
      auth_mode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      kerberos: new FormControl(''),
      domain_name: new FormControl(''),
      username: new FormControl(''),
      password: new FormControl(''),
      modifyFilter: new FormControl(false)
    });

    if (this.exterAgent) {
      this.formGroup.addControl(
        'proxyHost',
        new FormControl([], {
          validators: [this.baseUtilService.VALID.required()]
        })
      );
    }

    this.listernForm();
    this.patchValue();
  }

  setProxyValue(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 200,
      conditions: JSON.stringify({
        subType: [AgentsSubType.NasShare],
        environment: {
          linkStatus: [['in'], DataMap.resource_LinkStatus_Special.normal.value]
        }
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
        startPage === Math.ceil(res.totalCount / 200) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(recordsTemp, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            label: `${item.environment?.name}(${item.environment?.endpoint})`,
            value: item.rootUuid || item.parentUuid,
            isLeaf: true
          });
        });
        let agents = [];
        each(hostArray, item => {
          if (
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.builtin.value
          ) {
            agents.push(item.rootUuid || item.parentUuid);
          }
        });
        this.formGroup.get('proxyHost').setValue(agents);

        return;
      }
      this.getProxyOptions(recordsTemp, startPage);
    });
  }

  getProxyOptions(recordsTemp?, startPage?) {
    if (!this.exterAgent) {
      return;
    }
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 200,
      conditions: JSON.stringify({
        pluginType: AgentsSubType.NasShare,
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      })
    };
    this.clientManagerApiService
      .queryAgentListInfoUsingGET(params)
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
          const hostArray = [];
          each(recordsTemp, item => {
            hostArray.push({
              ...item,
              key: item.uuid,
              value: item.rootUuid || item.parentUuid,
              label: `${item.name}(${item.endpoint})`,
              isLeaf: true
            });
          });
          this.hostOptions = hostArray;
          if (isEmpty(omit(this.item, 'sub_type'))) {
            each(hostArray, item => {
              if (
                item.extendInfo.scenario === DataMap.proxyHostType.builtin.value
              ) {
                this.poxyOptions.push(item.rootUuid || item.parentUuid);
              }
            });
            this.formGroup.get('proxyHost').setValue(this.poxyOptions);
          }
          return;
        }
        this.getProxyOptions(recordsTemp, startPage);
      });
  }

  validNfsShareName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      return control.value !== '.' && control.value !== '..'
        ? null
        : {
            invalidSpecialPath: { value: control.value }
          };
    };
  }

  setNasNameValid(share) {
    if (share === DataMap.Shared_Mode.nfs.value) {
      this.formGroup
        .get('nas_name')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(512)
        ]);
      assign(this.nasNameErrorTip, {
        invalidName: this.i18n.get('protection_invalid_nasshare_nfs_name_label')
      });
    } else {
      this.formGroup
        .get('nas_name')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(512),
          this.validCifsName()
        ]);
      assign(this.nasNameErrorTip, {
        invalidName: this.i18n.get('protection_invalid_nasshare_name_label')
      });
    }
    this.formGroup.get('nas_name').updateValueAndValidity();
  }

  listernForm() {
    this.formGroup.get('share_mode').valueChanges.subscribe(res => {
      if (res === DataMap.Shared_Mode.nfs.value) {
        this.formGroup
          .get('auth_mode')
          .setValue(DataMap.Nas_Share_Auth_Mode.system.value);
        this.authOptions = this.dataMapService
          .toArray('Nas_Share_Auth_Mode')
          .filter(item => {
            return [
              DataMap.Nas_Share_Auth_Mode.system.value,
              DataMap.Nas_Share_Auth_Mode.kerberos.value
            ].includes(item.value);
          })
          .filter(item => (item.isLeaf = true));
        this.osType = DataMap.Os_Type.linux.value;
      } else {
        this.formGroup
          .get('auth_mode')
          .setValue(DataMap.Nas_Share_Auth_Mode.system.value);
        this.authOptions = this.dataMapService
          .toArray('Nas_Share_Auth_Mode')
          .filter(item => {
            return [
              DataMap.Nas_Share_Auth_Mode.password.value,
              DataMap.Nas_Share_Auth_Mode.kerberos.value
            ].includes(item.value);
          })
          .filter(item => (item.isLeaf = true));
        this.osType = DataMap.Os_Type.linux.value;
      }
      if (isEmpty(omit(this.item, 'sub_type'))) {
        this.setNasNameValid(res);
      }
    });
    this.formGroup.get('auth_mode').valueChanges.subscribe(res => {
      if (res === DataMap.Nas_Share_Auth_Mode.kerberos.value) {
        this.formGroup
          .get('kerberos')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('domain_name').clearValidators();
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
      } else if (res === DataMap.Nas_Share_Auth_Mode.password.value) {
        this.formGroup
          .get('domain_name')
          .setValidators([
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasshareDomain,
              false
            )
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
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup.get('kerberos').clearValidators();
      } else {
        this.formGroup.get('kerberos').clearValidators();
        this.formGroup.get('domain_name').clearValidators();
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
      }
      this.formGroup.get('kerberos').updateValueAndValidity();
      this.formGroup.get('domain_name').updateValueAndValidity();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
    });
  }

  patchValue() {
    if (isEmpty(omit(this.item, 'sub_type'))) {
      return;
    }
    this.formGroup.get('nas_name').setValue(this.item.name);
    this.formGroup.get('nas_name').clearValidators();
    this.formGroup.get('share_mode').setValue(this.item.extendInfo?.shareMode);
    if (!!size(this.items)) {
      this.formGroup.get('ip').setValue('');
      if (this.item.extendInfo?.shareMode === DataMap.Shared_Mode.cifs.value) {
        this.formGroup.get('auth_mode').setValue('');
      }
      return;
    }
    this.formGroup.get('ip').setValue(this.item.extendInfo?.ip);
    if (this.item.extendInfo?.shareMode === DataMap.Shared_Mode.nfs.value) {
      this.formGroup
        .get('auth_mode')
        .setValue(
          this.item.extendInfo?.authMode === '0'
            ? DataMap.Nas_Share_Auth_Mode.kerberos.value
            : DataMap.Nas_Share_Auth_Mode.system.value
        );
      if (this.item.extendInfo?.authMode === '0') {
        this.formGroup
          .get('kerberos')
          .setValue(this.item.extendInfo?.kerberosId);
      }
    } else {
      this.formGroup
        .get('encryption')
        .setValue(this.item.extendInfo?.encryption === '1');
      if (this.item.extendInfo?.authMode === '1') {
        if (!isEmpty(this.item.extendInfo?.kerberosId)) {
          this.formGroup
            .get('auth_mode')
            .setValue(DataMap.Nas_Share_Auth_Mode.kerberos.value);
          this.formGroup
            .get('kerberos')
            .setValue(this.item.extendInfo?.kerberosId);
        } else {
          this.formGroup
            .get('auth_mode')
            .setValue(DataMap.Nas_Share_Auth_Mode.system.value);
        }
      } else {
        this.formGroup
          .get('auth_mode')
          .setValue(DataMap.Nas_Share_Auth_Mode.password.value);
        this.formGroup
          .get('domain_name')
          .setValue(this.item.extendInfo?.domainName);
        this.formGroup.get('username').setValue(this.item.auth?.authKey);
      }
    }
    if (this.exterAgent) {
      if (!isUndefined(this.item.extendInfo?.agents)) {
        this.formGroup
          .get('proxyHost')
          .setValue(this.item.extendInfo?.agents?.split(';'));
      } else {
        this.setProxyValue();
      }
    }
  }

  getKerberos(callback?, recordsTemp?, startPage?) {
    this.kerberosApi
      .queryAllKerberosUsingGET({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        startPage: startPage || CommonConsts.PAGE_START,
        akDoException: false
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
          startPage ===
            Math.ceil(res.total / CommonConsts.PAGE_SIZE_OPTIONS[2]) ||
          res.total === 0
        ) {
          const kerberos = [];
          each(recordsTemp, item => {
            kerberos.push({
              ...item,
              label: item.name,
              isLeaf: true
            });
          });
          this.kerberosOptions = kerberos;
          if (!isUndefined(callback) && isFunction(callback)) {
            callback();
          }
          return;
        }
        this.getKerberos(callback, recordsTemp, startPage);
      });
  }

  createKerberos() {
    const kerberosComponent = new KerberosComponent(
      this.i18n,
      this.drawModalService
    );
    kerberosComponent.create(undefined, kerberos => {
      this.getKerberos(() => {
        this.formGroup.get('kerberos').setValue(kerberos);
      });
    });
  }

  getParams() {
    if (this.formGroup.value.modifyFilter || !size(this.items)) {
      this.filterComponent.collectParams();
    }
    const params = {
      name: this.formGroup.get('nas_name').value,
      path: this.formGroup.get('nas_name').value,
      parentUuid: this.item?.parentUuid,
      type: 'Storage',
      subType: DataMap.Resource_Type.NASShare.value,
      extendInfo: {
        ip: this.formGroup.value.ip,
        shareMode: this.formGroup.value.share_mode,
        authMode:
          this.formGroup.value.auth_mode ===
            DataMap.Nas_Share_Auth_Mode.system.value ||
          (this.formGroup.value.auth_mode ===
            DataMap.Nas_Share_Auth_Mode.kerberos.value &&
            this.formGroup.value.share_mode === DataMap.Shared_Mode.cifs.value)
            ? '1'
            : '0',
        filters: JSON.stringify(this.filterParams),
        isAutoScan: '0'
      }
    };
    if (this.exterAgent) {
      assign(params.extendInfo, {
        agents: this.formGroup.value.proxyHost.join(';')
      });
    }
    if (this.formGroup.value.share_mode === DataMap.Shared_Mode.cifs.value) {
      assign(params.extendInfo, {
        encryption: this.formGroup.value.encryption ? '1' : '0'
      });
    }
    const auth = {
      authType: this.formGroup.value.auth_mode
    };
    if (
      this.formGroup.value.auth_mode ===
      DataMap.Nas_Share_Auth_Mode.kerberos.value
    ) {
      assign(params.extendInfo, {
        kerberosId: this.formGroup.value.kerberos,
        domainName: ''
      });
    } else if (
      this.formGroup.value.auth_mode ===
      DataMap.Nas_Share_Auth_Mode.password.value
    ) {
      assign(params.extendInfo, {
        domainName: this.formGroup.value.domain_name,
        kerberosId: ''
      });
      assign(auth, {
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password
      });
    } else {
      assign(params.extendInfo, {
        domainName: '',
        kerberosId: ''
      });
    }
    assign(params, { auth });
    return params;
  }

  register(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      assign(params, {
        parentUuid: ''
      });
      this.protectedResourceApiService
        .CreateResource({ CreateResourceRequestBody: params })
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
    });
  }

  edit(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.item.uuid,
          UpdateResourceRequestBody: this.getParams()
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  batchEdit(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      this.batchOperateService.selfGetResults(
        item => {
          const params = assign(this.getParams(), {
            name: item.name,
            path: item.path
          });
          if (!this.formGroup.value.modifyFilter && !!size(this.items)) {
            params['extendInfo']['filters'] = item['extendInfo']['filters'];
          }
          return this.protectedResourceApiService.UpdateResource({
            resourceId: item.uuid,
            UpdateResourceRequestBody: params,
            akDoException: false,
            akOperationTips: false,
            akLoading: false
          });
        },
        this.items,
        () => {},
        '',
        false,
        5
      );
      observer.next();
      observer.complete();
    });
  }

  onOK(): Observable<void> {
    return isEmpty(omit(this.item, 'sub_type'))
      ? this.register()
      : !!size(this.items)
      ? this.batchEdit()
      : this.edit();
  }

  validCifsName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      if (!CommonConsts.REGEX.nasShareCifsName.test(control.value)) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }
}
