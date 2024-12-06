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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MultiCluster,
  RoleType,
  WarningMessageService,
  filterApplication,
  isRBACDPAdmin
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  reverse,
  some,
  sortBy
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-host-download-proxy',
  templateUrl: './download-proxy.component.html',
  styleUrls: ['./download-proxy.component.less']
})
export class DownloadProxyComponent implements OnInit {
  formGroup: FormGroup;
  dataMap = DataMap;
  selectedFile;
  filters = [];
  treeData = [];
  agentFileOptions = [];
  agentTableData = {};
  osTypeOptions = [];
  shareDisabled = true;
  agentTableConfig: TableConfig;
  valid$ = new Subject<boolean>();
  downloadProxyOptions = this.dataMapService
    .toArray('Proxy_Type_Options')
    .filter(v => {
      v.isLeaf = true;
      return !includes(
        [DataMap.Proxy_Type_Options.hostAgentOracle.value],
        v.value
      );
    });
  backupProxyTypeOptions = this.dataMapService
    .toArray('Backup_Proxy_File')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  osTypes = this.dataMapService.toArray('OS_Type').filter(v => {
    return (v.isLeaf = true);
  });
  packageTypeOps = this.dataMapService.toArray('packageType').filter(v => {
    return (v.isLeaf = true);
  });
  isMultiStandbyNode =
    MultiCluster.roleType === DataMap.nodeRole.backupNode.value;
  pwdValidTip = this.i18n.get('common_private_key_pwdtip_label', [8, 16, 2]);

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  @ViewChild('agentTable', { static: false }) agentTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private cookieService: CookieService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private clientManagerApiService: ClientManagerApiService,
    private appUtilsService: AppUtilsService,
    public messageService: MessageService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initTable();
    this.initFilters();
    this.getAgentFileOptions();
    this.rejectUpload();
  }

  rejectUpload() {
    if (this.isHcsUser || isRBACDPAdmin(this.cookieService.role)) {
      this.backupProxyTypeOptions = this.backupProxyTypeOptions.filter(item => {
        return item.value !== DataMap.Backup_Proxy_File.Upload.value;
      });
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      backupProxyFile: new FormControl(
        this.dataMap.Backup_Proxy_File.DownLoad.value
      ),
      privateKeyPwd: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 4, 16),
          this.validConfirmPwdIsSame()
        ]
      }),
      privateKeyPwdConfirm: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 4, 16),
          this.validNewPwdIsSame()
        ]
      }),
      proxyType: new FormControl(''),
      applications: new FormControl([]),
      osType: new FormControl(''),
      packageType: new FormControl(DataMap.packageType.zip.value),
      isShared: new FormControl('false')
    });

    this.formGroup.get('proxyType').valueChanges.subscribe(res => {
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
              DataMap.OS_Type.solaris.value
            ],
            item.value
          )
        );
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
        if (this.formGroup.value.osType) {
          this.queryApplicationList(this.formGroup.value.osType);
        }
      } else {
        this.formGroup.get('applications').clearValidators();
      }
      this.formGroup.get('applications').setValue([]);
      this.formGroup.get('applications').updateValueAndValidity();
    });

    this.formGroup.get('osType').valueChanges.subscribe(res => {
      this.formGroup.get('applications').setValue([]);
      if (
        this.formGroup.value.proxyType ===
        DataMap.Proxy_Type_Options.remoteAgent.value
      ) {
        this.queryApplicationList(res);
      }
      this.formGroup.get('packageType').setValue(DataMap.packageType.zip.value);
    });

    this.formGroup.get('backupProxyFile').valueChanges.subscribe(res => {
      if (
        this.dataMap.Backup_Proxy_File.Upload.value === res &&
        isEmpty(this.agentTableData)
      ) {
        setTimeout(() => {
          this.agentTable.fetchData();
        });
      }
    });

    this.formGroup.get('applications').valueChanges.subscribe(res => {
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

  validConfirmPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.privateKeyPwdConfirm &&
        this.formGroup.value.privateKeyPwdConfirm !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.privateKeyPwdConfirm) {
        this.formGroup.get('privateKeyPwdConfirm').setErrors(null);
      }

      return null;
    };
  }

  validNewPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.privateKeyPwd &&
        this.formGroup.value.privateKeyPwd !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.privateKeyPwd) {
        this.formGroup.get('privateKeyPwd').setErrors(null);
      }
      return null;
    };
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'agentName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'uploadTime',
        name: this.i18n.get('protection_upload_time_label'),
        cellRender: {
          type: 'date',
          config: {
            format: 'yyyy-MM-dd HH:mm:ss'
          }
        }
      }
    ];
    this.agentTableConfig = {
      table: {
        async: true,
        columns: cols,
        compareWith: 'id',
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: filter => this.getTableData(filter)
      },
      pagination: {
        pageIndex: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getTableData(filter?) {
    const params = {
      size: CommonConsts.PAGE_SIZE,
      index: CommonConsts.PAGE_START
    };
    if (filter && filter.paginator) {
      assign(params, {
        size: filter.paginator.pageSize,
        index: filter.paginator.pageIndex
      });
    }
    this.clientManagerApiService
      .queryAgentEntitiesUsingGET(params)
      .subscribe(res => {
        const sortItems = sortBy(res.items, item => {
          return +item.uploadTime;
        });
        reverse(sortItems);
        this.agentTableData = {
          data: sortItems,
          total: res.total
        };
      });
  }

  getAgentFileOptions(recordsTemp?, startPage?) {
    this.clientManagerApiService
      .queryAgentEntitiesUsingGET({
        size: CommonConsts.PAGE_SIZE * 5,
        index: startPage || 0
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
          startPage === Math.ceil(res.total / CommonConsts.PAGE_SIZE) ||
          res.total === 0
        ) {
          const agentArr = [];
          recordsTemp = sortBy(recordsTemp, item => {
            return +item.uploadTime;
          });
          reverse(recordsTemp);
          each(recordsTemp, item => {
            agentArr.push({
              key: item.id,
              value: item.id,
              label: item.agentName,
              version: item.version,
              isLeaf: true
            });
          });
          this.agentFileOptions = agentArr;
          return;
        }
        this.getAgentFileOptions(recordsTemp, startPage);
      });
  }

  initFilters() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['zip'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['zip'])
            );
            this.valid$.next(false);
            return false;
          }

          if (files[0].size > 1024 * 1024 * 1024 * 3) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['3GB'])
            );
            this.valid$.next(false);
            return false;
          }

          this.selectedFile = files[0].originFile;
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];
  }

  download() {
    const param = {
      agentID: first(this.agentFileOptions)['value'],
      agentName: first(this.agentFileOptions)['label'],
      type: this.formGroup.value.proxyType,
      privateKey: this.formGroup.value.privateKeyPwd,
      osType: this.formGroup.value.osType,
      compressedPackageType: this.formGroup.value.packageType,
      isShared: this.formGroup.value.isShared
    };
    this.messageService.info(
      this.i18n.get('common_file_download_processing_label'),
      {
        lvDuration: 0,
        lvShowCloseButton: true,
        lvMessageKey: 'agentDownloadKey'
      }
    );

    let applicationsArr = [];
    if (
      this.formGroup.value.proxyType ===
      DataMap.Proxy_Type_Options.remoteAgent.value
    ) {
      applicationsArr = this.getApplicationsInfo();
      assign(param, {
        agentApplicationMenu: JSON.stringify({
          menus: applicationsArr
        })
      });
    }

    this.clientManagerApiService
      .downloadAgentClientUsingPOSTResponse({ params: param, akLoading: false })
      .subscribe(
        res => {
          const bf = new Blob([res.body as any], {
            type:
              this.formGroup.value.packageType === DataMap.packageType.zip.value
                ? 'application/zip'
                : 'application/x-tar'
          });
          const fileName = res.headers
            .get('content-disposition')
            .split('filename="')[1]
            .split('"')[0];
          this.appUtilsService.downloadFile(fileName, bf);
          this.messageService.destroy('agentDownloadKey');
        },
        error => {
          this.messageService.destroy('agentDownloadKey');
        }
      );
  }

  upload() {
    this.messageService.info(
      this.i18n.get('common_file_upload_processing_label'),
      {
        lvDuration: 0,
        lvShowCloseButton: true,
        lvMessageKey: 'agentUploadKey'
      }
    );
    this.clientManagerApiService
      .uploadAgentClientUsingPOST({
        agentClient: this.selectedFile,
        akLoading: false
      })
      .subscribe(
        res => {
          this.messageService.destroy('agentUploadKey');
        },
        error => {
          this.messageService.destroy('agentUploadKey');
        }
      );
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
  queryApplicationList(osType) {
    this.clientManagerApiService
      .queryAgentApplicationsGET({
        lang: this.i18n.language,
        osType: osType,
        akLoading: false
      })
      .subscribe(res => {
        this.formatApplicationData(res);
      });
  }

  private formatApplicationData(res) {
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
  }
}
