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
import { Component, OnInit, Injectable } from '@angular/core';
import { MODAL_COMMON, I18NService } from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';

@Component({
  selector: 'aui-ip-config-list',
  templateUrl: './ip-config-list.component.html',
  styleUrls: ['./ip-config-list.component.less']
})
export class IpConfigListComponent implements OnInit {
  data;

  constructor() {}

  ngOnInit() {}
}

@Injectable({
  providedIn: 'root'
})
export class IpConfigListService {
  private ipConfigListComponent = IpConfigListComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService
  ) {}

  create(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.smallWidth,
      lvModalKey: 'ip_segment_list',
      ...{
        lvHeader: this.i18n.get('common_more_label'),
        lvContent: this.ipConfigListComponent,
        lvComponentParams: {
          data
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      }
    });
  }
}
