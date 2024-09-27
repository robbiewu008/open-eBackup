import { DatePipe } from '@angular/common';
import {
  Component,
  ElementRef,
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
  ValidatorFn,
  Validators
} from '@angular/forms';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { Router } from '@angular/router';
import {
  DatatableComponent,
  MenuItem,
  MessageboxService,
  ModalService,
  OptionItem
} from '@iux/live';
import {
  CommonConsts,
  DataMap,
  EnvironmentsService,
  SnmpApiService,
  MODAL_COMMON,
  ClientManagerApiService
} from 'app/shared';
import {
  BaseUtilService,
  DataMapService,
  GlobalService,
  I18NService
} from 'app/shared/services';
import { assign, isNull, isString, isUndefined, toUpper } from 'lodash';
import { of, Subject } from 'rxjs';
import { map } from 'rxjs/operators';
import { WarningMessageService } from './../../../../shared/services/warning-message.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SnmpV3EngineComponent } from './snmp-v3-engine/snmp-v3-engine.component';

@Component({
  selector: 'aui-app-add-trap-ip',
  template: `
    <lv-form
      [formGroup]="formGroupAddTrapIp"
      [lvLabelColon]="false"
      class="formGroup"
    >
      <lv-form-item>
        <lv-form-label lvRequired>{{ ipAdressLabel }}</lv-form-label>
        <lv-form-control [lvErrorTip]="baseUtilService.ipErrorTip">
          <input lv-input type="text" formControlName="trapIp" />
        </lv-form-control>
      </lv-form-item>
      <lv-form-item>
        <lv-form-label lvRequired>{{ portLabel }}</lv-form-label>
        <lv-form-control
          [lvErrorTip]="
            extend({}, baseUtilService.rangeErrorTip, {
              invalidRang: 'common_valid_rang_label' | i18n: [1, 65535]
            })
          "
        >
          <input
            lv-input
            type="text"
            formControlName="port"
            placeholder="1~65535"
          />
        </lv-form-control>
      </lv-form-item>
      <lv-form-item>
        <lv-form-label>{{ descLabel }}</lv-form-label>
        <lv-form-control>
          <lv-input-lint>
            <textarea
              formControlName="description"
              rows="3"
              maxlength="128"
              lv-input
            ></textarea>
          </lv-input-lint>
        </lv-form-control>
      </lv-form-item>
    </lv-form>
  `
})
export class AddTrapIpComponent implements OnInit {
  extend = assign;
  @Input() data;
  formGroupAddTrapIp: FormGroup;

  typeLabel = this.i18n.get('common_type_label');
  descLabel = this.i18n.get('common_desc_label');
  portLabel = this.i18n.get('common_port_label');
  ipAdressLabel = this.i18n.get('common_ip_address_label');

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroupAddTrapIp = this.fb.group({
      trapIp: new FormControl(this.data ? this.data.trapIp : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      }),
      port: new FormControl(this.data ? this.data.port || 162 : 162, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      description: this.data ? this.data.description : ''
    });
  }
}

@Component({
  selector: 'snmp-trap',
  templateUrl: './snmp-trap.component.html',
  providers: [DatePipe]
})
export class SnmpTrapComponent implements OnInit {
  successLabel: SafeHtml;
  authProtocolData: OptionItem[];
  encryptProtocolData: OptionItem[];
  versionData: OptionItem[];
  formGroup: FormGroup;
  search$ = new Subject<string>();
  addressData = [];
  trapConfigData;
  queryIP: any;
  queryPort: any;
  isView = true;
  version;
  protocolVersion = DataMap.Protocol_Version;
  hasNewAuthPwd = false;
  hasNewencryptPwd = false;
  hasNewencryptConfirmPwd = false;
  hasNewauthConfirmPwd = false;
  hasNewSecurityNameV2C = false;
  formGroupItms = [];
  formGroupItmsRight = [];
  formGroupItmsLeft = [];
  parame = { orderBy: 'PORT', orderType: 'ASC' };
  queryIpObject;
  queryPortObject;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE;
  pageIndex = CommonConsts.PAGE_START;

