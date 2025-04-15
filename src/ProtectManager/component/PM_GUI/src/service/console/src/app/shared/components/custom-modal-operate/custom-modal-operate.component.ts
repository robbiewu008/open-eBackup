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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { GlobalService } from 'app/shared';
import { Subscription } from 'rxjs';

@Component({
  selector: 'aui-custom-modal-operate',
  templateUrl: './custom-modal-operate.component.html',
  styleUrls: ['./custom-modal-operate.component.less']
})
export class CustomModalOperateComponent implements OnInit, OnDestroy {
  header;
  @Input() item;
  subscription$: Subscription;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy() {
    this.subscription$.unsubscribe();
  }

  ngOnInit() {
    this.init();
    this.autoRefreshOpt();
  }

  init() {
    this.header =
      this.item.name && this.item.ip
        ? `${this.item.name}(${this.item.ip})`
        : `${this.item.name}`;
  }

  optCallback = data => {
    return this.item.optItems || [];
  };

  autoRefreshOpt() {
    this.subscription$ = this.globalService
      .getState('autoReshResource')
      .subscribe(res => {
        if (this.item.optItemsFn) {
          this.optCallback = data => {
            return this.item.optItemsFn(res) || [];
          };
        }
      });
  }
}
