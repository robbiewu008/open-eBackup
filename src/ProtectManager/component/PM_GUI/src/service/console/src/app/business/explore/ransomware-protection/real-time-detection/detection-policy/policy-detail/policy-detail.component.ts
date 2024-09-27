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
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-policy-detail',
  templateUrl: './policy-detail.component.html',
  styleUrls: ['./policy-detail.component.less']
})
export class PolicyDetailComponent implements OnInit {
  rowData;
  activeIndex;
  formItems = [];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {
    this.getFormItems();
  }

  getFormItems() {
    this.formItems = [
      {
        key: 'retention_duration',
        label: this.i18n.get('explore_snapshot_lock_time_label'),
        content: this.rowData?.retentionDuration
          ? this.i18n.get('explore_days_label', [
              this.rowData?.retentionDuration
            ])
          : ''
      },
      {
        key: 'alarm_analysis',
        label: this.i18n.get('explore_alarm_analysis_label'),
        content: this.rowData?.isIoEnhancedEnabled
      },
      {
        key: 'detection_status',
        label: this.i18n.get('explore_decoy_detection_status_label'),
        content: this.rowData?.isHoneypotDetectEnable
      },
      {
        label: this.i18n.get('explore_decoy_update_frequency_label'),
        content: this.rowData?.isHoneypotDetectEnable
          ? this.rowData?.period
            ? this.i18n.get('explore_decoy_update_label', [
                this.rowData?.period
              ])
            : this.i18n.get('explore_honeypot_not_update_label')
          : ''
      },
      {
        label: this.i18n.get('explore_association_file_system_num_label'),
        content: this.rowData?.associationFsNum
      },
      {
        label: this.i18n.get('explore_associated_whitelist_count_label'),
        content: this.rowData?.whiteListNum
      }
    ];
  }
}
