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
import { Component, OnInit, Input } from '@angular/core';
import { DataMap, DataMapService, I18NService } from 'app/shared';

@Component({
  selector: 'aui-basic-info',
  templateUrl: './basic-info.component.html',
  styleUrls: ['./basic-info.component.less']
})
export class BasicInfoComponent implements OnInit {
  formItems;

  @Input() data;
  constructor(private i18n: I18NService, private dataMap: DataMapService) {}

  ngOnInit(): void {
    this.initFormItems();
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
        }
      ],
      [
        {
          key: 'clusterType',
          value: this.dataMap.getLabel(
            'Mysql_Cluster_Type',
            this.data?.clusterType
          ),
          label: this.i18n.get('common_type_label')
        }
      ]
    ];
  }
}
