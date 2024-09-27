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
import { Component, Input } from '@angular/core';
import { ReportDataService, I18NService } from 'app/shared';

@Component({
  selector: 'top-failed-tasks-resource-objects',
  templateUrl: './top-failed-tasks-resource-objects.component.html',
  styleUrls: ['./top-failed-tasks-resource-objects.component.less']
})
export class TopFailedTasksResourceObjectsComponent {
  @Input() cardInfo: any = {};
  data = [];
  constructor(
    private reportDataService: ReportDataService,
    private i18n: I18NService
  ) {}

  ngOnInit(): void {
    this.refreshData();
  }

  refreshData() {
    this.cardInfo.loading = true;
    let timeRange;
    switch (this.cardInfo.selectTime) {
      case 4:
        timeRange = 'LAST_WEEK';
        break;
      case 5:
        timeRange = 'LAST_MONTH';
        break;
    }
    this.reportDataService
      .QueryProtectTask({
        akLoading: false,
        akOperationTips: false,
        QueryProtectTaskRequestBody: {
          timeRange,
          dataQueryTypeEnum: 'RESOURCE'
        }
      })
      .subscribe(res => {
        let { resourceTaskSummary } = res;
        this.data = resourceTaskSummary;
        this.cardInfo.loading = false;
      });
  }
}
