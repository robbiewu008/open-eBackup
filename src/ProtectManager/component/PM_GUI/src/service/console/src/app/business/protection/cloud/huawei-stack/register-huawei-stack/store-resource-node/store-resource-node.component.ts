import { Component, OnInit } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  MessageService,
  ModalRef,
  UploadFile,
  UploadFileStatusEnum
} from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { TableData } from 'app/shared/components/pro-table';
import {
  assign,
  defer,
  each,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNil,
  size,
  toString as _toString,
  trim
} from 'lodash';
import { Observable, Observer, of } from 'rxjs';
@Component({
  selector: 'aui-store-resource-node',
  templateUrl: './store-resource-node.component.html',
  styleUrls: ['./store-resource-node.component.less']
})
export class StorResourceNodeComponent implements OnInit {
  includes = includes;
  formGroup: FormGroup;
  data;
  subType;
  dataMap = DataMap;
  fcCertFilters = [];
  selectFcSiteFile = '';
  revocationListFilters = [];
  selectRevocationList = '';
  tableData: TableData;
  value1 = 0;
  value2 = 1;

  certName = '';
  certSize = '';

  certFiles = [];

  revocationFiles = [];
  crlName = '';
  crlSize = '';

  ipRepeat = false;
  repeatTips;
  isOracle = false;
  deviceTypeOptions = this.dataMapService
    .toArray('Device_Storage_Type')
    .filter(v => {
      v.isLeaf = true;
      return (
        v.value === DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value
      );
    });
  protocolOptions = this.dataMapService
    .toArray('dataProtocolType')
    .filter(v => (v.isLeaf = true));
  centralizedLabel = this.i18n.get('common_san_storage_label');

  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  constructor(
    public i18n: I18NService,
    private modal: ModalRef,
    private fb: FormBuilder,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public message: MessageService
  ) {}

  ngOnInit() {
    this.isOracle = this.subType === DataMap.Resource_Type.oracle.value;
    this.initForm();
    this.updateData();
    this.initFilters();
  }

  initForm() {
    this.formGroup = this.fb.group({
      ips: this.fb.array([
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        })
      ]),
      port: new FormControl('8088', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      storageType: new FormControl(
        this.subType === DataMap.Resource_Type.vmware.value
          ? DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value
          : this.value1
      ),
      cert: new FormControl(true),
      transport_protocol: new FormControl(first(this.protocolOptions).value),
      deviceType: new FormControl(first(this.deviceTypeOptions).value),
      isVbsNodeInfo: new FormControl(false)
    });

