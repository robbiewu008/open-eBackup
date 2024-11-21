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
import { DataMapService, I18NService, DataMap } from 'app/shared';

@Component({
  selector: 'aui-sla-detail',
  templateUrl: './sla-detail.component.html',
  styleUrls: ['./sla-detail.component.less'],
  providers: [DatePipe]
})
export class SlaDetailComponent implements OnInit {
  sla;
  isReplica;
  optItems;
  viewResource;
  activeIndex = '0';
  slaIcon = '';
  slaIconMap = {
    Gold: 'aui-sla-gold',
    Silver: 'aui-sla-silver-medium',
    Bronze: 'aui-sla-bronze'
  };

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public modal: ModalRef,
    public datePipe: DatePipe
  ) {}

  ngOnInit() {
    this.getModalHeader();
    this.isReplica =
      this.sla.application === DataMap.Application_Type.Replica.value;
    if (this.viewResource) {
      this.activeIndex = '1';
    }
    this.getSlaIcon();
  }

  getSlaIcon() {
    this.slaIcon = this.slaIconMap[this.sla?.name] || 'aui-sla-myvmprotect';
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  optCallback = data => {
    return this.optItems || [];
  };
}
