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
  OnChanges,
  OnInit,
  SimpleChanges,
  ViewChild
} from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DatatableComponent, PaginatorComponent } from '@iux/live';
import { CommonConsts, I18NService } from 'app/shared';
import { CopiesService } from 'app/shared/api/services';
import { DataMap } from 'app/shared/consts';
import { BaseUtilService } from 'app/shared/services';
import { first, includes, isEmpty, trim } from 'lodash';

@Component({
  selector: 'aui-database-config',
  templateUrl: './database-config.component.html',
  styleUrls: ['./database-config.component.less']
})
export class DatabaseConfigComponent implements OnInit, OnChanges {
  includes = includes;
  pageSizeOptions = CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  dataMap = DataMap;
  resourceType;
  tableData = [];
  cacheTableData = [];
  newParamErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [512])
  };
  queryOriginParam;
  queryKey;
  newFilterMap = [
    {
      key: 'input',
      value: 'input',
      label: this.i18n.get('protection_db_restore_input_label')
    },
    {
      key: 'empty',
      value: 'empty',
      label: this.i18n.get('protection_db_restore_empty_label')
    }
  ];

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild(PaginatorComponent, { static: false }) lvPage: PaginatorComponent;

  @Input() formGroup: FormGroup;
  @Input() rowCopy;
  @Input() resSubType;
  @Input() isDrill;
  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private copiesService: CopiesService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.formGroup.addControl('isModify', new FormControl(false));
    this.resourceType = first(this.resSubType);
  }

  updateDrillData(key) {
    if (
      this.isDrill &&
      !isEmpty(this.rowCopy?.drillRecoveryConfig) &&
      !isEmpty(this.rowCopy?.drillRecoveryConfig.parameters?.mount_target_host)
    ) {
      const mountTarget = JSON.parse(
        this.rowCopy?.drillRecoveryConfig.parameters?.mount_target_host
      );
      return mountTarget[key] || '';
    }
    return '';
  }

  ngOnChanges(changes: SimpleChanges) {
    if (!isEmpty(changes.rowCopy.currentValue)) {
      if ((this.formGroup.get('dbConfig') as FormArray).controls.length) {
        (this.formGroup.get('dbConfig') as FormArray).controls.forEach(
          (control, index) => {
            this.pushTableData(
              index,
              control.value.key,
              control.value.originParam
            );
          }
        );
      }
      const params = changes.rowCopy.currentValue.restoreTimeStamp
        ? {
            time: changes.rowCopy.currentValue.restoreTimeStamp,
            resourceId: changes.rowCopy.currentValue.dbUuid
          }
        : changes.rowCopy.currentValue.scn
        ? {
            systemChangeNumber: changes.rowCopy.currentValue.scn,
            resourceId: changes.rowCopy.currentValue.dbUuid
          }
        : {
            copyId: changes.rowCopy.currentValue.uuid
          };
      if (
        !includes(
          [
            DataMap.Resource_Type.MySQLCluster.value,
            DataMap.Resource_Type.MySQLClusterInstance.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ],
          changes.rowCopy.currentValue.resource_sub_type
        )
      ) {
        this.tableData = [];
        this.copiesService
          .queryResourcesV1CopiesGet({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            conditions: JSON.stringify({ uuid: this.rowCopy.uuid })
          })
          .subscribe(res => {
            if (isEmpty(res.items)) {
              return;
            }
            const data = JSON.parse(res.items[0].properties || '{}')
              .pfileParams;
            let index = 0;
            for (let key in data) {
              (this.formGroup.get('dbConfig') as FormArray).push(
                this.fb.group({
                  key: new FormControl(key),
                  originParam: new FormControl(data[key]),
                  newParam: new FormControl(this.updateDrillData(key), {
                    validators: [this.baseUtilService.VALID.maxLength(512)]
                  }),
                  newWriteable: new FormControl(false)
                })
              );
              this.pushTableData(index, key, data[key]);
              index++;
            }
            this.tableData = [...this.tableData];
          });
      }
    }
  }

  searchByOriginParam(queryOriginParam) {
    this.lvTable.filter({
      key: 'originParam',
      value: trim(queryOriginParam),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  searchByKey(queryKey) {
    this.lvTable.filter({
      key: 'key',
      value: trim(queryKey),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  filterChange(event) {
    this.tableData = [];
    (this.formGroup.get('dbConfig') as FormArray).controls.forEach(
      (control, index) => {
        if (
          (event.value.includes('input') && event.value.includes('empty')) ||
          !event.value.length
        ) {
          this.pushTableData(
            index,
            control.value.key,
            control.value.originParam
          );
        } else if (event.value.includes('input') && control.value.newParam) {
          this.pushTableData(
            index,
            control.value.key,
            control.value.originParam
          );
        } else if (event.value.includes('empty') && !control.value.newParam) {
          this.pushTableData(
            index,
            control.value.key,
            control.value.originParam
          );
        }
      }
    );
  }

  pushTableData(id, key, originParam) {
    this.tableData.push({ id, key, originParam });
  }
}