  @ViewChild('addTarpIpComponent', { static: false })
  addTarpIpComponent: TemplateRef<any>;
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild('successContentTpl', { static: false })
  successContentTpl: TemplateRef<any>;

  communityTipLabel = this.i18n.get('system_snmp_pwd_label', [
    15,
    64,
    this.i18n.get('common_pwd_complex_label'),
    ''
  ]);
  pwdComplexTipLabel = this.i18n.get('system_snmp_pwd_label', [
    15,
    64,
    this.i18n.get('common_pwd_complex_label'),
    this.i18n.get('system_snmp_pwd_username_same_label')
  ]);
  viewEngineLabel = this.i18n.get('system_view_snmp_v3_engine_id_label');
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  engineId: string;
  engineIdLabel = this.i18n.get('system_engine_id_label', ['--']);

  constructor(
    public router: Router,
    public fb: FormBuilder,
    public i18n: I18NService,
    public datePipe: DatePipe,
    private sanitizer: DomSanitizer,
    public modalService: ModalService,
    public globalService: GlobalService,
    public messageBox: MessageboxService,
    public snmpApiService: SnmpApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    public warningMessageService: WarningMessageService,
    private clientManagerApiService: ClientManagerApiService
  ) {
    this.successLabel = this.sanitizer.bypassSecurityTrustHtml(
      this.i18n.get('system_setting_trap_info_label')
    );
  }

