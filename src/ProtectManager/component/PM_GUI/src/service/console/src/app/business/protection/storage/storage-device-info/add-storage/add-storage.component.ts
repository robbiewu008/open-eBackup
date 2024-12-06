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
  ClientManagerApiService,
  PluginsStorageService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
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
  isFunction,
  map,
  pick
} from 'lodash';

@Component({
  selector: 'aui-add-storage',
  templateUrl: './add-storage.component.html',
  styleUrls: ['./add-storage.component.less']
})
export class AddStorageComponent implements OnInit {
  includes = includes;
  item;
  @Input() refresh;
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
      DataMap.Deploy_Type.e6000.value,
      DataMap.Deploy_Type.decouple.value
    ],
    this.i18n.get('deploy_type')
  );
  isAgentExternal = false;
  externalAgentLists = [];
  poxyOptions = [];
  replicaPortOptions = [];
  isTest = false;
  enableBtn = false;
  testLoading = false;
  tableConfig: TableConfig;
  tableData: TableData;
  isTestSevice = [
    this.deviceStorageType.DoradoV7.value,
    this.deviceStorageType.OceanStorDoradoV7.value,
    this.deviceStorageType.OceanStorDorado_6_1_3.value,
    this.deviceStorageType.OceanStor_6_1_3.value,
    this.deviceStorageType.OceanProtect.value
  ];
  isShow = false;

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
  taskErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 64])
  };
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  @ViewChild('modalFooter', { static: true }) modalFooter: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

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
    private appUtilsService: AppUtilsService,
    private pluginsStorageService: PluginsStorageService
  ) {}

  ngOnInit() {
    this.initFooter();
    this.initForm();
    this.initFilters();
    this.updateData();
  }

  initFooter() {
    this.modal.setProperty({ lvFooter: this.modalFooter });
  }

  enableBtnFn() {
    defer(() => {
      if (this.formGroup.value.verify_status) {
        this.enableBtn = !this.formGroup.invalid && this.selectFcSiteFile;
      } else {
        this.enableBtn = !this.formGroup.invalid;
      }
    });
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
      proxyHost: new FormControl([]),
      isSelected: new FormControl(false),
      selectedIp: new FormControl([]),
      taskNum: new FormControl('8')
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.listenType(res);
    });

    this.formGroup.get('fqdn').valueChanges.subscribe(res => {
      if (this.item) {
        if (res === this.item.endpoint) {
          this.isShow = true;
          this.initConfig();
          this.formGroup
            .get('isSelected')
            .setValue(!!this.item?.extendInfo?.repIps);
          this.formGroup
            .get('selectedIp')
            .setValue(
              map(JSON.parse(this.item?.extendInfo?.repIps || '{}'), 'IPV4ADDR')
            );
        } else {
          this.isShow = false;
          this.formGroup.get('isSelected').setValue(false);
          this.formGroup.get('selectedIp').setValue([]);
        }
      }
    });

    this.formGroup.statusChanges.subscribe(formGroupStatus => {
      this.enableBtnFn();
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
            this.enableBtn = false;
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
            this.enableBtnFn();
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
            this.enableBtn = false;
            return '';
          }

          const reader = new FileReader();
          this.crlName = first(files)?.name;
          this.crlSize = first(files)?.fileSize;
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            this.enableBtnFn();
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
      this.enableBtnFn();
    }
  }

  revocationListChange(files) {
    if (isEmpty(files)) {
      this.selectRevocationList = '';
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'NAME',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'IPV4ADDR',
        name: this.i18n.get('common_ip_address_label')
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: false
        },
        scrollFixed: true,
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };

    const data = JSON.parse(this.item?.extendInfo?.repIps || '{}');
    this.tableData = {
      data: data,
      total: data.length
    };
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

    if (includes(this.isTestSevice, this.item.subType)) {
      this.initConfig();
    }

    const item = {
      type: this.item.subType,
      equipment_name: this.item.name,
      fqdn: this.item.endpoint,
      port: +this.item.port,
      username: this.item.username,
      verify_status: !!+this.item.extendInfo.verifyStatus,
      isSelected: !!this.item?.extendInfo?.repIps || false,
      selectedIp: map(
        JSON.parse(this.item?.extendInfo?.repIps || '{}'),
        'IPV4ADDR'
      )
    };
    if (this.item.subType === this.deviceStorageType.ndmp.value) {
      assign(item, {
        taskNum: this.item.extendInfo?.maxConcurrentJobNumber
      });
    }
    this.formGroup.patchValue(item);
    defer(() => {
      this.enableBtnFn();
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
      if (this.exterAgent) {
        this.getProxyOptions();
        this.formGroup
          .get('proxyHost')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('verify_status').setValue(false);
      this.formGroup
        .get('taskNum')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 64)
        ]);
    } else {
      this.formGroup.get('verify_status').setValue(true);
      this.formGroup.get('proxyHost').clearValidators();
      this.formGroup.get('taskNum').clearValidators();
    }
    this.formGroup.get('proxyHost').updateValueAndValidity();
    this.formGroup.get('fqdn').updateValueAndValidity();
    this.formGroup.get('port').updateValueAndValidity();
    this.formGroup.get('username').updateValueAndValidity();
    this.formGroup.get('password').updateValueAndValidity();
    this.formGroup.get('taskNum').updateValueAndValidity();
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

  test() {
    const params = {
      endpoint: this.formGroup.get('fqdn').value,
      port: this.formGroup.get('port').value,
      authKey: this.formGroup.get('username').value,
      authPwd: this.formGroup.get('password').value
    };

    this.pluginsStorageService.queryManagerIp(params).subscribe(
      res => {
        this.message.success(this.i18n.get('common_operate_success_label'));
        this.isTest = true;
        const arr = [];
        each(res?.detailParams, item => {
          const obj = JSON.parse(item || '{}');
          arr.push({
            ...obj,
            label: `${obj.NAME}(${obj.IPV4ADDR})`,
            value: obj.IPV4ADDR,
            isLeaf: true
          });
        });
        this.replicaPortOptions = arr;
      },
      () => {
        this.isTest = false;
        this.replicaPortOptions = [];
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
      assign(params.extendInfo, {
        maxConcurrentJobNumber: Number(
          this.formGroup.get('taskNum')?.value || 8
        )
      });
    }

    if (includes(this.isTestSevice, this.formGroup.value.type)) {
      const arr = [];
      each(this.formGroup.get('selectedIp').value, item => {
        const obj = find(this.replicaPortOptions, { value: item });
        arr.push({
          NAME: obj.NAME,
          IPV4ADDR: obj.IPV4ADDR
        });
      });
      assign(params.extendInfo, {
        repIps: JSON.stringify(arr)
      });
    }
    return params;
  }

  create() {
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
      .subscribe(res => {
        this.modal.close();
        cacheGuideResource(res);
        if (isFunction(this.refresh)) {
          this.refresh();
        }
      });
  }

  modify() {
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
      .subscribe(() => {
        this.modal.close();
        if (isFunction(this.refresh)) {
          this.refresh();
        }
      });
  }

  onOK() {
    return this.item ? this.modify() : this.create();
  }

  cancle() {
    this.modal.close();
  }
}
