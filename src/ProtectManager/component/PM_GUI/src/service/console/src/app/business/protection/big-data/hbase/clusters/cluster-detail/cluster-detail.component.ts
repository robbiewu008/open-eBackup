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
  Component,
  OnInit,
  TemplateRef,
  ViewChild,
  ChangeDetectorRef
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap,
  CommonConsts
} from 'app/shared';
import {
  Filters,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  isNumber,
  map,
  split,
  toString as _toString
} from 'lodash';

@Component({
  selector: 'aui-cluster-detail',
  templateUrl: './cluster-detail.component.html',
  styleUrls: ['./cluster-detail.component.less']
})
export class ClusterDetailComponent implements OnInit {
  data = {} as any;
  formItems = [];
  tableConfig: TableConfig;
  tableData;
  hosts;
  resSubType;

  constructor(
    private i18n: I18NService,
    private dataMap: DataMapService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initFormItems();
    this.initConfig();
    this.getAgents();
  }

  initDetailData(data: any) {
    this.data = data;
    this.hosts = split(this.data?.proxyHosts, '/');
    this.data.subType === DataMap.Resource_Type.Hive.value
      ? (this.resSubType = DataMap.Resource_Type.HiveBackupSet.value)
      : this.data.subType === DataMap.Resource_Type.Elasticsearch.value
      ? (this.resSubType = DataMap.Resource_Type.ElasticsearchBackupSet.value)
      : (this.resSubType = DataMap.Resource_Type.HBaseBackupSet.value);

    if (this.data.subType === DataMap.Resource_Type.Elasticsearch.value) {
      this.data = assign(cloneDeep(data), {
        endpoint: this.data?.extendInfo?.ElasticSearchAddress
      });
    }
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
          key: 'authType',
          value: this.dataMap.getLabel(
            'HDFS_Clusters_Auth_Type',
            this.data?.authType
          ),
          label: this.i18n.get('protection_auth_mode_label')
        }
      ],
      [
        {
          key: 'endpoint',
          value: this.data.endpoint,
          label:
            this.data.subType === DataMap.Resource_Type.Hive.value
              ? this.i18n.get('protection_hive_server_link_label')
              : this.data.subType === DataMap.Resource_Type.Elasticsearch.value
              ? this.i18n.get('protection_server_ip_label')
              : this.i18n.get('common_ip_domain_name_label')
        }
      ]
    ];
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label')
      },

      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMap.toArray('resource_LinkStatus_Special')
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'ip',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '420px' },
        colDisplayControl: false
      },
      pagination: null
    };
  }

  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: [`${this.data.subType}BackupSetPlugin`],
          environment: {}
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          const proxyHostOptions = map(recordsTemp, item => {
            return {
              ...item,
              key: item.uuid,
              label: `${item.environment?.name}(${item.environment?.endpoint})`,
              value: item.rootUuid || item.parentUuid,
              isLeaf: true
            };
          });
          const dataList = [];
          each(proxyHostOptions, item => {
            const host = find(
              this.hosts,
              val => val === item?.environment?.endpoint
            );

            if (host) {
              dataList.push({
                name: item.environment?.name,
                ip: item.environment?.endpoint,
                status: item.environment?.linkStatus
              });
            }
          });

          this.tableData = {
            data: dataList,
            total: dataList.length
          };
          this.cdr.detectChanges();
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
  }
}