  trap = this.i18n.get('system_trap');
  addLabel = this.i18n.get('common_add_label');
  parameterConfig = this.i18n.get('system_trap_config_label');
  parameterCOnfigTip = this.i18n.get('system_trap_config_tip_label');
  environmentName = this.i18n.get('system_trap_environment_name_label');
  trapIpAdress = this.i18n.get('system_trap_ip_adress_label');
  trapSetting = this.i18n.get('system_snmp_trap_label');
  trapTypeLable = this.i18n.get('system_trap_type_label');
  authProtocol = this.i18n.get('system_trap_authorized_agreement_label');
  encryptProtocol = this.i18n.get('system_trap_data_encryption_protocol_label');
  contextEngineId = this.i18n.get('system_trap_environment_engine_id_label');
  authPwd = this.i18n.get('system_trap_authorization_password_label');
  confirmAuthPwd = this.i18n.get('system_trap_auth_confirm_password_label');
  encryptPwd = this.i18n.get('system_trap_data_encryption_password_label');
  confirmEncryptPwd = this.i18n.get(
    'system_trap_encryption_confirm_password_label'
  );
  addTrapIpAdressHeader = this.i18n.get('system_trap_add_ip_adress_label');
  modifyTrapIpAdressHeader = this.i18n.get(
    'system_trap_modify_ip_adress_label'
  );
  warnLabel = this.i18n.get('common_warn_label');
  typeLabel = this.i18n.get('common_type_label');
  versionLabel = this.i18n.get('system_protocol_version_label');
  usernameLabel = this.i18n.get('common_username_label');
  editLabel = this.i18n.get('common_modify_label');
  saveLabel = this.i18n.get('common_save_label');
  cancelLabel = this.i18n.get('common_cancel_label');
  deleteLabel = this.i18n.get('common_delete_label');
  ipAdressLabel = this.i18n.get('common_ip_address_label');
  portLabel = this.i18n.get('common_port_label');
  optLabel = this.i18n.get('common_operation_label');
  descLabel = this.i18n.get('common_desc_label');
  communityLabel = this.i18n.get('common_community_label');
  securityNameV2CErrorTip = assign(
    {
      ...this.baseUtilService.requiredErrorTip,
      ...this.baseUtilService.pwdErrorTip
    },
    {
      invalidMinLength: this.i18n.get('common_valid_minlength_label', [15]),
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
    }
  );
  securityNameErrorTip = assign(
    {
      ...this.baseUtilService.requiredErrorTip
    },
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
    }
  );
  contextEngineIdErrorTip = {
    invalidName: this.baseUtilService.invalidInputLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pwdErrorTip = assign(
    {
      ...this.baseUtilService.pwdErrorTip
    },
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64]),
      invalidMinLength: this.i18n.get('common_valid_minlength_label', [15])
    }
  );

  ngOnInit() {
    this.initForm();
    this.initAddressData();
    this.getEngineId();
  }

  onChange() {
    this.ngOnInit();
  }

  initForm() {
    this.initSelectMethod();
    this.snmpApiService.getTrapConfigUsingGET({}).subscribe(res => {
      this.trapConfigData = res;
      this.version = res.version;
      this.setFormGroup(res);
    });
  }

  setFormGroup(config) {
    this.formGroup = this.fb.group({
      version: new FormControl(config.version || '', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });

    this.formGroup.get('version').valueChanges.subscribe(res => {
      if (res === this.protocolVersion.V3.value) {
        this.addV3Control(config);
        this.removeV2CControl();
      } else if (res === this.protocolVersion.V2C.value) {
        this.addV2CControl(config);
        this.removeV3Control();
      }
      this.version = res;
    });

    if (config.version === this.protocolVersion.V3.value) {
      this.initV3FormItems(config);
      this.addV3Control(config);
      this.removeV2CControl();
    } else if (config.version === this.protocolVersion.V2C.value) {
      this.initV2CFormItems(config);
      this.addV2CControl(config);
      this.removeV3Control();
    } else {
      this.formGroupItmsLeft = [
        {
          label: this.versionLabel,
          content: ''
        }
      ];
      return;
    }
    this.formGroupItmsRight = this.formGroupItms.slice(5);
    this.formGroupItmsLeft = this.formGroupItms.slice(0, 5);
  }

  initV3FormItems(config: any) {
    this.formGroupItms = [
      {
        label: this.versionLabel,
        content: config.version
          ? this.versionData.find(item => item.value === config.version)
            ? this.versionData.find(item => item.value === config.version).label
            : ''
          : ''
      },
      {
        label: this.usernameLabel,
        content: config.securityName
      },
      {
        label: this.authProtocol,
        content:
          config.authProtocol === 'HMACMD5'
            ? 'HMAC_MD5'
            : (isNull(config.authProtocol)
                ? 'HMAC_SHA2'
                : config.authProtocol) || this.i18n.get('common_none_label')
      },
      {
        label: this.authPwd,
        content: !!config.authProtocol ? '***************' : '--'
      },
      {
        label: this.confirmAuthPwd,
        content: !!config.authProtocol ? '***************' : '--'
      },
      {
        label: this.encryptProtocol,
        content:
          (isNull(config.encryptProtocol) ? 'AES' : config.encryptProtocol) ||
          this.i18n.get('common_none_label')
      },
      {
        label: this.encryptPwd,
        content: !!config.encryptProtocol ? '***************' : '--'
      },
      {
        label: this.confirmEncryptPwd,
        content: !!config.encryptProtocol ? '***************' : '--'
      },
      {
        label: this.environmentName,
        content: config.contextName
      },
      {
        label: this.contextEngineId,
        content: config.contextEngineId
      }
    ];
  }

  initV2CFormItems(config: any) {
    this.formGroupItms = [
      {
        label: this.versionLabel,
        content: config.version
          ? this.versionData.find(item => item.value === config.version)
            ? this.versionData.find(item => item.value === config.version).label
            : ''
          : ''
      },
      {
        label: this.communityLabel,
        content: isNull(config.securityNameV2C) ? '--' : '***************'
      }
    ];
  }

  addV3Control(config: any) {
    this.formGroup.addControl(
      'securityName',
      new FormControl(config.securityName, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl(
      'contextName',
      new FormControl(config.contextName, {
        validators: [this.baseUtilService.VALID.maxLength(64)],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl(
      'authProtocol',
      new FormControl(
        isNull(config.authProtocol)
          ? 'HMAC_SHA2'
          : config.authProtocol === ''
          ? 'None'
          : config.authProtocol,
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
    this.formGroup.addControl(
      'encryptProtocol',
      new FormControl(
        isNull(config.encryptProtocol)
          ? 'AES'
          : config.encryptProtocol === ''
          ? 'None'
          : config.encryptProtocol,
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
    this.formGroup.addControl(
      'contextEngineId',
      new FormControl(config.contextEngineId, {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.contextEngineId,
            false
          ),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl(
      'authPwd',
      new FormControl(
        this.trapConfigData.id && config.authProtocol ? '***************' : '',
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
    this.formGroup.addControl(
      'authConfirmPwd',
      new FormControl(
        this.trapConfigData.id && config.authProtocol ? '***************' : '',
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
    this.formGroup.addControl(
      'encryptPwd',
      new FormControl(
        this.trapConfigData.id && config.encryptProtocol
          ? '***************'
          : '',
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
    this.formGroup.addControl(
      'encryptConfirmPwd',
      new FormControl(
        this.trapConfigData.id && config.encryptProtocol
          ? '***************'
          : '',
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );

    this.formGroup.get('authProtocol').valueChanges.subscribe(res => {
      if (res === 'None') {
        this.formGroup.get('authPwd').disable();
        this.formGroup.get('authPwd').setValue('');
        this.formGroup.get('authConfirmPwd').disable();
        this.formGroup.get('authConfirmPwd').setValue('');
        this.formGroup.get('encryptProtocol').setValue('None');
      } else {
        this.formGroup.get('authPwd').enable();
        this.formGroup.get('authConfirmPwd').enable();
      }
    });

    this.formGroup.get('encryptProtocol').valueChanges.subscribe(res => {
      if (res === 'None') {
        this.formGroup.get('encryptPwd').disable();
        this.formGroup.get('encryptPwd').setValue('');
        this.formGroup.get('encryptConfirmPwd').disable();
        this.formGroup.get('encryptConfirmPwd').setValue('');
      } else {
        this.formGroup.get('encryptPwd').enable();
        this.formGroup.get('encryptConfirmPwd').enable();
      }
    });

    if (this.formGroup.get('authProtocol').value === 'None') {
      this.formGroup.get('authPwd').disable();
      this.formGroup.get('authConfirmPwd').disable();
      this.formGroup.get('encryptProtocol').setValue('None');
    } else {
      this.formGroup.get('authPwd').enable();
      this.formGroup.get('authConfirmPwd').enable();
    }

    if (this.formGroup.get('encryptProtocol').value === 'None') {
      this.formGroup.get('encryptPwd').disable();
      this.formGroup.get('encryptConfirmPwd').disable();
    } else {
      this.formGroup.get('encryptPwd').enable();
      this.formGroup.get('encryptConfirmPwd').enable();
    }
  }

  removeV3Control() {
    this.formGroup.removeControl('securityName');
    this.formGroup.removeControl('contextName');
    this.formGroup.removeControl('authProtocol');
    this.formGroup.removeControl('encryptProtocol');
    this.formGroup.removeControl('contextEngineId');
    this.formGroup.removeControl('authPwd');
    this.formGroup.removeControl('authConfirmPwd');
    this.formGroup.removeControl('encryptPwd');
    this.formGroup.removeControl('encryptConfirmPwd');
  }

  addV2CControl(config: any) {
    this.formGroup.addControl(
      'securityNameV2C',
      new FormControl(
        isNull(config.securityNameV2C)
          ? ''
          : this.hasNewSecurityNameV2C
          ? ''
          : '***************',
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      )
    );
  }

  removeV2CControl() {
    this.formGroup.removeControl('securityNameV2C');
  }

  initSelectMethod() {
    this.authProtocolData = [
      {
        value: 'None',
        label: this.i18n.get('common_none_label'),
        isLeaf: true
      },
      {
        value: 'HMACMD5',
        label: 'HMAC_MD5',
        isLeaf: true
      },
      {
        value: 'HMAC_SHA1',
        label: 'HMAC_SHA1',
        isLeaf: true
      },
      {
        value: 'HMAC_SHA2',
        label: 'HMAC_SHA2',
        isLeaf: true
      }
    ];
    this.encryptProtocolData = [
      {
        value: 'None',
        label: this.i18n.get('common_none_label'),
        isLeaf: true
      },
      {
        value: 'DES',
        label: 'DES',
        isLeaf: true
      },
      {
        value: 'AES',
        label: 'AES',
        isLeaf: true
      }
    ];
    this.versionData = this.dataMapService
      .toArray('Protocol_Version')
      .filter(item => (item.isLeaf = true));
  }

  getHosts() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: 1,
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.oracle.value}Plugin`,
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      })
    };
    // 安全一体机不需要查询主机
    if (this.isCyberEngine) return of(false);
    return this.clientManagerApiService.queryAgentListInfoUsingGET(params).pipe(
      map(res => {
        return !!res.records.length;
      })
    );
  }

  save() {
    this.getHosts().subscribe(res => {
      const params = this.formGroup.value;
      if (!this.formGroup.valid) {
        return;
      }
      if (!this.hasNewAuthPwd) {
        params.authPwd = '';
      }

      if (!this.hasNewencryptPwd) {
        params.encryptPwd = '';
      }

      if (!this.hasNewSecurityNameV2C) {
        params.securityNameV2C = '';
      }

      if (params.encryptProtocol === 'None') {
        params.encryptProtocol = '';
      }

      if (params.authProtocol === 'None') {
        params.authProtocol = '';
      }

      if (this.formGroup.value.version) {
        this.version = this.formGroup.value.version;
      }

      const snmpRequest = assign(this.trapConfigData, params);
      delete snmpRequest.authConfirmPwd;
      delete snmpRequest.encryptConfirmPwd;

      if (DataMap.Protocol_Version.V3.value === this.version) {
        for (const key in snmpRequest) {
          if (['securityNameV2C'].includes(key)) {
            snmpRequest[key] = null;
          }
        }
      } else {
        for (const key in snmpRequest) {
          if (!['securityNameV2C', 'version', 'id'].includes(key)) {
            snmpRequest[key] = null;
          }
        }
      }
      this.snmpApiService
        .modifyTrapConfigUsingPUT({ snmpRequest, akOperationTips: !res })
        .subscribe(() => {
          this.cancel();
          if (res) {
            this.messageBox.success({
              lvOkText: this.i18n.get('common_close_label'),
              lvContent: this.successContentTpl,
              lvAfterOpen: () => this.jumpHostsView()
            });
          }
        });
    });
  }

  jumpHostsView() {
    const openDeviceDom = document.querySelector('#open-host');
    if (!openDeviceDom) {
      return;
    }
    openDeviceDom.addEventListener('click', () => {
      this.router.navigateByUrl('/protection/host');
    });
  }

  cancel() {
    this.isView = !this.isView;
    this.initForm();
    this.hasNewAuthPwd = false;
    this.hasNewauthConfirmPwd = false;
    this.hasNewencryptPwd = false;
    this.hasNewencryptConfirmPwd = false;
    this.hasNewSecurityNameV2C = false;
  }

  modify() {
    this.isView = !this.isView;
  }

  focusAuthPwd() {
    if (!this.hasNewAuthPwd) {
      this.hasNewAuthPwd = true;
      this.formGroup.get('authPwd').setValue('');
    }
    this.formGroup
      .get('authPwd')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(15, 2, 64, 0),
        this.validPwdIsSame('authConfirmPwd'),
        this.validUserNameIsSame('securityName')
      ]);
  }

  focusAuthConfirmPwd() {
    if (!this.hasNewauthConfirmPwd) {
      this.hasNewauthConfirmPwd = true;
      this.formGroup.get('authConfirmPwd').setValue('');
    }
    this.formGroup
      .get('authConfirmPwd')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(15, 2, 64, 0),
        this.validPwdIsSame('authPwd'),
        this.validUserNameIsSame('securityName')
      ]);
  }

  focusEncryptPwd() {
    if (!this.hasNewencryptPwd) {
      this.hasNewencryptPwd = true;
      this.formGroup.get('encryptPwd').setValue('');
    }
    this.formGroup
      .get('encryptPwd')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(15, 2, 64, 0),
        this.validPwdIsSame('encryptConfirmPwd'),
        this.validUserNameIsSame('securityName')
      ]);
  }

  focusEncryptConfirmPwd() {
    if (!this.hasNewencryptConfirmPwd) {
      this.hasNewencryptConfirmPwd = true;
      this.formGroup.get('encryptConfirmPwd').setValue('');
    }
    this.formGroup
      .get('encryptConfirmPwd')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(15, 2, 64, 0),
        this.validPwdIsSame('encryptPwd'),
        this.validUserNameIsSame('securityName')
      ]);
  }

  hasNewSecurityNameV2CHandle() {
    if (!this.hasNewSecurityNameV2C) {
      this.hasNewSecurityNameV2C = true;
      this.formGroup.get('securityNameV2C').setValue('');
    }
    this.formGroup
      .get('securityNameV2C')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.password(15, 2, 64, 0)
      ]);
    this.formGroup.get('securityNameV2C').updateValueAndValidity();
  }

  validPwdIsSame(key: string): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value[key] &&
        this.formGroup.value[key] !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value[key]) {
        this.formGroup.get(key).setErrors(null);
      }

      return null;
    };
  }

  validUserNameIsSame(key: string): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value[key] &&
        this.formGroup.value[key] === control.value
      ) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
  }

  initAddressData() {
    this.snmpApiService
      .queryTrapAddressListUsingGET(this.parame as any)
      .subscribe(res => {
        this.addressData = res;
      });
  }

  addTrapIpAdress(data) {
    this.modalService.create({
      lvModalKey: 'addTrapIpModal',
      lvType: 'drawer',
      lvWidth: 600,
      lvDrawerPosition: 'right',
      lvDrawerPositionOffset: CommonConsts.DRAWER_OFFSET,
      lvHeader:
        data === 'right'
          ? this.addTrapIpAdressHeader
          : this.modifyTrapIpAdressHeader,
      lvCloseButtonDisplay: false,
      lvContent: AddTrapIpComponent,
      lvComponentParams: { data },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddTrapIpComponent;
        const modalIns = modal.getInstance();
        modalIns.lvOkDisabled = true;
        content.formGroupAddTrapIp.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as AddTrapIpComponent;
        if (data === 'right') {
          this.snmpApiService
            .addTrapAddressUsingPOST({
              snmpTrapAddress: content.formGroupAddTrapIp.value
            })
            .subscribe(res => this.initAddressData());
        } else {
          const parameData = {
            ...content.formGroupAddTrapIp.value,
            id: content.data.id
          };
          this.snmpApiService
            .modfiyTrapAddressUsingPUT({
              snmpTrapAddress: parameData
            })
            .subscribe(res => this.initAddressData());
        }
      }
    });
  }

  deleteTrapIpAdress(source) {
    this.warningMessageService.create({
      content: this.i18n.get('system_trap_delete_trap_adress_label', [
        source.trapIp
      ]),
      onOK: () =>
        this.snmpApiService
          .deleteTrapAddressUsingDELETE({ id: source.id })
          .subscribe(res => this.initAddressData())
    });
  }

  changeSort(sortData: any) {
    this.parame.orderType = toUpper(sortData.direction);
    this.parame.orderBy = toUpper(sortData.key);
    this.initAddressData();
  }

  optsCallback: (data) => Array<MenuItem> = data => {
    return [
      {
        id: 'delete',
        label: this.deleteLabel,
        onClick: (d: any) => {
          this.deleteTrapIpAdress(data);
        }
      },
      {
        id: 'modify',
        label: this.editLabel,
        onClick: (d: any) => {
          this.snmpApiService
            .queryTrapAddressUsingGET({ id: data.id })
            .subscribe(res => this.addTrapIpAdress(res));
        }
      }
    ];
  };

  searchByIpOrPort(e, type) {
    if (e) {
      this.addressData = this.addressData.filter(res => {
        if (type === 'ip') {
          return res.trapIp.includes(e);
        }
        if (type === 'port') {
          return Number(String(res.port).includes(e));
        }
      });
    }
  }

  viewSnmpV3Engine() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_view_snmp_v3_engine_id_label'),
        lvContent: SnmpV3EngineComponent
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  getEngineId() {
    if (this.engineId || !this.isCyberEngine) {
      return;
    }
    this.snmpApiService.queryAllSnmpSecurityAgentUsingGET({}).subscribe(res => {
      this.engineIdLabel = this.i18n.get('system_engine_id_label', [
        isString(res[0]?.engineId)
          ? res[0]?.engineId.replace(/:/g, '')
          : res[0]?.engineId
      ]);
    });
  }
}
