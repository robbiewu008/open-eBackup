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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  DatatableComponent,
  FilterConfig,
  ModalRef,
  PaginatorComponent
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CopyControllerService,
  DataMap,
  extendParams,
  I18NService,
  MODAL_COMMON,
  RestoreV2LocationType,
  VmFileReplaceStrategy
} from 'app/shared';
import { FileSystemResponse } from 'app/shared/api/models/file-system-response';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  isString,
  map,
  remove,
  set,
  sortedIndex,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import ListCopyCatalogsParams = CopyControllerService.ListCopyCatalogsParams;

@Component({
  selector: 'aui-database-group-restore',
  templateUrl: './restore.component.html',
  styleUrls: ['./restore.component.less'],
  providers: [DatePipe],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class RestoreComponent implements OnInit {
  @Input() rowCopy: any;
  @Input() childResType: string;
  @Input() restoreType: string;
  formGroup: FormGroup;
  isDrill;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  dataMap = DataMap;
  hostAndDAGOptions = [];
  sourceData = [];
  sourceSelection = [];
  targetData = [];
  logCopyTimeStamp = [];
  isExchangeDatabaseAndLogCopy = false; // Exchange数据库的日志副本需要查询sqlite
  isNeedSelectTimeStamp = false; // 接口正常返回时，才能手动选择时间
  pageSizeS = CommonConsts.PAGE_SIZE;
  pageIndexS = CommonConsts.PAGE_START;
  pageSizeT = CommonConsts.PAGE_SIZE; // target pageSize
  pageIndexT = CommonConsts.PAGE_START;
  querySourceName = '';
  queryTargetName = '';
  queryTargetDBName = '';
  rowCopyResourceProperties: any;
  rowCopyProperties: any;
  limitWidth = false; // 单机/可用性组恢复日志副本需要限制input框宽度
  hiddenDatabase = true;
  hideAdvanced = true; // DAG和DAG数据库不需要高级参数
  scriptPlaceholder = this.i18n.get(
    'protection_fileset_advance_script_windows_label'
  );
  scriptToolTip = this.i18n.get('common_script_agent_windows_position_label');
  disableOriginLocation = false;
  disableNewLocation = false;
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  databaseNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get(
      'protection_exchange_valid_database_name_error_tip_label'
    ),
    invalidAscii: this.i18n.get(
      'protection_valid_database_name_ascii_tip_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  dbFilePathPlaceholder = this.i18n.get(
    'protection_exchange_restore_database_file_path_dag_placeholder_label'
  );
  dbFilePathTips = this.i18n.get(
    'protection_exchange_restore_database_file_path_dag_tips_label'
  );
  logFilePathPlaceholder = this.i18n.get(
    'protection_exchange_restore_database_file_path_dag_placeholder_label'
  );
  logFilePathTips = this.i18n.get(
    'protection_exchange_restore_log_file_path_tips_label'
  );
  @ViewChild('lvSourceTable', { static: false })
  lvSourceTable: DatatableComponent;
  @ViewChild('lvTargetTable', { static: false })
  lvTargetTable: DatatableComponent;
  @ViewChild('pageS', { static: false }) pageS: PaginatorComponent;
  @ViewChild('pageT', { static: false }) pageT: PaginatorComponent;
  constructor(
    public baseUtilService: BaseUtilService,
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private copyControllerService: CopyControllerService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initForm();
    this.getTargetHosts();
    if (this.isExchangeDatabaseAndLogCopy) {
      this.getLogCopyRestoreTimeStamp();
    }
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('host').setValue(config.targetEnv);
      this.formGroup
        .get('target_db_data_path')
        .setValue(config.extendInfo?.target_db_data_path);
      this.formGroup
        .get('target_db_log_path')
        .setValue(config.extendInfo?.target_db_log_path);
      this.formGroup
        .get('auto_dismount')
        .setValue(config.extendInfo?.auto_dismount);
      this.formGroup.get('auto_mount').setValue(config.extendInfo?.auto_mount);
      this.formGroup
        .get('new_db_name')
        .setValue(config.extendInfo?.new_db_name);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getTargetHosts(event);
  }

  initConfig() {
    this.rowCopyResourceProperties = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.rowCopyProperties = isString(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties)
      : {};
    this.hiddenDatabase =
      includes(
        [
          DataMap.CopyData_generatedType.tapeArchival.value,
          DataMap.CopyData_generatedType.cloudArchival.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value ||
      DataMap.Resource_Type.ExchangeDataBase.value === this.childResType;
    // restore.service里根据类型给不同宽度 这里进行限制
    this.limitWidth =
      get(this.modal, 'lvWidth', MODAL_COMMON.normalWidth) >
      MODAL_COMMON.normalWidth + 100;
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;
    this.scriptPlaceholder = this.i18n.get(
      'protection_fileset_advance_script_windows_label'
    );
    // exchange不支持2010版本及以前的版本的新位置恢复，微软官方定义的2010版本号为14开头
    this.disableNewLocation =
      get(this.rowCopyProperties, 'version', '').split('.')[0] <= '14';
    if (!this.hiddenDatabase) {
      this.getDatabases();
    }
    if (this.childResType === DataMap.Resource_Type.ExchangeDataBase.value) {
      this.dbFilePathPlaceholder = this.i18n.get(
        'protection_exchange_restore_database_file_path_db_placeholder_label'
      );
      this.dbFilePathTips = this.i18n.get(
        'protection_exchange_restore_database_file_path_db_tips_label'
      );
      this.logFilePathPlaceholder = this.i18n.get(
        'protection_exchange_restore_database_log_path_db_placeholder_label'
      );
    }
    this.hideAdvanced =
      this.childResType === DataMap.Resource_Type.ExchangeGroup.value ||
      this.rowCopyResourceProperties?.environment_sub_type ===
        DataMap.Resource_Type.ExchangeGroup.value;
    this.isExchangeDatabaseAndLogCopy =
      this.childResType === DataMap.Resource_Type.ExchangeDataBase.value &&
      this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value &&
      !isEmpty(get(this.rowCopy, 'restoreTimeStamp', ''));
  }

  initForm() {
    const scriptValidator: ValidatorFn[] = [
      this.baseUtilService.VALID.maxLength(8192),
      this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsScript, false)
    ];
    this.formGroup = this.fb.group({
      restoreLocation: [RestoreV2LocationType.ORIGIN],
      originLocation: [
        {
          value: '',
          disabled: true
        }
      ],
      host: [
        { value: '', disabled: true },
        [this.baseUtilService.VALID.required()]
      ],
      target_db_data_path: [
        { value: '', disabled: true },
        [this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsPath, false)]
      ],
      target_db_log_path: [
        { value: '', disabled: true },
        [this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsPath, false)]
      ],
      auto_mount: [true],
      auto_dismount: [false],
      // 数据库名称前缀
      db_name_prefix: [
        '',
        [this.validDBName(), this.baseUtilService.VALID.maxLength(256)]
      ],
      // 数据库名称后缀
      db_name_suffix: [
        '',
        [this.validDBName(), this.baseUtilService.VALID.maxLength(256)]
      ],
      // 新数据库名称，数据库恢复时必填
      new_db_name: [this.rowCopyResourceProperties?.name || ''],
      preScript: ['', scriptValidator],
      postScript: ['', scriptValidator],
      executeScript: ['', scriptValidator]
    });
    this.listenForm();
    if (this.childResType === DataMap.Resource_Type.ExchangeDataBase.value) {
      this.formGroup
        .get('new_db_name')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validDBName(),
          this.baseUtilService.VALID.maxLength(256)
        ]);
      this.formGroup.get('new_db_name').updateValueAndValidity();
    }
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  validDBName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      /*
        exchange官方的要求：
        1. 该值不能包含 '\','/','=',';','0x00','0x0A'.
        2. 该值必须只包含 ASCII 字符.
        3. 数据库名称间可以加空格，例如'Exchange Server Database'
      */
      const asciiReg: RegExp = /^[\x00-\x7F]*$/;
      const reg: RegExp = /[\/\\=;\x00\x0A]/;
      if (!asciiReg.test(control.value)) {
        return { invalidAscii: { value: control.value } };
      }

      if (reg.test(control.value)) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    });

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('');

      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
        this.formGroup.get('target_db_data_path').disable();
        this.formGroup.get('target_db_log_path').disable();
        this.formGroup.get('target_db_data_path').clearValidators();
        this.formGroup.get('target_db_log_path').clearValidators();
      } else {
        this.formGroup.get('host').enable();
        this.formGroup.get('target_db_data_path').enable();
        this.formGroup.get('target_db_log_path').enable();
        this.formGroup
          .get('target_db_data_path')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsPath),
            this.baseUtilService.VALID.maxLength(256)
          ]);
        this.formGroup
          .get('target_db_log_path')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsPath),
            this.baseUtilService.VALID.maxLength(256)
          ]);
      }
      this.formGroup.get('target_db_data_path').updateValueAndValidity();
      this.formGroup.get('target_db_log_path').updateValueAndValidity();
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('target_db_data_path').setValue('');
      this.formGroup.get('target_db_log_path').setValue('');
      if (isEmpty(res)) {
        return;
      }
    });

    this.formGroup.get('db_name_prefix').valueChanges.subscribe(res => {
      const prefix = trim(res);
      const suffix = trim(this.formGroup.value.db_name_suffix);
      each(this.targetData, item => {
        assign(item, {
          new_db_name: `${prefix && prefix !== '' ? `${prefix}_` : ''}${
            item.name
          }${suffix && suffix !== '' ? `_${suffix}` : ''}`
        });
      });
      this.cdr.detectChanges();
    });

    this.formGroup.get('db_name_suffix').valueChanges.subscribe(res => {
      const suffix = trim(res);
      const prefix = trim(this.formGroup.value.db_name_prefix);
      each(this.targetData, item => {
        assign(item, {
          new_db_name: `${prefix && prefix !== '' ? `${prefix}_` : ''}${
            item.name
          }${suffix && suffix !== '' ? `_${suffix}` : ''}`
        });
      });
      this.cdr.detectChanges();
    });
  }

  selectionChange(selection) {
    const prefix = trim(this.formGroup.value.db_name_prefix);
    const suffix = trim(this.formGroup.value.db_name_suffix);
    const cacheSelection = [...selection];
    if (!isEmpty(selection)) {
      each(cacheSelection, item => {
        const target_name = `${prefix && prefix !== '' ? `${prefix}_` : ''}${
          item.name
        }${suffix && suffix !== '' ? `_${suffix}` : ''}`;
        assign(item, {
          new_db_name: target_name
        });
      });
    }
    this.targetData = cacheSelection;
  }

  removeItem(data) {
    const removeRow = remove(this.targetData, item => item.key === data.key);
    this.cdr.detectChanges();
    this.lvSourceTable.deleteSelection(removeRow);
  }

  pageChangeS(page) {
    this.pageSizeS = page.pageSize;
    this.pageIndexS = page.pageIndex;
    this.cdr.detectChanges();
  }

  pageChangeT(page) {
    this.pageSizeT = page.pageSize;
    this.pageIndexT = page.pageIndex;
    this.cdr.detectChanges();
  }

  searchByName(
    name: string,
    searchKey: string,
    table: DatatableComponent,
    page: PaginatorComponent
  ) {
    const option: FilterConfig = {
      key: searchKey,
      value: name,
      filterMode: 'contains'
    };
    table.filter(option);
    page.jumpToFisrtPage();
  }

  getDatabases() {
    const extParams = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      copyId: this.rowCopy.uuid,
      parentPath: '/'
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      (param: ListCopyCatalogsParams) =>
        this.copyControllerService.ListCopyCatalogs(param),
      (resource: FileSystemResponse[]) => {
        this.sourceData = map(resource, (item, index) => {
          assign(item, {
            environment_name: this.rowCopyResourceProperties.name,
            name: get(item, 'path', ''),
            key: index
          });
          return item;
        });
        this.cdr.detectChanges();
      }
    );
  }

  getTargetHosts(labelParams?: any) {
    const conditions = {
      subType: [
        `${DataMap.Resource_Type.ExchangeSingle.value}`,
        `${DataMap.Resource_Type.ExchangeGroup.value}`
      ]
    };
    extendParams(conditions, labelParams);

    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.hostAndDAGOptions = map(resource, item => ({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item?.path})`,
          isLeaf: true
        })).filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        if (!this.disableOriginLocation) {
          const originalHost = find(this.hostAndDAGOptions, {
            uuid: this.rowCopyResourceProperties?.environment_uuid
          });
          this.formGroup
            .get('originLocation')
            .setValue(
              originalHost?.label ||
                this.rowCopyResourceProperties?.environment_name
            );
        }
        this.updateDrillData();
      }
    );
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.formGroup.getRawValue().originLocation
      : find(this.hostAndDAGOptions, { uuid: this.formGroup.value.host })[
          'label'
        ];
  }

  getLogCopyRestoreTimeStamp() {
    const params = {
      parentPath: `/logtime`, // 固定的请求名
      copyId: this.rowCopy.uuid,
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akDoException: false,
      akOperationTips: false
    };
    this.copyControllerService.ListCopyCatalogs(params).subscribe({
      next: res => {
        const data = first(res.records);
        const timestamps = get(
          JSON.parse(get(data, 'extendInfo', '{}')),
          'logtimes',
          []
        );
        if (isEmpty(timestamps)) {
          this.isNeedSelectTimeStamp = false;
          return;
        }
        this.isNeedSelectTimeStamp = true;
        this.cdr.markForCheck();
        this.formGroup.addControl(
          'log_copy_restore_time_stamp',
          new FormControl('', {
            validators: [this.baseUtilService.VALID.required()]
          })
        );
        this.findClosestTimestamps(
          timestamps,
          Number(get(this.rowCopy, 'restoreTimeStamp'))
        );
      },
      error: err => {
        this.isNeedSelectTimeStamp = false;
      }
    });
  }

  // 辅助函数，用于在数组中找到更接近给定时间戳的时间戳
  findCloserTimestamp(
    timestamps: number[],
    givenTimestamp: number,
    currentIndex: number
  ): number {
    let closerIndex = currentIndex;
    let minDiff = Math.abs(timestamps[currentIndex] - givenTimestamp);

    // 检查左右两边的时间戳
    if (currentIndex > 0) {
      const leftDiff = Math.abs(timestamps[currentIndex - 1] - givenTimestamp);
      if (leftDiff < minDiff) {
        minDiff = leftDiff;
        closerIndex = currentIndex - 1;
      }
    }

    if (currentIndex < timestamps.length - 1) {
      const rightDiff = Math.abs(timestamps[currentIndex + 1] - givenTimestamp);
      if (rightDiff < minDiff) {
        minDiff = rightDiff;
        closerIndex = currentIndex + 1;
      }
    }

    return closerIndex;
  }

  // 右优先
  // 输入2在[1,3]中会优先找到3
  findClosestTimestamps(timestamps: number[], givenTimestamp: number) {
    // 找到给定时间戳最接近的时间戳
    let index = sortedIndex(timestamps, givenTimestamp);
    let start: number;
    let end: number;

    if (index === 0) {
      start = 0;
      end = Math.min(index + 3, timestamps.length);
    } else if (index === timestamps.length) {
      start = Math.max(index - 3, 0);
      end = timestamps.length;
    } else {
      // 检查目标时间戳是否与给定时间戳相同
      if (timestamps[index] !== givenTimestamp) {
        // 找到更接近给定时间戳的时间戳
        index = this.findCloserTimestamp(timestamps, givenTimestamp, index);
      }
      start = Math.max(index - 2, 0);
      end = Math.min(index + 3, timestamps.length);
    }
    // 返回结果
    this.logCopyTimeStamp = timestamps.slice(start, end).map(time => {
      return {
        value: time,
        key: time,
        label: this.datePipe.transform(time * 1e3, 'yyyy-MM-dd HH:mm:ss'),
        isLeaf: true
      };
    });
  }

  getParams() {
    const {
      restoreLocation,
      host,
      auto_dismount,
      auto_mount,
      target_db_log_path,
      target_db_data_path
    } = this.formGroup.value;
    const params = {
      copyId: this.rowCopy.uuid,
      restore_location: restoreLocation,
      targetEnv:
        restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.rowCopyResourceProperties?.environment_uuid
          : host,
      // 单机和DAG下发id，数据库恢复下发名称
      targetObject:
        restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.rowCopyResourceProperties?.environment_uuid
          : host,
      restoreType: this.restoreType,
      targetLocation: restoreLocation,
      agents: [],
      extendInfo: {
        auto_dismount,
        auto_mount,
        restore_location: restoreLocation,
        db_name_prefix: trim(this.formGroup.value.db_name_prefix),
        db_name_suffix: trim(this.formGroup.value.db_name_suffix)
      },
      scripts: {
        preScript: trim(this.formGroup.value.preScript),
        postScript: trim(this.formGroup.value.postScript),
        failPostScript: trim(this.formGroup.value.executeScript)
      }
    };
    if (this.childResType === DataMap.Resource_Type.ExchangeDataBase.value) {
      assign(params, {
        targetObject:
          restoreLocation === RestoreV2LocationType.ORIGIN
            ? this.rowCopyResourceProperties?.environment_name
            : find(this.hostAndDAGOptions, { key: host }).name
      });
      assign(params.extendInfo, {
        new_db_name: trim(this.formGroup.value.new_db_name)
      });
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value
        ],
        this.childResType
      )
    ) {
      assign(params, {
        subObjects: map(this.sourceSelection, item =>
          JSON.stringify({
            source_db_name: item.name,
            source_target_name: item?.new_db_name || item.name
          })
        )
      });
    }
    if (restoreLocation === RestoreV2LocationType.NEW) {
      assign(params.extendInfo, {
        target_db_log_path,
        target_db_data_path
      });
    }
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      const timeStamp =
        get(this.rowCopy, 'restoreTimeStamp') ||
        get(this.rowCopyProperties, 'endTime');
      assign(params.extendInfo, {
        restoreTimestamp: timeStamp
      });
      if (this.isExchangeDatabaseAndLogCopy && this.isNeedSelectTimeStamp) {
        set(
          params,
          'extendInfo.restoreTimestamp',
          this.formGroup.get('log_copy_restore_time_stamp').value
        );
      }
    }
    return params;
  }

  restore(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
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
    });
  }
}
