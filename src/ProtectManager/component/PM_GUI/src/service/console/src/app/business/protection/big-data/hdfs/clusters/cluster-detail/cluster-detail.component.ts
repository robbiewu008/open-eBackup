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
import { I18NService, DataMapService, DataMap } from 'app/shared';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-cluster-detail',
  templateUrl: './cluster-detail.component.html',
  styleUrls: ['./cluster-detail.component.less']
})
export class ClusterDetailComponent implements OnInit {
  data = {} as any;
  formItems = [];
  dataMap = DataMap;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private modal: ModalRef,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.getModalHeader();
    this.initFormItems();
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
        },
        {
          key: 'endpoint',
          value: this.data.endpoint,
          label: this.i18n.get('fs.defaultFS')
        }
      ],
      [
        {
          key: 'linkStatus',
          value: this.data.linkStatus,
          label: this.i18n.get('common_status_label')
        },
        {
          key: 'proxyHosts',
          value: this.data.proxyHosts,
          label: this.i18n.get('protection_client_label')
        }
      ]
    ];
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
