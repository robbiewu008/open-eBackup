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
import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  ApiStorageBackupPluginService,
  BaseUtilService,
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  I18NService,
  LiveMountApiService,
  PermissonLimitation,
  PortPermisson,
  RestoreApiV2Service,
  RootPermisson,
  SnapshotRstore
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  each,
  filter,
  includes,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  reject,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-snapshot-restore',
  templateUrl: './snapshot-restore.component.html',
  styleUrls: ['./snapshot-restore.component.less'],
  providers: [DatePipe]
})
export class SnapshotRestoreComponent implements OnInit, AfterViewInit {
  isPacific = false;
  isResource;
  rowData;
  restoreLocation;
  namePrefix = 'Mount_';
  formGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  snapshotRstore = SnapshotRstore;
  title = '';
  selectionData = [];
  userOptions = [];
  copyStatus = DataMap.snapshotCopyStatus;
  dataMap = DataMap;
  _isEn = this.i18n.isEn;
  _includes = includes;
  snapshotName;

  rowDataResourceProperties;
  rowDataProperties;

  validProtocol = new Subject();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('timeTpl', { static: true }) timeTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label')
  };
  integerErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidMaxSize: this.i18n.get('common_valid_maxsize_label'),
    invalidMinSize: this.i18n.get('common_valid_minsize_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 96])
  };
  cifsNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  clientErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255]),
    unsupportValueError: this.i18n.get('common_shnopshot_client_error_label')
  };
  clientHostTipLabel = this.i18n.get('protection_file_system_host_tip_label');

  userTypeOptions = this.dataMapService
    .toArray('Cifs_Domain_Client_Type')
    .filter(v => {
      v.isLeaf = true;
      return includes(
        [
          DataMap.Cifs_Domain_Client_Type.everyone.value,
          DataMap.Cifs_Domain_Client_Type.windows.value,
          DataMap.Cifs_Domain_Client_Type.windowsGroup.value
        ],
        v.value
      );
    });

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private liveMountApiService: LiveMountApiService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngAfterViewInit(): void {
    if (this.isResource) {
      this.dataTable.fetchData();
    }
  }

  ngOnInit() {
    this.initModalHeader();
    this.getSnapshotName();
    if (this.isResource) {
      this.initTable();
      this.restoreLocation = this.rowData.location;
    } else {
      this.restoreLocation = this.rowData.resource_location;
    }
    this.isPacific = this.restoreLocation.slice(0, 17) === 'OceanStor Pacific';

    this.initForm();
    this.getRowDataProperties();
  }

  initModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  openHelp() {
    const lang = this.i18n.isEn ? 'en-us' : 'zh-cn';
    const targetUrl = `/console/assets/help/cyberengine/${lang}/index.html#${lang}_topic_0000001783060378.html`;
    window.open(targetUrl, '_blank');
  }

  getRowDataProperties() {
    if (!this.isResource) {
      try {
        this.rowDataResourceProperties = JSON.parse(
          this.rowData.resource_properties
        );
        this.rowDataProperties = JSON.parse(this.rowData.properties);
      } catch (error) {}
    }
  }

  getSnapshotName() {
    if (
      this.rowData.generate_type === DataMap.snapshotGeneratetype.ioDetect.value
    ) {
      try {
        let properties;
        if (this.isResource) {
          properties = JSON.parse(this.selectionData[0]?.properties);
        } else {
          properties = JSON.parse(this.rowData?.properties);
        }
        this.snapshotName = properties.snapshotName;
      } catch (error) {
        this.snapshotName = this.rowData.name;
      }
    }
  }

  initTable() {
    const copyStatus = this.dataMapService.toArray('detectionSnapshotStatus');
    const antiStatus = this.dataMapService
      .toArray('snapshotCopyStatus')
      .filter(item =>
        includes(
          [
            this.copyStatus.deleteFailed.value,
            this.copyStatus.deleting.value,
            this.copyStatus.restoring.value
          ],
          item.value
        )
      );
    const cols: TableCols[] = [
      {
        key: 'time',
        name: this.i18n.get('common_hyperdetect_time_stamp_label'),
        cellRender: this.timeTpl
      },
      {
        key: 'anti_status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: [...antiStatus, ...copyStatus]
        },
        cellRender: this.statusTpl
      }
    ];
    this.tableConfig = {
      pagination: {
        winTablePagination: true,
        mode: 'simple'
      },
      table: {
        columns: cols,
        compareWith: 'uuid',
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        scrollFixed: true,
        fetchData: (filter: Filters, args) => {
          this.getSnapshotList(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.getSnapshotName();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  getFilterConditions(filters) {
    const defaultParams = {};
    const restoringMap = [
      DataMap.snapshotCopyStatus.mounting.value,
      DataMap.snapshotCopyStatus.mounted.value,
      DataMap.snapshotCopyStatus.unmounting.value
    ];
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.anti_status) {
        let statusArr = conditions.anti_status;
        if (includes(statusArr, DataMap.snapshotCopyStatus.restoring.value)) {
          statusArr = [...statusArr, ...restoringMap];
        }
        assign(defaultParams, {
          copy_status: filter(statusArr, item => isString(item)),
          anti_status: filter(statusArr, item => !isString(item))
        });
      }
    }
    each(defaultParams, (value, key) => {
      if (isEmpty(value) && !isNumber(value)) {
        delete defaultParams[key];
      }
    });
    return defaultParams;
  }

  getSnapshotList(filters, args) {
    const params = {
      resourceId: this.rowData.resource_id,
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const conditions = this.getFilterConditions(filters);
    if (!isEmpty(conditions)) {
      assign(params, { conditions: JSON.stringify(conditions) });
    }

    this.copiesDetectReportService
      .ShowDetectionDetails(params)
      .subscribe(res => {
        this.tableData = {
          data: res.items,
          total: res.total
        };
        this.dataTable.setSelections([res.items[0]]);
        this.selectionData = [res.items[0]];
        this.getSnapshotName();
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.snapshotRstore.ORIGIN),
      targetPath: new FormControl({
        value: this.restoreLocation,
        disabled: true
      }),
      name: new FormControl(),
      keep_time: new FormControl(),
      nfsEnable: new FormControl(false),
      nfsShareName: new FormControl(''),
      client: new FormControl(''),
      cifsEnable: new FormControl(false),
      cifsShareName: new FormControl(''),
      userType: new FormControl(DataMap.Cifs_Domain_Client_Type.everyone.value),
      userName: new FormControl([])
    });
    this.formGroup.get('name').setValue(`${new Date().getTime()}`);
    this.watch();
    assign(this.nameErrorTip, {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
        255 - size(this.namePrefix)
      ])
    });
  }

  watch() {
    let nfs = true;
    let cifs = false;
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === this.snapshotRstore.ORIGIN) {
        this.formGroup.get('name').clearValidators();
        this.formGroup.get('keep_time').clearValidators();
        this.formGroup.get('nfsShareName').clearValidators();
        this.formGroup.get('client').clearValidators();
        this.formGroup.get('cifsShareName').clearValidators();
        this.formGroup.get('userType').clearValidators();
        this.formGroup.get('userName').clearValidators();
      } else {
        this.formGroup
          .get('name')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(255 - size(this.namePrefix))
          ]);
        this.formGroup
          .get('keep_time')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 96)
          ]);
        this.formGroup.get('nfsEnable').setValue(true);
        this.formGroup
          .get('client')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildclient(),
            this.baseUtilService.VALID.maxLength(127)
          ]);
      }
      this.formGroup.get('cifsShareName').updateValueAndValidity();
      this.formGroup.get('userType').updateValueAndValidity();
      this.formGroup.get('client').updateValueAndValidity();
      this.formGroup.get('keep_time').updateValueAndValidity();
      this.formGroup.get('name').updateValueAndValidity();
      this.formGroup.get('nfsShareName').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
    });
    this.formGroup.get('nfsEnable').valueChanges.subscribe(nfsres => {
      if (nfsres) {
        this.formGroup
          .get('client')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildclient(),
            this.baseUtilService.VALID.maxLength(127)
          ]);
        this.validProtocol.next(true);
        nfs = true;
      } else {
        this.formGroup.get('client').clearValidators();
        nfs = false;
      }
      this.formGroup.get('client').updateValueAndValidity();
    });
    this.formGroup.get('cifsEnable').valueChanges.subscribe(cres => {
      if (cres) {
        cifs = true;
        this.formGroup
          .get('cifsShareName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(255)
          ]);
        this.formGroup
          .get('userType')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (
          this.formGroup.get('userType').value !==
          DataMap.Cifs_Domain_Client_Type.everyone.value
        ) {
          this.formGroup
            .get('userName')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
      } else {
        cifs = false;
        this.formGroup.get('cifsShareName').clearValidators();
        this.formGroup.get('userType').clearValidators();
        this.formGroup.get('userName').clearValidators();
      }
      this.formGroup.get('cifsShareName').updateValueAndValidity();
      this.formGroup.get('userType').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
    });
    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (
        res === '' ||
        !this.formGroup.get('cifsEnable').value ||
        this.formGroup.get('restoreTo').value === this.snapshotRstore.ORIGIN
      ) {
        return;
      }
      if (res === DataMap.Cifs_Domain_Client_Type.everyone.value) {
        this.formGroup.get('userName').clearValidators();
      } else if (res === DataMap.Cifs_Domain_Client_Type.windows.value) {
        this.getUsers();
        this.formGroup
          .get('userName')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.getUserGroups();
        this.formGroup
          .get('userName')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('userName').updateValueAndValidity();
    });
    this.formGroup.get('nfsShareName').disable();

    this.formGroup.get('name').valueChanges.subscribe(res => {
      this.formGroup.get('nfsShareName').setValue(`${this.namePrefix}${res}`);

      this.formGroup.get('cifsShareName').setValue(`${this.namePrefix}${res}`);
    });
  }

  getUsers() {
    let rootuuid = '0';
    let vstoreId;
    if (this.selectionData.length !== 0) {
      rootuuid = JSON.parse(this.selectionData[0].resource_properties)
        .root_uuid;
      vstoreId = this.rowData?.tenant_id;
    } else {
      rootuuid = this.rowDataResourceProperties?.root_uuid;
      vstoreId = this.rowDataProperties?.tenantId;
    }
    if (!vstoreId) {
      vstoreId = '0';
    }
    this.apiStorageBackupPluginService
      .ListNasUsersInfo({ esn: rootuuid, vstoreId })
      .subscribe(res => {
        this.userOptions = reject(
          map(res.records, item => {
            return {
              id: item.id,
              label: item.name,
              value: item.name,
              isLeaf: true
            };
          }),
          val => {
            return val.value === 'cifs_backup';
          }
        );
        this.formGroup.get('userName').setValue([]);
      });
  }
  vaildclient(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const client = control.value;
      if (includes(client, ' ')) {
        return { unsupportValueError: { value: control.value } };
      }
      if (client.split('')[0] === '-') {
        return { unsupportValueError: { value: control.value } };
      }
      return null;
    };
  }

  getUserGroups() {
    let rootuuid = '0';
    let vstoreId;
    if (this.selectionData.length !== 0) {
      rootuuid = JSON.parse(this.selectionData[0].resource_properties)
        .root_uuid;
      vstoreId = this.rowData?.tenant_id;
    } else {
      rootuuid = this.rowDataResourceProperties?.root_uuid;
      vstoreId = this.rowDataProperties?.tenantId;
    }
    if (!vstoreId) {
      vstoreId = '0';
    }
    this.apiStorageBackupPluginService
      .ListNasUserGroupsInfo({
        esn: rootuuid,
        vstoreId
      })
      .subscribe(res => {
        this.userOptions = map(res['records'], item => {
          return {
            id: item.id,
            label: item.name,
            value: item.name,
            isLeaf: true
          };
        });
        this.formGroup.get('userName').setValue([]);
      });
  }

  getParams() {
    let data = cloneDeep(this.rowData);
    if (this.isResource) {
      data = this.selectionData[0];
    }

    const params = {};

    if (this.formGroup.value.restoreTo === this.snapshotRstore.ORIGIN) {
      assign(params, {
        copyId: data.uuid,
        targetEnv: JSON.parse(data.resource_properties).root_uuid,
        restoreType: 'normalRestore',
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: data.resource_id
      });
    } else {
      const name = `${this.namePrefix}${this.formGroup.value.name}`;
      assign(params, {
        source_resource_id: data.resource_id,
        copy_id: data.uuid,
        target_resource_uuid_list: [name],
        target_location: this.snapshotRstore.ORIGIN,
        name: name,
        file_system_share_info_list: [],
        parameters: {
          performance: {
            fileSystemKeepTime: parseInt(this.formGroup.value.keep_time)
          }
        }
      });
      let shareInfo = [];
      if (this.formGroup.value.nfsEnable) {
        shareInfo.push({
          fileSystemName: `${this.namePrefix}${this.formGroup.value.name}`,
          type: +DataMap.Shared_Mode.nfs.value,
          accessPermission:
            DataMap.Livemount_Filesystem_Authority_Level.readWrite.value,
          advanceParams: {
            clientType: DataMap.Nfs_Share_Client_Type.host.value,
            clientName: this.formGroup.value.client,
            squash: PermissonLimitation.Retained,
            rootSquash: RootPermisson.Enable,
            portSecure: PortPermisson.Arbitrary
          }
        });
      }

      if (this.formGroup.value.cifsEnable) {
        shareInfo.push({
          fileSystemName: `${this.namePrefix}${this.formGroup.value.name}`,
          type: +DataMap.Shared_Mode.cifs.value,
          accessPermission:
            DataMap.Livemount_Filesystem_Authority_Level.readWrite.value,
          advanceParams: {
            shareName: this.formGroup.value.cifsShareName,
            domainType: 2,
            usernames:
              this.formGroup.value.userType ===
              DataMap.Cifs_Domain_Client_Type.everyone.value
                ? ['@EveryOne']
                : this.formGroup.value.userType ===
                  DataMap.Cifs_Domain_Client_Type.windowsGroup.value
                ? map(this.formGroup.value.userName, item => {
                    return '@' + item;
                  })
                : this.formGroup.value.userName
          }
        });
      }
      params['file_system_share_info_list'] = shareInfo;
    }
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (this.formGroup.value.restoreTo === this.snapshotRstore.ORIGIN) {
        this.restoreV2Service
          .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
      } else {
        this.liveMountApiService
          .createLiveMountCyberUsingPOST({
            liveMountObject: params as any
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
      }
    });
  }

  getTargetPath() {
    if (this.formGroup.value.restoreTo === this.snapshotRstore.ORIGIN) {
      let targetPath = '';
      if (this.isResource) {
        targetPath = this.selectionData[0]?.resource_location;
      } else {
        targetPath = this.rowData?.resource_location;
      }
      return targetPath;
    } else {
      return '';
    }
  }

  getName() {
    let name = '';
    if (this.isResource) {
      name = this.datePipe.transform(
        this.selectionData[0]?.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
    } else {
      name = this.datePipe.transform(
        this.rowData?.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
    }

    return name;
  }
}