    this.formGroup.statusChanges.subscribe(() =>
      defer(() => this.disableOkBtn())
    );
    this.formGroup.get('cert')?.valueChanges.subscribe(res => {
      this.selectFcSiteFile = ''; // 清空
      this.selectRevocationList = '';
    });
    this.formGroup.get('ips').statusChanges.subscribe(() => {
      defer(() => this.validReatIp());
    });
    this.formGroup.get('isVbsNodeInfo')?.valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.addControl(
          'vbsNodeUserName',
          new FormControl('', [this.baseUtilService.VALID.name()])
        );
        this.formGroup.addControl(
          'vbsNodeIp',
          new FormControl('', [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ipv4()
          ])
        );
        this.formGroup.addControl(
          'vbsNodePort',
          new FormControl('22', [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ])
        );
        this.formGroup.addControl(
          'vbsNodePassword',
          new FormControl('', [this.baseUtilService.VALID.required()])
        );
      } else {
        this.formGroup.removeControl('vbsNodeUserName');
        this.formGroup.removeControl('vbsNodeIp');
        this.formGroup.removeControl('vbsNodePort');
        this.formGroup.removeControl('vbsNodePassword');
      }
    });
    if (
      this.subType === DataMap.Resource_Type.vmware.value &&
      isEmpty(this.data)
    ) {
      this.formGroup.get('storageType').valueChanges.subscribe(res => {
        if (res === DataMap.Device_Storage_Type.NetApp.value) {
          this.formGroup.get('port').setValue('443');
        } else {
          this.formGroup.get('port').setValue('8088');
        }
      });
    }
  }

  validReatIp() {
    const ips = this.formGroup.value.ips;
    const allIps = [];
    const repeatIps = [];
    each(ips, ip => {
      if (!trim(ip)) {
        return;
      }
      if (!includes(allIps, ip)) {
        allIps.push(ip);
      } else {
        repeatIps.push(ip);
      }
    });
    if (repeatIps.length) {
      this.ipRepeat = true;
      this.repeatTips = this.i18n.get('common_same_ip_tips_label', [
        repeatIps.join(this.i18n.isEn ? ',' : '，')
      ]);
    } else {
      this.ipRepeat = false;
    }
  }

  get ipArr() {
    return (this.formGroup.get('ips') as FormArray).controls;
  }

  addIp() {
    (this.formGroup.get('ips') as FormArray).push(
      new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      })
    );
  }

  deleteIp(i) {
    (this.formGroup.get('ips') as FormArray).removeAt(i);
  }

  updateData() {
    if (isEmpty(this.data)) {
      return;
    }
    if (isEmpty(this.data.ip)) {
      assign(this.data, {
        ip: this.data.ipList
      });
    }
    const showData = [this.data];
    this.formGroup.patchValue(showData[0]);
    if (this.subType === DataMap.Resource_Type.vmware.value) {
      this.formGroup
        .get('storageType')
        .setValue(
          this.data.storageType ||
            DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value
        );
    } else {
      this.formGroup
        .get('storageType')
        .setValue(this.data.storageType === this.centralizedLabel ? 0 : 1);
    }
    (this.formGroup.get('ips') as FormArray).clear();
    if (isArray(this.data.ip)) {
      each(this.data.ip, () => this.addIp());
    } else {
      each(this.data.ip.split(','), () => this.addIp());
    }
    this.formGroup
      .get('ips')
      .setValue(isArray(this.data.ip) ? this.data.ip : this.data.ip.split(','));
    defer(() => {
      if (!isNil(this.data.enableCert)) {
        this.formGroup.get('cert').setValue(this.data.enableCert == '1');
      }
      if (!isNil(this.data.isVbsNodeInfo)) {
        this.formGroup.get('isVbsNodeInfo').setValue(this.data.isVbsNodeInfo);
      }
      if (!isEmpty(this.data?.certification)) {
        this.selectFcSiteFile = this.data?.certification;
      }
      if (!isEmpty(this.data?.revocationList)) {
        this.selectRevocationList = this.data?.revocationList;
      }
      if (!isEmpty(this.data.certName)) {
        this.certFiles = [
          {
            key: '-1',
            name: this.data.certName,
            fileSize: this.data.certSize,
            status: UploadFileStatusEnum.SUCCESS
          }
        ];

        this.certName = this.data.certName;
        this.certSize = this.data.certSize;
      }
      if (!isEmpty(this.data.crlName)) {
        this.revocationFiles = [
          {
            key: '-1',
            name: this.data.crlName,
            fileSize: this.data.crlSize,
            status: UploadFileStatusEnum.SUCCESS
          }
        ];

        this.crlName = this.data.crlName;
        this.crlSize = this.data.crlSize;
      }
      this.disableOkBtn();
    });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.value.cert
      ? !(
          this.formGroup.status === 'VALID' &&
          (this.certName || this.selectFcSiteFile) &&
          !this.ipRepeat
        )
      : this.formGroup.status !== 'VALID' || this.ipRepeat;
  }

  certFilesChange(files) {
    if (size(files) === 0) {
      each(['certName', 'certSize'], key => (this[key] = ''));
    } else {
      this.certName = get(first(files), 'name');
      this.certSize = get(first(files), 'fileSize');
    }
    this.disableOkBtn();
  }

  revocationFilesChange(files) {
    if (size(files) === 0) {
      each(['crlName', 'crlSize'], key => (this[key] = ''));
    } else {
      this.crlName = get(first(files), 'name');
      this.crlSize = get(first(files), 'fileSize');
    }
    this.disableOkBtn();
  }

  cartChange(e) {
    e?.action === 'remove' && this._certClear();
  }

  revocationChange(e) {
    e?.action === 'remove' && this._revocationClear();
  }

  private _certClear() {
    this.certFiles = [];
    each(['selectFcSiteFile', 'certName', 'certSize'], key => {
      this[key] = void 0;
    });
    this.disableOkBtn();
    return this;
  }

  private _revocationClear() {
    this.revocationFiles = [];
    each(['selectRevocationList', 'crlName', 'crlSize'], key => {
      this[key] = '';
    });
    this.disableOkBtn();
    return this;
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
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );

            this.selectFcSiteFile = ''; // 清空
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
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
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              (!this.selectFcSiteFile && !this.certName) ||
              this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  onOK() {
    if (this.formGroup.invalid) {
      return of(null);
    }
    let existItem;
    if (!this.isOracle) {
      // oracle存储资源不需要重复校验
      if (isEmpty(this.data)) {
        existItem = this.tableData?.data.find(
          item =>
            _toString(item.ip) === _toString(this.formGroup.value.ips) &&
            item.port === +this.formGroup.value.port
        );
      } else {
        existItem = this.tableData?.data.find(
          item =>
            _toString(item.ip) === _toString(this.formGroup.value.ips) &&
            item.port === +this.formGroup.value.port &&
            !(
              _toString(item.ip) === _toString(this.data.ip) &&
              item.port === this.data.port
            )
        );
      }
    }
    if (!isEmpty(existItem)) {
      this.message.error(this.i18n.get('common_storage_resource_exist_label'), {
        lvMessageKey: 'errorKey',
        lvShowCloseButton: true
      });
      return of(null);
    }

    return new Observable<any>((observer: Observer<any>) => {
      this.data = {
        username: this.formGroup.value.username,
        password: this.formGroup.value.password,
        port: +this.formGroup.value.port,
        ip: this.formGroup.value.ips,
        enableCert: String(+this.formGroup.value.cert),
        certification: this.selectFcSiteFile,
        revocationList: this.selectRevocationList,
        certName: this.certName,
        certSize: this.certSize,
        crlName: this.crlName,
        crlSize: this.crlSize,
        storageType: this.formGroup.value.storageType,
        isVbsNodeInfo: this.formGroup.value.isVbsNodeInfo,
        vbsNodeUserName: this.formGroup.value.vbsNodeUserName,
        vbsNodeIp: this.formGroup.value.vbsNodeIp,
        vbsNodePort: this.formGroup.value.vbsNodePort,
        vbsNodePassword: this.formGroup.value.vbsNodePassword
      };
      if (this.isOracle) {
        assign(this.data, {
          transport_protocol: this.formGroup.value.transport_protocol,
          device_type: this.formGroup.value.deviceType,
          ipList: this.formGroup.value.ips.join(',')
        });
        delete this.data.ip;
      }
      observer.next(this.data);
      observer.complete();
    });
  }
}
