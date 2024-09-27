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
import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';

@Component({
  selector: 'aui-environment-info',
  templateUrl: './environment-info.component.html',
  styleUrls: ['./environment-info.component.less']
})
export class EnvironmentInfoComponent implements OnInit {
  data;
  treeSelection;
  online = true;
  formItems;
  interval;

  @ViewChild('timeTpl', { static: false })
  timeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    public dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.data = this.treeSelection;
    this.getResource();
  }

  getResource() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.treeSelection.uuid
      })
      .subscribe((res: any) => {
        this.online =
          res.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
        this.interval = res.scanInterval / 3600;
        this.formItems = [
          [
            {
              label: this.i18n.get('common_name_label'),
              content: this.data.name
            },
            {
              label: this.i18n.get('common_ip_label'),
              content: this.data.endpoint
            },
            {
              label: this.i18n.get('common_username_label'),
              content: this.data.userName
            }
          ],
          [
            {
              label: this.i18n.get('common_port_label'),
              content: this.data.port
            },
            {
              label: this.i18n.get('protection_register_vm_rescan_label'),
              content: `${this.interval}${this.i18n.get('common_hour_label')}`
            }
          ]
        ];
      });
  }
}
