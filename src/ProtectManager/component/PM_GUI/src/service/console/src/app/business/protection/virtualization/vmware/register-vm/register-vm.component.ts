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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
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
import { StorResourceNodeComponent } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node.component';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  EnvironmentsService,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedEnvironmentService,
  ProtectedResourceApiService,
  ResourceType,
  getPermissionMenuItem
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  toString as _toString,
  assign,
  cloneDeep,
  differenceBy,
  each,
  filter,
  first,
  includes,
  isEmpty,
  isEqual,
  map,
  remove,
  size,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-vm',
  templateUrl: './register-vm.component.html',
  styleUrls: ['./register-vm.component.less']
})
export class RegisterVmComponent implements OnInit {
  isModify;
  isModifyEsxi;
  treeSelection;
  // cnware复用vmware注册
  resourceType;
  item;
  DataMap = DataMap;
  includes = includes;
  ResourceType = ResourceType;
  optItems = [];
  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };

  fcCertFilters = [];
  certFiles = [];
  certModifyStatus: boolean = false;
  selectFcSiteFile = '';
  crlFiles = [];
  crlModifyStatus: boolean = false;

  revocationListFilters = [];
  selectRevocationList = '';
  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';
  certTipLabel = '';

  proxyOptions = [];

  formGroup: FormGroup;
  originNameList = [];
  originIpList = [];
  defaultNutanixPort = '9440';
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    invalidSameIp: this.i18n.get('protection_vm_duplicate_ip_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  rescanIntervalErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 72])
  };
  certTipMap = {
    [ResourceType.CNWARE]: this.i18n.get(
      'protection_register_cnware_cert_tips_label'
    ),
    [ResourceType.NUTANIX]: this.i18n.get(
      'protection_register_nutanix_cert_tips_label'
    )
  };

  @ViewChild('storageTypeTpl', { static: true }) storageTypeTpl: TemplateRef<
    any
  >;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private modal: ModalRef,
    private message: MessageService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentService: ProtectedEnvironmentService,
    private environmentsService: EnvironmentsService,
    private drawModalService: DrawModalService,
    private appUtilsService: AppUtilsService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    if (this.certTipMap[this.resourceType]) {
      this.certTipLabel = this.certTipMap[this.resourceType];
    } else {
      this.certTipLabel = this.i18n.get(
        'protection_register_vmware_cert_tips_label'
      );
    }
    this.isModifyEsxi =
      this.isModify &&
      this.treeSelection[0].rootNodeSubType ===
        DataMap.Resource_Type.vmwareEsxi.value;
    this.initForm();
    this.initFilters();
    this.initTableConfig();
    this.getOriginInfo(CommonConsts.PAGE_START);
    this.isModify && this.getResource();
    this.getProxyOptions();
  }

  getProxyOptions() {
    if (
      !includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.resourceType)
    ) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${this.resourceType}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            !isEmpty(item.environment) &&
            item.environment.extendInfo.scenario === '0'
        );
        if (isEmpty(this.item)) {
          if (MultiCluster.isMulti) {
            resource = filter(resource, item => {
              const val = item.environment;
              const connection = val?.extendInfo?.connection_result;
              const target = JSON.parse(connection)[MultiCluster.esn];
              if (target?.link_status) {
                return true;
              }
              return (
                val.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
              );
            });
          } else {
            resource = filter(
              resource,
              item =>
                item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          }
        }
        each(resource, item => {
          // 注册的场景
          if (isEmpty(this.item)) {
            const tmp = item.environment;
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          } else {
            // 修改的场景
            const tmp = item.environment;
            if (
              (map(this.item.dependencies?.agents, 'uuid')?.includes(
                tmp.uuid
              ) &&
                tmp.extendInfo.scenario === '0') ||
              tmp.linkStatus === '1'
            ) {
              hostArray.push({
                ...tmp,
                key: tmp.uuid,
                value: tmp.uuid,
                label: `${tmp.name}(${tmp.endpoint})`,
                isLeaf: true
              });
            }
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.value.cert
      ? this.formGroup.invalid || (!this.selectFcSiteFile && !this.certName)
      : this.formGroup.invalid;
  }

  initForm() {
    let defaultPort = '443';
    if (this.isModify) {
      defaultPort = this.treeSelection[0].port;
    } else {
      if (includes([ResourceType.NUTANIX], this.resourceType)) {
        defaultPort = this.defaultNutanixPort;
      }
    }

    this.formGroup = this.fb.group({
      name: new FormControl(this.isModify ? this.treeSelection[0].name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.validIsSameName()
        ],
        updateOn: 'change'
      }),
      ip: new FormControl(
        {
          value: this.isModify ? this.treeSelection[0].endpoint : '',
          disabled: !!this.isModify
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(),
            this.validIsSameIp()
          ],
          updateOn: 'change'
        }
      ),
      port: new FormControl(defaultPort, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      userName: new FormControl(
        this.isModify ? this.treeSelection[0].userName : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ],
          updateOn: 'change'
        }
      ),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      }),
      cert: new FormControl(
        this.isModify ? !isEmpty(this.treeSelection[0].cert_name) : true
      ),
      tls_compatible: new FormControl(false),
      rescan_interval_in_sec: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 72)
        ]
      }),
      enableFilter: new FormControl(
        this.isModify
          ? !isEmpty(this.treeSelection[0]?.extendInfo?.nameLike)
          : false
      ),
      filterName: new FormControl(
        this.isModify ? this.treeSelection[0]?.extendInfo?.nameLike || '' : ''
      ),
      children: new FormControl([])
    });

    if (
      includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.resourceType)
    ) {
      this.formGroup
        .get('ip')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validCnwareIp()
        ]);
      this.formGroup.addControl(
        'agents',
        new FormControl([], {
          validators: [this.baseUtilService.VALID.required()]
        })
      );
      if (
        this.isModify &&
        !isEmpty(this.treeSelection[0]?.extendInfo?.nameLike)
      ) {
        this.formGroup
          .get('filterName')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('enableFilter').valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('filterName')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('filterName').clearValidators();
        }
        this.formGroup.get('filterName').updateValueAndValidity();
      });
    }

    this.formGroup.statusChanges.subscribe(() => this.disableOkBtn());
    this.formGroup.get('cert')?.valueChanges.subscribe(res => {
      this.selectFcSiteFile = ''; // 清空
      this.selectRevocationList = '';
      !res && this._certClear()._crlClear();
    });
  }

  initTableConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        disableCheck: () => {
          return this.tableData?.total === 32;
        },
        disabledTips: this.i18n.get('protection_number_limited_label'),
        onClick: () => this.addStorage()
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'storageType',
        name: this.i18n.get('protection_hcs_storage_type_label'),
        cellRender: this.storageTypeTpl
      },
      {
        key: 'ip',
        name: this.i18n.get('common_management_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'username',
        name: this.i18n.get('common_username_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                onClick: ([data]) => this.addStorage(data)
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                onClick: data => this.delete(data)
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10,
        pageSizeOptions: [10]
      }
    };
  }

  addStorage(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-cluster-node',
        lvWidth: MODAL_COMMON.normalWidth + 150,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: StorResourceNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data,
          tableData: this.tableData,
          children: this.formGroup.value.children,
          subType: DataMap.Resource_Type.vmware.value
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as StorResourceNodeComponent;
            content.onOK().subscribe({
              next: res => {
                if (!res) {
                  resolve(false);
                  return;
                }
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                if (isEmpty(data)) {
                  currentTableData = currentTableData.concat([content.data]);
                } else {
                  const oldItem = currentTableData.find(
                    item =>
                      _toString(item.ip) === _toString(data.ip) &&
                      item.port === data.port
                  );
                  assign(oldItem, content.data);
                }
                this.formGroup.get('children').setValue(currentTableData);
                this.formGroup.get('children').updateValueAndValidity();
                this.tableData = {
                  data: currentTableData,
                  total: size(currentTableData)
                };
              }
            });
          });
        }
      })
    );
  }

  delete(data?) {
    const currentTableData = cloneDeep(this.tableData.data);
    remove(currentTableData, item => isEqual(data[0], item));
    this.tableData = {
      data: currentTableData,
      total: size(currentTableData)
    };
    this.formGroup.get('children').setValue(currentTableData);
    this.formGroup.get('children').updateValueAndValidity();
  }

  getResource() {
    if (this.isModify) {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: this.treeSelection[0].uuid
        })
        .subscribe((res: any) => {
          const { extendInfo } = res;
          if (extendInfo.cert_name) {
            this.certFiles = [
              {
                key: '-1',
                name: extendInfo.cert_name,
                fileSize: extendInfo.cert_size,
                status: UploadFileStatusEnum.SUCCESS
              }
            ];
            this.certName = extendInfo.cert_name;
            this.certSize = extendInfo.cert_size;
          }
          if (extendInfo.crl_name) {
            this.crlFiles = [
              {
                key: '-1',
                name: extendInfo.crl_name,
                fileSize: extendInfo.crl_size,
                status: UploadFileStatusEnum.SUCCESS
              }
            ];
            this.crlName = extendInfo.crl_name;
            this.crlSize = extendInfo.crl_size;
          }
          this.formGroup
            .get('tls_compatible')
            ?.setValue(extendInfo.tls_compatible === 'True');
          if (res.scanInterval) {
            this.formGroup
              .get('rescan_interval_in_sec')
              ?.setValue(res.scanInterval / 3600);
          }

          if (
            includes(
              [ResourceType.CNWARE, ResourceType.NUTANIX],
              this.resourceType
            )
          ) {
            this.formGroup.get('userName').setValue(res.auth?.authKey);
            this.formGroup
              .get('agents')
              .setValue(map(res.dependencies?.agents, 'uuid'));
            this.formGroup.get('cert').setValue(extendInfo?.enableCert === '1');
            if (extendInfo?.certName) {
              this.certFiles = [
                {
                  key: '-1',
                  name: extendInfo.certName,
                  fileSize: extendInfo.certSize,
                  status: UploadFileStatusEnum.SUCCESS
                }
              ];
              this.certName = extendInfo.certName;
              this.certSize = extendInfo.certSize;
            }
            if (extendInfo?.crlName) {
              this.crlFiles = [
                {
                  key: '-1',
                  name: extendInfo.crlName,
                  fileSize: extendInfo.crlSize,
                  status: UploadFileStatusEnum.SUCCESS
                }
              ];
              this.crlName = extendInfo.crlName;
              this.crlSize = extendInfo.crlSize;
            }
          }

          const showData = JSON.parse(extendInfo?.storages || '[]').map(
            item => {
              return {
                ip: item.ip,
                port: item.port,
                username: item.username,
                enableCert: item.enableCert,
                certName: item.certName,
                certSize: item.certSize,
                storageType:
                  item.type ||
                  DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value
              };
            }
          );
          this.tableData = {
            data: showData,
            total: size(showData)
          };
          this.formGroup.get('children').setValue(this.tableData.data);
        });
    }
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
              (!this.selectFcSiteFile && !this.certName) ||
              this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  private _certClear() {
    this.certFiles = [];
    each(['selectFcSiteFile', 'certName', 'certSize'], key => {
      this[key] = '';
    });
    this.disableOkBtn();
    return this;
  }

  private _updateCertModifyStatus() {
    if (this.isModify) {
      this.certModifyStatus = true;
    }
  }

  private _crlClear() {
    this.crlFiles = [];
    each(['selectRevocationList', 'crlName', 'crlSize'], key => {
      this[key] = '';
    });
    return this;
  }

  private _updateCrlModifyStatus() {
    if (this.isModify) {
      this.crlModifyStatus = true;
    }
  }

  cartChange(e) {
    e?.action === 'remove' && this._certClear();
    e?.action === 'ready' && this._updateCertModifyStatus();
  }

  crlChange(e) {
    e?.action === 'remove' && this._crlClear();
    e?.action === 'ready' && this._updateCrlModifyStatus();
  }
  getOriginInfo(startPage) {
    if (
      includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.resourceType)
    ) {
      return;
    }
    this.environmentsService
      .queryResourcesV1EnvironmentsGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          type: ResourceType.VSPHERE
        })
      })
      .subscribe(platformRes => {
        platformRes.items.forEach(item => {
          if (
            !(
              this.isModify &&
              (item.name === this.treeSelection[0].name ||
                item.endpoint === this.treeSelection[0].port)
            )
          ) {
            this.originNameList.push(item.name);
            this.originIpList.push(item.endpoint);
          }
        });

        startPage++;
        if (
          platformRes.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] >
          0
        ) {
          this.getOriginInfo(startPage);
          return;
        }
      });
  }

  validCnwareIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (control.value.length > 128) {
        return { invalidMaxLength: { value: control.value } };
      }
      const domainRe = CommonConsts.REGEX.nasshareDomain;
      const ipv4Re = CommonConsts.REGEX.ipv4;
      const ipv6Re = CommonConsts.REGEX.ipv6;

      return !domainRe.test(control.value) &&
        !ipv4Re.test(control.value) &&
        !ipv6Re.test(control.value)
        ? { invalidName: { value: control.value } }
        : null;
    };
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

  validIsSameIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originIpList.includes(trim(control.value))
        ? { invalidSameIp: { value: control.value } }
        : null;
    };
  }

  getParams() {
    const storages = map(this.formGroup.value.children, item => {
      return {
        username: item.username,
        password: item.password,
        port: item.port,
        ip: item.ip,
        enableCert: String(+item.enableCert),
        certName: item.certName,
        certSize: item.certSize,
        crlName: item.crlName,
        crlSize: item.crlSize,
        type: item.storageType
      };
    });
    const params = {
      name: this.formGroup.value.name,
      user_name: this.formGroup.value.userName,
      password: this.formGroup.value.password,
      endpoint: this.formGroup.value.ip || this.treeSelection[0]?.endpoint,
      port: +this.formGroup.value.port,
      verify_cert: this.formGroup.value.cert ? 1 : 0,
      rescan_interval_in_sec:
        this.formGroup.value.rescan_interval_in_sec * 3600,
      extend_context: {
        certification: this.selectFcSiteFile,
        cert_name: this.formGroup.value.cert ? this.certName : '',
        cert_size: this.formGroup.value.cert ? this.certSize : '',
        revocation_list: this.selectRevocationList,
        crl_name: this.formGroup.value.cert ? this.crlName : '',
        crl_size: this.formGroup.value.cert ? this.crlSize : '',
        tls_compatible: this.formGroup.value.tls_compatible ? 'True' : 'False',
        storages: storages
      }
    };

    if (this.isModify) {
      const _clearExtendItem = (keys: string[]) => {
        each(keys, key => {
          params.extend_context[key] = '';
        });
        if (!this.certModifyStatus) {
          _clearExtendItem(['certification']);
        }
        if (!this.crlModifyStatus) {
          _clearExtendItem(['revocation_list']);
        }
      };
    } else {
      assign(params, {
        type: ResourceType.VSPHERE as any,
        sub_type: DataMap.Resource_Type.vmware.value as any
      });
    }
    return params;
  }

  getCnwareParams(): any {
    let reduceAgents = [];
    if (this.item) {
      reduceAgents = differenceBy(
        this.item?.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    return {
      name: this.formGroup.value.name,
      type: this.resourceType,
      subType: this.resourceType,
      endpoint: this.formGroup.value.ip || this.treeSelection[0]?.endpoint,
      port: +this.formGroup.value.port,
      auth: {
        authType: '1',
        authKey: this.formGroup.value.userName,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          enableCert: this.formGroup.value.cert ? '1' : '0',
          certification: !isEmpty(this.selectFcSiteFile)
            ? this.selectFcSiteFile
            : '',
          revocationlist: !isEmpty(this.selectRevocationList)
            ? this.selectRevocationList
            : ''
        }
      },
      extendInfo: {
        rescanIntervalInSec: this.formGroup.value.rescan_interval_in_sec * 3600,
        nameLike: this.formGroup.value.enableFilter
          ? this.formGroup.value.filterName
          : '',
        enableCert: this.formGroup.value.cert ? '1' : '0',
        certName: this.formGroup.value.cert ? this.certName : '',
        certSize: this.formGroup.value.cert ? this.certSize : '',
        crlName: this.formGroup.value.cert ? this.crlName : '',
        crlSize: this.formGroup.value.cert ? this.crlSize : ''
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return { uuid: item };
        }),
        '-agents': !isEmpty(this.item)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (
        includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.resourceType)
      ) {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: this.getCnwareParams()
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        const params = this.getParams();
        this.protectedEnvironmentService
          .scanEnvV1EnvironmentsPost({
            body: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (
        includes([ResourceType.CNWARE, ResourceType.NUTANIX], this.resourceType)
      ) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: this.getCnwareParams(),
            envId: this.treeSelection[0].uuid
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        const params = this.getParams();
        this.protectedEnvironmentService
          .modifyEnvV1EnvironmentsEnvIdPut({
            envId: this.treeSelection[0].uuid,
            body: params
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
