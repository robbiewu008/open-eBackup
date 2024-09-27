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
import { Component, OnInit, ViewChild } from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';

import {
  BaseUtilService,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  SystemApiService
} from 'app/shared';

import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { includes } from 'lodash';
import { InitConfigProcessComponent } from '../init-config-process/init-config-process.component';
import { ConfigNetworkTableComponent } from '../config-network-table/config-network-table.component';

@Component({
  selector: 'aui-decouple-init',
  templateUrl: './decouple-init.component.html',
  styleUrls: ['./decouple-init.component.less']
})
export class DecoupleInitComponent implements OnInit {
  selectedTabIndex = 'backup';
  dataMap = DataMap;
  activeIndex = 0;
  componentData = {};
  serviceType = 'backup';
  selectedData: any = {};

  @ViewChild(InitConfigProcessComponent, { static: false })
  initConfigProcessComponent: InitConfigProcessComponent;
  @ViewChild(ConfigNetworkTableComponent, { static: false })
  configNetworkTableComponent: ConfigNetworkTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private router: Router,
    private i18n: I18NService,
    private cookieSerive: CookieService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private systemApiService: SystemApiService,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.virtualScroll.getScrollParam(370);
  }

  ngAfterViewInit() {
    this.getStatus();
  }

  onChangeTab(e) {
    if (e === 'backup') {
      this.virtualScroll.getScrollParam(370);
    } else {
      this.virtualScroll.getScrollParam(310);
    }
  }

  onResetChange(event) {
    this.systemApiService.getInitConfigUsingGET({}).subscribe(res => {
      if (res.status === DataMap.System_Init_Status.no.value) {
        this.router.navigate(['/home']);
        return;
      }
      this.activeIndex = 0;
    });
  }
  getStatus() {
    if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.running.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.getStatus();
    } else if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.archiveFailed.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.status =
        DataMap.System_Init_Status.archiveFailed.value;
      this.initConfigProcessComponent.result = {
        code: DataMap.System_Init_Status.archiveFailed.label,
        params: []
      };
    }
  }

  createInitConfig() {
    this.systemApiService
      .createInitConfigUsingPOST({
        initNetworkBody: {
          backupNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData['backup']
          },
          copyNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData['replication']
          },
          archiveNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData['archived']
          }
        },
        akOperationTips: false
      })
      .subscribe(res => {
        if (res.status === DataMap.System_Init_Status.ok.value) {
          this.router.navigate(['/home']);
          return;
        }

        if (res.status === DataMap.System_Init_Status.running.value) {
          this.activeIndex = 3;
          this.initConfigProcessComponent.getStatus();
          return;
        }

        if (
          includes(
            [
              DataMap.System_Init_Status.archiveFailed.value,
              DataMap.System_Init_Status.backupFailed.value,
              DataMap.System_Init_Status.authFailed.value,
              DataMap.System_Init_Status.failed.value
            ],
            res.status
          )
        ) {
          this.messageService.error(
            this.i18n.get(
              this.dataMapService.getLabel('System_Init_Status', res.status)
            ),
            {
              lvMessageKey: 'system_Init_Status_key',
              lvShowCloseButton: true
            }
          );
          return;
        }
      });
  }
}
