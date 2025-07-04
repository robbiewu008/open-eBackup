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
import { Component, OnInit } from '@angular/core';
import { CAPACITY_UNIT, DataMap, I18NService, SYSTEM_TIME } from 'app/shared';
import { includes, isNumber } from 'lodash';

@Component({
  selector: 'aui-snapshot-detail',
  templateUrl: './snapshot-detail.component.html',
  styleUrls: ['./snapshot-detail.component.less']
})
export class SnapshotDetailComponent implements OnInit {
  rowData: any;
  formItems = [];
  copyStatus = DataMap.snapshotCopyStatus;
  _includes = includes;
  _isNumber = isNumber;
  unitconst = CAPACITY_UNIT;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.initFormItems();
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          label: this.i18n.get('protection_hyperdetect_copy_name_label'),
          value: this.rowData?.name
        },
        {
          key: 'status',
          label: this.i18n.get('common_status_label'),
          value: this.rowData?.status
        },
        {
          key: 'generate_type',
          label: this.i18n.get('common_generated_type_label'),
          value: this.rowData?.generate_type
        },
        {
          key: 'is_security_snapshot',
          label: this.i18n.get('explore_security_snapshot_label'),
          value: this.rowData?.is_security_snapshot
        },
        {
          key: 'expiration_time',
          label: this.i18n.get('explore_snapshot_expire_time_label'),
          value: this.rowData?.expiration_time
        }
      ],
      [
        {
          key: 'total_file_size',
          label: this.i18n.get('explore_total_file_size_label'),
          value: this.rowData?.total_file_size
        },
        {
          key: 'added_file_count',
          label: this.i18n.get('explore_new_file_num_label'),
          value: this.rowData?.added_file_count
        },
        {
          key: 'changed_file_count',
          label: this.i18n.get('explore_modify_file_count_label'),
          value: this.rowData?.changed_file_count
        },
        {
          key: 'deleted_file_count',
          label: this.i18n.get('explore_delete_file_count_label'),
          value: this.rowData?.deleted_file_count
        },
        {
          key: 'detection_time',
          label: this.i18n.get('explore_detect_end_time_label'),
          value: this.rowData?.detection_time
        }
      ]
    ];
  }
}
