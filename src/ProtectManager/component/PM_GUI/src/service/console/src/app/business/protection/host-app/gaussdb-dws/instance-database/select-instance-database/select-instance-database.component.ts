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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  assign,
  each,
  first,
  get,
  has,
  isArray,
  isEmpty,
  isUndefined,
  set,
  size
} from 'lodash';
import { Subject } from 'rxjs';
import { map as _map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-instance-database-gaussdb-dws',
  templateUrl: './select-instance-database.component.html',
  styleUrls: ['./select-instance-database.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SelectInstanceDatabaseComponent implements OnInit {
  type;
  formGroup;
  allTableData = {};
  selectionData = [];
  valid$ = new Subject<boolean>();
  title = this.i18n.get('protection_selected_pvc_number_label', ['']);
  resourceType = DataMap.Resource_Type;
  backupOptions = this.dataMapService.toArray('Backup_Type').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  columns = [
    {
      key: 'name',
      name: this.i18n.get('common_name_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'ip',
      name: this.i18n.get('protection_host_cluster_name_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'sla_name',
      name: this.i18n.get('common_sla_label')
    }
  ];
  dataMap = DataMap;
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInput: this.i18n.get('protection_dws_metadata_path_error_tip_label')
  };

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private cdr: ChangeDetectorRef,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      path: new FormControl('', {
        validators: [this.baseUtilService.VALID.required(), this.validPath()]
      }),
      backup: new FormControl(DataMap.Backup_Type.GDS.value)
    });

    if (
      !isArray(this.selectionData) &&
      has(this.selectionData, 'protectedObject')
    ) {
      this.formGroup
        .get('path')
        .setValue(
          get(
            this.selectionData,
            'protectedObject.extParameters.backup_metadata_path'
          )
        );
      if (this.type === DataMap.Resource_Type.DWS_Table.value) {
        this.formGroup
          .get('backup')
          .setValue(
            get(
              this.selectionData,
              'protectedObject.extParameters.backup_tool_type'
            ) || DataMap.Backup_Type.GDS.value
          );
      }
    }

    this.formGroup.statusChanges.subscribe(res => {
      this.dataChange(this.selectionData);
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const reg_path = /[|;&$><`'\"!+\n]/;

      if (reg_path.test(control.value)) {
        return { invalidInput: { value: control.value } };
      }
      return null;
    };
  }

  updateTable(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      subType: [first(this.selectionData).subType]
    };

    if (!isEmpty(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        _map(res => {
          each(res.records, item => {
            extendSlaInfo(item);
            assign(item, {
              sub_type: item.subType,
              ip: item.extendInfo?.ip,
              disabled: !!get(item, 'sla_id')
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.allTableData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData) && this.formGroup.valid);
  }

  initData(data: any) {
    this.selectionData = data;
    isArray(this.selectionData)
      ? (this.type = first(this.selectionData).subType)
      : (this.type = data.subType);
  }

  onOK() {
    const params = {
      selectedList: this.selectionData,
      ext_parameters: {
        backup_metadata_path: this.formGroup.value.path
      }
    };

    if (this.type === DataMap.Resource_Type.DWS_Table.value) {
      set(
        params,
        'ext_parameters.backup_tool_type',
        DataMap.Backup_Type.GDS.value
      );
    }
    return params;
  }
}
