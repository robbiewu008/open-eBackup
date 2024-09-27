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
import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-cluster-backupset-detail-hbase',
  templateUrl: './cluster-backupset-detail.component.html',
  styleUrls: ['./cluster-backupset-detail.component.less']
})
export class ClusterBackupsetDetailComponent implements OnInit {
  data = {} as any;
  formItems = [];
  resSubType;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.data = data;

    this.data.subType === DataMap.Resource_Type.Hive.value
      ? (this.resSubType = DataMap.Resource_Type.HiveBackupSet.value)
      : this.data.subType === DataMap.Resource_Type.Elasticsearch.value
      ? (this.resSubType = DataMap.Resource_Type.ElasticsearchBackupSet.value)
      : (this.resSubType = DataMap.Resource_Type.HBaseBackupSet.value);
  }
}
