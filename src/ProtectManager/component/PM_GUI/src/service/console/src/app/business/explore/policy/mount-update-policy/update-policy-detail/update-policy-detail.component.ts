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
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  I18NService,
  RetentionPolicy,
  SchedulePolicy,
  DataMapService,
  LANGUAGE
} from 'app/shared';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-update-policy-detail',
  templateUrl: './update-policy-detail.component.html',
  styleUrls: ['./update-policy-detail.component.less'],
  providers: [DatePipe]
})
export class UpdatePolicyDetailComponent implements OnInit {
  data;
  schedulePolicyLabel;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );
  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private datePipe: DatePipe,
    private modal: ModalRef
  ) {}
  ngOnInit() {
    this.getData();
    this.updatetHeader();
  }

  getData() {
    if (this.data.schedulePolicy === SchedulePolicy.PeriodSchedule) {
      this.schedulePolicyLabel = this.i18n.get(
        'common_param_comma_param_label',
        [
          `${this.executionPeriodLabel}${this.spaceLabel}${
            this.data.scheduleInterval
          }${this.spaceLabel}${this.dataMapService.getLabel(
            'Interval_Unit',
            this.data.scheduleIntervalUnit
          )}`,
          `${this.firstExecuteTimeLabel}${this.spaceLabel}${this.getStartTime(
            this.data.scheduleStartTime
          )}`
        ]
      );
    }
  }

  getStartTime(displayTime) {
    let time: any;
    try {
      time = this.datePipe.transform(displayTime, 'yyyy-MM-dd HH:mm:ss');
    } catch (e) {
      time = displayTime;
    }
    return time;
  }

  updatetHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
