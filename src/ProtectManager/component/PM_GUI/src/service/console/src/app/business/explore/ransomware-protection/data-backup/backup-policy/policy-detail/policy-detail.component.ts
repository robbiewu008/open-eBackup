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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import { DataMap, I18NService, TRIGGER_TYPE } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-policy-detail',
  templateUrl: './policy-detail.component.html',
  styleUrls: ['./policy-detail.component.less'],
  providers: [DatePipe]
})
export class PolicyDetailComponent implements OnInit {
  policy: any;
  optItems: any[];
  activeIndex: any;
  triggerMap = {
    [TRIGGER_TYPE.hour]: this.i18n.get('common_by_hour_label'),
    [DataMap.Interval_Unit.hour.value]: this.i18n.get('common_by_hour_label'),
    [TRIGGER_TYPE.day]: this.i18n.get('common_by_day_label'),
    [DataMap.Interval_Unit.day.value]: this.i18n.get('common_by_day_label'),
    [TRIGGER_TYPE.week]: this.i18n.get('common_by_week_label'),
    [TRIGGER_TYPE.month]: this.i18n.get('common_by_month_label'),
    [TRIGGER_TYPE.year]: this.i18n.get('common_by_year_label')
  };

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    public i18n: I18NService,
    public datePipe: DatePipe,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    if (this.policy?.isResource) {
      this.activeIndex = '1';
    }
    this.getModalHeader();
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  optCallback = () => {
    return this.optItems || [];
  };

  getDetectionPolicy(backupItem): string {
    return this.appUtilsService.getDetectionPolicy(backupItem, this.datePipe);
  }
}
