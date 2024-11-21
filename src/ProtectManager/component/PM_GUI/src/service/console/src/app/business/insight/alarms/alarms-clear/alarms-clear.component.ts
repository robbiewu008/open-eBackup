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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import { I18NService } from 'app/shared';
import { map } from 'lodash';

@Component({
  selector: 'aui-alarms-clear',
  templateUrl: './alarms-clear.component.html',
  styleUrls: ['./alarms-clear.component.less']
})
export class AlarmsClearComponent implements OnInit {
  selectionData;
  isAlarm;
  isCyberEngine;
  pageSize = 10;
  pageIndex = 0;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(public i18n: I18NService, private modal: ModalRef) {}

  ngOnInit() {
    this.updateHeader();
    this.isCyberEngine && this.updateCyberEngineData();
  }

  updateCyberEngineData() {
    this.selectionData = map(this.selectionData, item => ({
      ...item,
      name: item.alarmName,
      objctType: item.sourceType
    }));
  }

  updateHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
