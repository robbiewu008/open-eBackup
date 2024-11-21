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
import { Router } from '@angular/router';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  filterApplication,
  HcsResourceServiceService,
  I18NService,
  MODAL_COMMON,
  RouterUrl,
  WarningMessageService
} from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  omit,
  pick,
  reject,
  size,
  some
} from 'lodash';
import { AddHostIngfoComponent } from './add-host-ingfo/add-host-ingfo.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-host-register',
  templateUrl: './host-register.component.html',
  styleUrls: ['./host-register.component.less']
})
export class HostRegisterComponent implements OnInit {
  treeData = [];
  isTest = false;
  testLoading = false;
  okLoading = false;
  formGroup: FormGroup;
  dataMap = DataMap;
  includes = includes;
  osTypeOptions = [];
  ipRepeat = false;
  portTips = '';
  repeatTips;
  shareDisabled = true;
  ipData = [];
  thmsOptions = this.dataMapService
    .toArray('hmacSignatureAlgorithm')
    .filter(item => {
      return (item.isLeaf = true);
    });
  isBusinessOptions = this.dataMapService
    .toArray('isBusinessOptions')
    .filter(item => {
      return (item.isLeaf = true);
    });
  vmHelp = this.i18n.get('protection_agent_register_vm_help_label');
  typeOptions = this.dataMapService.toArray('Proxy_Type_Options').filter(v => {
    v.isLeaf = true;
    return !includes(
      [DataMap.Proxy_Type_Options.hostAgentOracle.value],
      v.value
    );
  });
  osTypes = this.dataMapService.toArray('OS_Type').filter(v => {
    return (v.isLeaf = true);
  });
  userTypeOptions = this.dataMapService.toArray('userType').filter(v => {
    return (v.isLeaf = true);
  });
  proxyTypeOptions = this.dataMapService.toArray('agentType').filter(v => {
    return (v.isLeaf = true);
  });
  passwordTypeOptions = this.dataMapService
    .toArray('passwordType')
    .filter(v => {
      return (v.isLeaf = true);
    });
  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });

  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });

  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });

  ipsErrorTip = assign({}, this.baseUtilService.ipErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  });
  pathErrorTip = {
    pathError: this.i18n.get('common_path_error_label'),
    linuxPathError: this.i18n.get('protection_host_linux_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_host_linux_path_label'
    )
  };
  autoSyncTip = this.i18n.get('common_auto_sync_host_name_tips_label');

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  hideSourceDuplication;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionEcs = [];
  osTypeMap = {
    [DataMap.OS_Type.Windows.value]: 'Windows',
    [DataMap.OS_Type.Linux.value]: 'Linux',
    [DataMap.OS_Type.Unix.value]: 'Unix'
  };
  cacheAllAgents = [];

  selectionAgents = [];
  userFormGroup: FormGroup;

  isDistributed = this.appUtilsService.isDistributed;

  userGuideCache = USER_GUIDE_CACHE_DATA;

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;
  @ViewChild('portTpl', { static: true }) portTpl: TemplateRef<any>;
  @ViewChild('userTypeTpl', { static: true }) userTypeTpl: TemplateRef<any>;
  @ViewChild('passwordTypeTpl', { static: true }) passwordTypeTpl: TemplateRef<
    any
  >;
  @ViewChild('usernameTpl', { static: true }) usernameTpl: TemplateRef<any>;
  @ViewChild('passwordTpl', { static: true }) passwordTpl: TemplateRef<any>;
  @ViewChild('sudoPasswordTpl', { static: true }) sudoPasswordTpl: TemplateRef<
    any
  >;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('batchUsernamePopover', { static: false }) batchUsernamePopover;

  constructor(
    private router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private hcsResourceService: HcsResourceServiceService,
    private clientManagerApiService: ClientManagerApiService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initTableConfig();
    this.getAllAgents();
    this.initUserForm();
  }

  ngAfterViewInit() {
    this.osHelpHover();
  }

  osHelpHover() {
    setTimeout(() => {
      const domes = document.getElementsByClassName('special-query-support');
      const url = 'https://info.support.huawei.com/storage/comp/#/oceanprotect';
      each(domes, (dom, index) => {
        dom.addEventListener('click', () => {
          window.open(url, '_blank');
        });
      });
    }, 100);
  }

  initTableConfig() {
    if (!this.isHcsUser) {
      return;
    }
    this.tableConfig = {
      table: {
        async: false,
        colDisplayControl: false,
        compareWith: 'id',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'fixed_floating_endpoint',
            name: this.i18n.get('common_ip_address_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'port',
            name: this.i18n.get('common_port_label'),
            width: 120,
            cellRender: this.portTpl
          },
          {
            key: 'userType',
            name: this.i18n.get('common_user_type_label'),
            cellRender: this.userTypeTpl
          },
          {
            key: 'passwordType',
            name: this.i18n.get('common_sudo_no_pwd_label'),
            cellRender: this.passwordTypeTpl
          },
          {
            key: 'username',
            name: this.i18n.get('common_username_label'),
            cellRender: this.usernameTpl
          },
          {
            key: 'password',
            name: this.i18n.get('common_password_label'),
            cellRender: this.passwordTpl
          },
          {
            key: 'sudoPassword',
            name: this.i18n.get('common_sudo_password_label'),
            cellRender: this.sudoPasswordTpl
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: data => {
          this.selectionEcs = data;
          this.selectionAgents = data;
          this.setTestValid();
        },
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  getAllAgents() {
    if (!this.isHcsUser) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        type: 'Host',
        subType: [
          DataMap.Resource_Type.DBBackupAgent.value,
          DataMap.Resource_Type.VMBackupAgent.value,
          DataMap.Resource_Type.UBackupAgent.value,
          DataMap.Resource_Type.SBackupAgent.value
        ],
        scenario: '0',
        isCluster: false
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        this.cacheAllAgents = resource;
      }
    );
  }

  getEcsServer(osType: string, recordsTemp?, startPage?) {
    const params = {
      offset: startPage || CommonConsts.PAGE_START_EXTRA,
      limit: CommonConsts.PAGE_SIZE_MAX,
      notTags: '__type_baremetal',
      expectCapabilities:
        'resize_server,live_resize_info,create_instance_snapshot,clone_server,live_plug_disk,watch_dog_capabilities,live_plug_port,delete',
      expectFields:
        'basic,flavor,scheduler_hints,image_meta,flavor_detail,metadata,addresses,tags,capabilities,resize_or_migrate,cdrom,device_limit,vmtools_detail,operation_rights,action_rights,block_device,vcpu_model,advanced_properties,os_hostname'
    };
    this.hcsResourceService.GetEcmServer(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START_EXTRA;
      }
      recordsTemp = [...recordsTemp, ...res.servers];

      if (
        startPage === Math.ceil(res.count / CommonConsts.PAGE_SIZE_MAX) ||
        !res.count
      ) {
        each(recordsTemp, (item: any) => {
          for (const key in item.addresses) {
            if (Object.prototype.hasOwnProperty.call(item.addresses, key)) {
              const floatingIp = find(item.addresses[key], addr => {
                return addr['OS-EXT-IPS:type'] === 'floating';
              });
              if (floatingIp) {
                assign(item, {
                  floating_endpoint: floatingIp.addr,
                  fixed_endpoint: find(item.addresses[key], addr => {
                    return addr['OS-EXT-IPS:type'] === 'fixed';
                  })?.addr
                });
              }
            }
          }
        });
        recordsTemp = filter(recordsTemp, item => {
          return (
            isEmpty(item.metadata?.os_type) || item.metadata?.os_type === osType
          );
        });
        // 过滤已经注册的IP
        recordsTemp = reject(recordsTemp, item => {
          return !isEmpty(
            find(this.cacheAllAgents, agent => {
              return (
                (agent.extendInfo?.subNetFixedIp === item.floating_endpoint ||
                  agent.extendInfo?.subNetFixedIp === item.fixed_endpoint) &&
                agent.linkStatus ===
                  DataMap.resource_LinkStatus_Special.normal.value
              );
            })
          );
        });
        each(recordsTemp, item => {
          assign(item, {
            fixed_floating_endpoint: `${item.floating_endpoint}(${item.fixed_endpoint})`,
            port:
              osType === this.osTypeMap[DataMap.OS_Type.Windows.value]
                ? '5985'
                : '22',
            userType: DataMap.userType.admin.value,
            passwordType: false,
            valid: true,
            coverSudoPwd: true,
            coverPwd: true
          });
        });
        this.tableData = {
          data: recordsTemp,
          total: size(recordsTemp)
        };
        this.dataTable.setSelections([]);
        this.selectionEcs = [];
        return;
      }
      startPage++;
      this.getEcsServer(osType, recordsTemp, startPage);
    });
  }

  displayTableCols(isWindows) {
    defer(() => {
      if (isWindows) {
        this.dataTable?.setColsDisplay([
          'name',
          'fixed_floating_endpoint',
          'port',
          'username',
          'password'
        ]);
      } else {
        this.dataTable?.setColsDisplay([
          'name',
          'fixed_floating_endpoint',
          'port',
          'userType',
          'passwordType',
          'username',
          'password',
          'sudoPassword'
        ]);
      }
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      osType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      applications: new FormControl([]),
      isShared: new FormControl('false'),
      isEnableDataturbo: new FormControl('false'),
      ipType: new FormControl(DataMap.IP_Type.ipv4.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      algorithms: new FormControl(DataMap.hmacSignatureAlgorithm.safe.value),
      installPath: new FormControl('', {
        validators: [this.validPath(), this.validLinuxPath()]
      })
    });
    this.formGroup.addControl(
      'isAutoSynchronizeHostName',
      new FormControl(true)
    );
    if (this.isHcsUser) {
      this.formGroup.addControl(
        'proxyType',
        new FormControl(DataMap.agentType.external.value)
      );
      this.formGroup.get('proxyType').valueChanges.subscribe(res => {
        if (res === DataMap.agentType.ecs.value) {
          if (isEmpty(this.tableData?.data)) {
            defer(() =>
              this.getEcsServer(this.osTypeMap[this.formGroup.value.osType])
            );
          }
          this.displayTableCols(
            this.formGroup.value.osType === DataMap.OS_Type.Windows.value
          );
        }
      });
    }
    this.formGroup.valueChanges.subscribe(res => {
      this.isTest = false;
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Proxy_Type_Options.hostAgentOracle.value) {
        this.osTypeOptions = this.osTypes.filter(item =>
          includes(
            [DataMap.OS_Type.Linux.value, DataMap.OS_Type.Unix.value],
            item.value
          )
        );
      } else if (
        includes([DataMap.Proxy_Type_Options.remoteAgent.value], res)
      ) {
        this.osTypeOptions = this.osTypes.filter(item =>
          includes(
            [
              DataMap.OS_Type.Windows.value,
              DataMap.OS_Type.Linux.value,
              DataMap.OS_Type.Unix.value,
              DataMap.OS_Type.solaris.value,
              DataMap.OS_Type.hpux.value
            ],
            item.value
          )
        );
        if (this.formGroup.value.osType) {
          this.treeData = [];
          this.queryApplicationList(this.formGroup.value.osType);
          this.formGroup.get('applications').setValue([]);
        }
      } else if (
        includes([DataMap.Proxy_Type_Options.remoteAgentVmware.value], res)
      ) {
        this.osTypeOptions = this.osTypes.filter(item =>
          includes([DataMap.OS_Type.Linux.value], item.value)
        );
      } else if (
        includes([DataMap.Proxy_Type_Options.sanclientAgent.value], res)
      ) {
        this.osTypeOptions = this.osTypes.filter(item =>
          includes([DataMap.OS_Type.Linux.value], item.value)
        );
      }

      if (res === DataMap.Proxy_Type_Options.remoteAgent.value) {
        this.formGroup
          .get('applications')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('applications').clearValidators();
      }
      this.formGroup.get('applications').setValue([]);
      this.formGroup.get('applications').updateValueAndValidity();
    });
    this.formGroup.get('osType').valueChanges.subscribe(res => {
      defer(() => {
        if (
          includes(
            [
              DataMap.OS_Type.Linux.value,
              DataMap.OS_Type.Unix.value,
              DataMap.OS_Type.solaris.value
            ],
            res
          )
        ) {
          this.portTips = '22';
          this.formGroup
            .get('algorithms')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.portTips = '5985';
          this.formGroup.get('algorithms').clearValidators();
        }
        this.formGroup.get('algorithms').updateValueAndValidity();
        if (
          this.isHcsUser &&
          this.formGroup.value.proxyType === DataMap.agentType.ecs.value
        ) {
          defer(() => this.getEcsServer(this.osTypeMap[res]));
        }
        this.displayTableCols(res === DataMap.OS_Type.Windows.value);
      });

      if (
        this.formGroup.value.type ===
        DataMap.Proxy_Type_Options.remoteAgent.value
      ) {
        this.treeData = [];
        this.queryApplicationList(res);
        this.formGroup.get('applications').setValue([]);
      }
      this.formGroup.get('installPath').setValue('');
    });
    this.formGroup.get('applications').valueChanges.subscribe(res => {
      this.hideSourceDuplication = find(
        res,
        item => item?.value === DataMap.Resource_Type.ActiveDirectory.value
      );
      if (isEmpty(res)) return;
      if (this.isHcsUser) return;

      // HDFS HBASE  HIVE HCS_gauss  DWS ECS
      const shareApplications = [
        'HDFSFileset',
        'HBaseBackupSet',
        'HiveBackupSet',
        'HCSGaussDBProject,HCSGaussDBInstance',
        'DWS-cluster,DWS-database,DWS-schema,DWS-table',
        'HCScontainer,HCSCloudHost,HCSProject,HCSTenant',
        'ObjectStorage'
      ];
      this.shareDisabled = some(res, item => {
        if (item.level === 0) {
          return item.applications.some(
            v => !includes(shareApplications, v.appValue)
          );
        } else {
          return !includes(shareApplications, item.appValue);
        }
      });
    });
  }

  initUserForm() {
    this.userFormGroup = this.fb.group({
      userType: new FormControl(DataMap.userType.admin.value),
      passwordType: new FormControl(false),
      sudoPassword: new FormControl(''),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ]
      })
    });
  }

  addHost(row?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'client-info-modal',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_client_info_label'),
        lvContent: AddHostIngfoComponent,
        lvOkDisabled: isEmpty(row),
        lvComponentParams: {
          formGroup: this.formGroup,
          portTips: this.portTips,
          isHcsEnvir: this.isHcsEnvir,
          isHcsUser: this.isHcsUser,
          ipData: this.ipData,
          rowData: row
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddHostIngfoComponent;
          content.hostFormGroup.statusChanges.subscribe(status => {
            modal.lvOkDisabled = status !== 'VALID';
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as AddHostIngfoComponent;
          if (row) {
            each(this.ipData, item => {
              if (item.ip === row.ip && item.port === row.port) {
                assign(item, content.onOk());
              }
            });
          } else {
            this.ipData.push(content.onOk());
          }
          this.ipData = [...this.ipData];
          this.setTestValid();
        }
      })
    );
  }

  modifyRow(row) {
    this.addHost(row);
  }

  removeRow(row) {
    this.ipData = reject(
      this.ipData,
      item => item.ip === row.ip && item.port === row.port
    );
  }

  getStorageFrontIp(item): string {
    if (
      item.isDpcNode &&
      isArray(item.networkInfo) &&
      !isEmpty(item.networkInfo)
    ) {
      return map(item.networkInfo, 'storageIp').join(',');
    }
    return '';
  }

  getStorageFrontGateWay(item): string {
    if (
      item.isDpcNode &&
      isArray(item.networkInfo) &&
      !isEmpty(item.networkInfo)
    ) {
      return map(item.networkInfo, 'storageGateway').join(',');
    }
    return '';
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      if (this.formGroup.value.osType === DataMap.OS_Type.Windows.value) {
        const reg = CommonConsts.REGEX.templateWindowsPath;
        if (!reg.test(control.value) || control.value.length > 1024) {
          return { pathError: { value: control.value } };
        }
      }
      if (
        includes(
          [DataMap.OS_Type.Linux.value, DataMap.OS_Type.Unix.value],
          this.formGroup.value.osType
        )
      ) {
        const reg = CommonConsts.REGEX.templateHostLinuxPath;
        if (!reg.test(control.value) || control.value.length > 1024) {
          return { linuxPathError: { value: control.value } };
        }
      }
      return null;
    };
  }

  validLinuxPath() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !control.value ||
        this.formGroup.value.osType === DataMap.Os_Type.windows.value
      ) {
        return null;
      }

      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });

      for (let path of paths) {
        if (
          includes(['tmp', 'dev', 'bin', 'usr'], path.split('/')[1]) ||
          path === '/'
        ) {
          return { unsupportPathError: { value: control.value } };
        }
      }

      return null;
    };
  }

  setTestValid() {
    this.isTest = false;
  }

  isHcsEcsChecked(item) {
    return !isEmpty(find(this.selectionEcs, ecs => ecs.id === item.id));
  }

  batchUsernamePopoverHide() {
    this.batchUsernamePopover?.hide();
  }

  batchUsername() {
    if (
      this.isHcsUser &&
      this.formGroup.value.proxyType === DataMap.agentType.ecs.value
    ) {
      each(this.selectionEcs, item => {
        assign(item, {
          username: this.userFormGroup.value.username,
          validName: true,
          focusedName: true,
          password: this.userFormGroup.value.password,
          validPwd: true,
          focusedPwd: true
        });
        if (this.formGroup.value.osType !== DataMap.OS_Type.Windows.value) {
          assign(item, {
            userType: this.userFormGroup.value.userType,
            passwordType: this.userFormGroup.value.passwordType,
            sudoPassword: this.userFormGroup.value.sudoPassword,
            validSudoPwd: true,
            focusedSudoPwd: true
          });
        }
      });
    } else {
      each(this.selectionAgents, item => {
        assign(item, {
          username: this.userFormGroup.value.username,
          password: this.userFormGroup.value.password
        });
        if (this.formGroup.value.osType !== DataMap.OS_Type.Windows.value) {
          assign(item, {
            userType: this.userFormGroup.value.userType,
            passwordType: this.userFormGroup.value.passwordType,
            sudoPassword: this.userFormGroup.value.sudoPassword
          });
        }
      });
    }
    this.batchUsernamePopoverHide();
  }

  coverPwd(item) {
    item.coverPwd = !item.coverPwd;
  }

  coverSudoPwd(item) {
    item.coverSudoPwd = !item.coverSudoPwd;
  }

  vaildPort(port) {
    return (
      /^[1-9]\d*$/.test(port) &&
      parseFloat(port) >= 1 &&
      parseFloat(port) <= 65535
    );
  }

  portChange(_, item) {
    if (!item.focused && item.port) {
      item.focused = true;
    }
    assign(item, {
      valid: this.vaildPort(item.port)
    });
    this.setTestValid();
  }

  userTypeChange() {
    this.setTestValid();
  }

  passwordTypeChange() {
    this.setTestValid();
  }

  usernameChange(_, item) {
    if (!item.focusedName && item.username) {
      item.focusedName = true;
    }
    assign(item, {
      validName: item.username && item.username.length <= 255
    });
    this.setTestValid();
  }

  passwordChange(_, item) {
    if (!item.focusedPwd && item.password) {
      item.focusedPwd = true;
    }
    assign(item, {
      validPwd: item.password && item.password.length <= 255
    });
    this.setTestValid();
  }

  sudoPasswordChange(_, item) {
    if (!item.focusedSudoPwd && item.sudoPassword) {
      item.focusedSudoPwd = true;
    }
    assign(item, {
      validSudoPwd: item.sudoPassword && item.sudoPassword.length <= 255
    });
    this.setTestValid();
  }

  getValidStatus() {
    if (this.isHcsUser) {
      return this.formGroup.value.proxyType === DataMap.agentType.ecs.value
        ? this.formGroup.invalid ||
            isEmpty(this.selectionEcs) ||
            some(
              this.selectionEcs,
              item =>
                !item.valid ||
                !item.validName ||
                !item.validPwd ||
                (item.userType === DataMap.userType.common.value &&
                  !item.passwordType &&
                  !item.validSudoPwd)
            )
        : this.formGroup.invalid || isEmpty(this.ipData);
    } else {
      return this.formGroup.invalid || isEmpty(this.ipData);
    }
  }

  testConnection() {
    this.onOK(true);
  }

  register() {
    this.onOK();
  }

  gotoHost() {
    this.router.navigateByUrl(RouterUrl.ProtectionHostAppHost);
  }

  copy() {
    return false;
  }

  onOK(isTest?) {
    let params: any = cloneDeep(
      pick(this.formGroup.value, [
        'type',
        'ipType',
        'osType',
        'installPath',
        'isShared',
        'isEnableDataturbo'
      ])
    );
    params.isEnableDataturbo = this.hideSourceDuplication
      ? false
      : this.formGroup.get('isEnableDataturbo').value;
    let ipInfos = [];
    const windowsFlag =
      this.formGroup.value.osType === DataMap.OS_Type.Windows.value;
    for (let i = 0; i < this.ipData.length; i++) {
      const ipInfo = this.ipData[i];
      const tempObj = {
        ip: ipInfo.ip,
        port: ipInfo.port,
        sftpPort: ipInfo.sftpPort,
        username: ipInfo.username,
        password: ipInfo.password
      };
      if (windowsFlag) {
        assign(tempObj, {
          businessIpFlag: ipInfo.businessIpFlags
        });
      } else {
        assign(tempObj, {
          isSuperUser: ipInfo.userType === DataMap.userType.admin.value,
          isSudoNoPassword: ipInfo.passwordType,
          superPassword:
            ipInfo.userType === DataMap.userType.common.value &&
            !ipInfo.passwordType
              ? ipInfo.sudoPassword
              : ''
        });
      }
      if (this.isHcsEnvir || this.isHcsUser) {
        assign(tempObj, {
          availableZone: ipInfo.az
        });
      }
      // 分布式一体机
      if (this.isDistributed) {
        assign(tempObj, {
          isDpcComputeNode: ipInfo.isDpcNode,
          dpcStorageFrontInfo: map(ipInfo.networkInfo, item => {
            return {
              dpcStorageFrontIp: item.storageIp,
              dpcStorageFrontGateWay: item.storageGateway
            };
          })
        });
        params = omit(params, 'isEnableDataturbo');
      }
      ipInfos.push(tempObj);
    }
    assign(params, {
      ipInfos: ipInfos
    });
    if (this.formGroup.value.type === DataMap.Backup_Proxy.VMware.value) {
      assign(params, {
        osType: DataMap.OS_Type.Others.value
      });
    }
    // 自动同步主机名
    assign(params, {
      isAutoSynchronizeHostName: this.formGroup.value.isAutoSynchronizeHostName
    });
    if (
      includes(
        [
          DataMap.OS_Type.Linux.value,
          DataMap.OS_Type.Unix.value,
          DataMap.OS_Type.solaris.value
        ],
        this.formGroup.value.osType
      )
    ) {
      assign(params, {
        macs: this.formGroup.value.algorithms
      });
    }
    if (
      this.isHcsUser &&
      this.formGroup.value.proxyType === DataMap.agentType.ecs.value
    ) {
      assign(params, {
        ipInfos: map(this.selectionEcs, item => {
          const info = {
            ip: item.floating_endpoint,
            port: item.port,
            username: item.username,
            password: item.password
          };
          if (this.formGroup.value.osType !== DataMap.OS_Type.Windows.value) {
            assign(info, {
              isSuperUser: item.userType === DataMap.userType.admin.value,
              isSudoNoPassword: item.passwordType,
              superPassword: item.sudoPassword
            });
          }
          return info;
        }),
        privateIpInfos: map(this.selectionEcs, item => {
          return {
            ip: item.fixed_endpoint,
            port: item.port
          };
        })
      });
    }
    let applicationsArr = [];
    if (
      this.formGroup.value.type === DataMap.Proxy_Type_Options.remoteAgent.value
    ) {
      applicationsArr = this.getApplicationsInfo();
      assign(params, {
        agentApplicationMenu: {
          menus: applicationsArr
        }
      });
    }
    if (isTest) {
      params = pick(params, ['ips', 'osType', 'ipType', 'macs', 'installPath']);
      if (
        !includes(
          [
            DataMap.OS_Type.Linux.value,
            DataMap.OS_Type.Unix.value,
            DataMap.OS_Type.solaris.value
          ],
          this.formGroup.value.osType
        )
      ) {
        params = pick(params, ['ips', 'osType', 'ipType', 'installPath']);
      }
      if (
        this.isHcsUser &&
        this.formGroup.value.proxyType === DataMap.agentType.ecs.value
      ) {
        assign(params, {
          ipInfos: map(this.selectionEcs, item => {
            const info = {
              ip: item.floating_endpoint,
              port: item.port,
              username: item.username,
              password: item.password
            };
            if (this.formGroup.value.osType !== DataMap.OS_Type.Windows.value) {
              assign(info, {
                isSuperUser: item.userType === DataMap.userType.admin.value,
                isSudoNoPassword: item.passwordType,
                superPassword: item.sudoPassword
              });
            }
            return info;
          }),
          privateIpInfos: map(this.selectionEcs, item => {
            return {
              ip: item.fixed_endpoint,
              port: item.port
            };
          })
        });
      } else {
        assign(params, {
          ipInfos: ipInfos
        });
      }
      this.clientManagerApiService
        .checkRegisterConnectionUsingPOST({ params })
        .subscribe({
          next: (res: any) => {
            if (
              res &&
              res.result &&
              res.result === 'INSTALL_PATH_PERMIT_TOO_HIGH'
            ) {
              const ips = res.ips;
              this.warningMessageService.create({
                content: this.i18n.get('protection_directory_warn_tips_label', [
                  `(${ips})`
                ]),
                okText: this.i18n.get('common_continnue_label'),
                onOK: () => {
                  this.isTest = true;
                },
                onCancel: () => {
                  this.isTest = false;
                }
              });
            } else {
              this.isTest = true;
            }
          },
          error: () => {
            this.isTest = false;
          }
        });
    } else {
      this.clientManagerApiService
        .registerHostAgentUsingPOST({ params })
        .subscribe(res => {
          // 向导缓存注册的主机
          if (this.userGuideCache.active) {
            this.userGuideCache.host.push(...map(params.ipInfos, 'ip'));
          }
          this.gotoHost();
        });
    }
  }

  getApplicationsInfo() {
    const paramArr = [];
    each(this.formGroup.value.applications, item => {
      if (!get(item, 'isLeaf')) {
        // 父级
        each(item.applications, v => {
          v.isChosen = true;
          delete v.appDesc;
        });
        paramArr.push({
          menuValue: item.menuValue,
          menuLabel: item.menuLabel,
          isChosen: true,
          applications: item.applications
        });
      } else {
        // 子级
        const childRes = {
          appValue: item.appValue,
          appLabel: item.appLabel,
          pluginName: item.pluginName,
          isChosen: true
        };
        const parentRes = find(paramArr, { menuValue: item.parent.menuValue });
        if (parentRes) {
          parentRes.applications.push(childRes);
        } else {
          paramArr.push({
            menuValue: item.parent.menuValue,
            menuLabel: item.parent.menuLabel,
            isChosen: false,
            applications: [childRes]
          });
        }
      }
    });
    return paramArr;
  }

  // 根据操作系统查询应用类型列表
  queryApplicationList(osType) {
    this.clientManagerApiService
      .queryAgentApplicationsGET({
        lang: this.i18n.language,
        osType: osType,
        akLoading: false
      })
      .subscribe(res => {
        const resourceArr = [];
        each(res as any, item => {
          resourceArr.push({
            ...item,
            label: item.menuDesc,
            key: item.menuValue,
            value: item.menuValue,
            disabled: false,
            isLeaf: false,
            children: map(filterApplication(item, this.appUtilsService), v => {
              return {
                ...v,
                label: v.appDesc,
                key: v.appValue,
                value: v.appValue,
                disabled: false,
                isLeaf: true
              };
            })
          });
          this.treeData = resourceArr;
        });
      });
  }

  showTypeGuide(item): boolean {
    if (!this.userGuideCache.active) {
      return false;
    }
    if (item.value === DataMap.Proxy_Type_Options.remoteAgent.value) {
      return this.userGuideCache.appType !== DataMap.Resource_Type.vmware.value;
    } else if (
      item.value === DataMap.Proxy_Type_Options.remoteAgentVmware.value
    ) {
      return this.userGuideCache.appType === DataMap.Resource_Type.vmware.value;
    } else {
      return false;
    }
  }

  guideRecommend(item) {
    return (
      isString(item.appValue) &&
      includes(item.appValue.split(','), this.userGuideCache.appType)
    );
  }

  showAppTypeGuide(item) {
    if (!this.userGuideCache.active) {
      return false;
    }
    if (!isEmpty(item.children)) {
      return !isEmpty(find(item.children, child => this.guideRecommend(child)));
    }
    return this.guideRecommend(item);
  }
}
