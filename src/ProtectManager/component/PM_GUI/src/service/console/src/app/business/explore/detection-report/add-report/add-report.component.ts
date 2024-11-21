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
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  assign,
  defer,
  each,
  eq,
  find,
  get,
  isEmpty,
  isUndefined,
  map,
  set,
  tail
} from 'lodash';
import { DatePipe } from '@angular/common';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DetectReportAPIService,
  I18NService,
  ProtectedResourceApiService,
  extendSlaInfo
} from 'app/shared';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { map as _map } from 'rxjs/operators';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';

@Component({
  selector: 'app-add-report',
  templateUrl: './add-report.component.html',
  styleUrls: ['./add-report.component.less'],
  providers: [DatePipe]
})
export class AddReportComponent implements OnInit {
  formGroup: FormGroup;
  tableConfig: TableConfig;
  storageOptions = [];
  vStoreOptions = [];
  selectionData = [];
  tableData: TableData;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_valid_client_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [30])
  };
  timeErrorTip = {
    invaildTime: this.i18n.get('common_valid_endtime_label')
  };
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private datePipe: DatePipe,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private detectReportApi: DetectReportAPIService,
    private systemTimeService: SystemTimeService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm()
      .loadStorageOptions()
      .watchChanges();
    this.initTable();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
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
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    let tenantName = find(this.vStoreOptions, {
      value: this.formGroup.value.vStore
    });
    let parentName = find(this.storageOptions, {
      uuid: this.formGroup.value.storage
    });

    const defaultConditions = {
      subType: [
        DataMap.Resource_Type.LocalFileSystem.value,
        DataMap.Job_Target_Type.fileSystem.value
      ],
      environment: {
        name: parentName.name
      },
      tenantName: tenantName.name
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditions = JSON.parse(filters.conditions_v2);
      if (conditions.parentName) {
        assign(defaultConditions, {
          environment: {
            name: tail(conditions.parentName)
          }
        });
        delete conditions.parentName;
      }
      assign(defaultConditions, conditions);
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });
    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        _map(res => {
          each(res.records, item => {
            assign(item, {
              tenantName: item.extendInfo?.tenantName
            });
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      reportName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.reportName, true),
          this.baseUtilService.VALID.maxLength(30)
        ]
      }),
      storage: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      vStore: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      detectTimeRange: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()],
        asyncValidators: [this.validTimeRange()]
      })
    });
    return this;
  }

  validTimeRange() {
    return (control: AbstractControl): Promise<{ [key: string]: any }> => {
      return new Promise(resolve => {
        this.systemTimeService.getSystemTime(false).subscribe(value => {
          const now = new Date(value.time).getTime();
          const selectTime = control?.value[1]?.getTime();
          if (selectTime > now) {
            resolve({ invaildTime: { value: control.value } });
          }
          resolve(null);
        });
      });
    };
  }

  loadStorageOptions() {
    this.detectReportApi.ListQueryResources({}).subscribe(res =>
      set(
        this,
        'storageOptions',
        map(res, item => ({
          label: `${item.name} (${item.endpoint})`,
          value: item.uuid,
          key: item.uuid,
          isLeaf: true,
          ...item
        }))
      )
    );
    return this;
  }

  watchChanges() {
    this.formGroup.get('storage')?.valueChanges.subscribe(value => {
      set(this, 'vStoreOptions', []);
      this.tableData = {
        data: [],
        total: 0
      };
      this.selectionData = [];
      this.dataTable.setSelections([]);
      this.formGroup.get('vStore').setValue('', { emitEvent: false });
      this.detectReportApi
        .ListQueryResources({ deviceId: value })
        .subscribe(res =>
          set(
            this,
            'vStoreOptions',
            map(res, item => ({
              label: item.name,
              value: item.uuid,
              key: item.uuid,
              isLeaf: true,
              ...item
            }))
          )
        );
    });
    this.formGroup.get('vStore')?.valueChanges.subscribe(value => {
      this.tableData = {
        data: [],
        total: 0
      };
      this.selectionData = [];
      this.dataTable.setSelections([]);
      defer(() => this.dataTable.fetchData());
    });

    this.systemTimeService.getSystemTime(false).subscribe(value => {
      const now = new Date(value.time);
      now.setHours(0);
      now.setMinutes(0);
      now.setSeconds(0);
      this.formGroup
        .get('detectTimeRange')
        .setValue([now, new Date(value.time)]);
    });
  }

  onOK() {
    const currentStorage = find(this.storageOptions, item =>
      eq(item.uuid, this.formGroup.get('storage').value)
    );
    const currentVStorage = find(this.vStoreOptions, item =>
      eq(item.uuid, this.formGroup.get('vStore').value)
    );
    return this.detectReportApi.AddDetectReport({
      AddDetectReportRequestBody: {
        reportName: this.formGroup.get('reportName').value,
        storageName: get(currentStorage, 'name'),
        storageId: get(currentStorage, 'uuid'),
        storageEndpoint: get(currentStorage, 'endpoint'),
        storagePort: get(currentStorage, 'port'),
        tenantName: get(currentVStorage, 'name'),
        tenantId: get(currentVStorage, 'uuid'),
        fileSystemName: this.selectionData[0].name,
        fileSystemId: this.selectionData[0].uuid,
        detectTimeRange: map(
          this.formGroup.get('detectTimeRange').value,
          item => this.datePipe.transform(item, 'yyyy-MM-ddTHH:mm:ss')
        )
      }
    });
  }
}
