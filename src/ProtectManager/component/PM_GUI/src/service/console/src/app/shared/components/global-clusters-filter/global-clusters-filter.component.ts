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
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  I18NService
} from 'app/shared';
import { filter, find, includes, isUndefined, size, union } from 'lodash';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from '../pro-table';
import { Router } from '@angular/router';

@Component({
  selector: 'aui-global-clusters-filter',
  templateUrl: './global-clusters-filter.component.html',
  styleUrls: ['./global-clusters-filter.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class GlobalClustersFilterComponent implements OnInit, AfterViewInit {
  @Input() currentCluster;
  _isUndefined = isUndefined;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  startPage = CommonConsts.PAGE_START;
  pageSize = 200;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private clusterApiService: ClustersApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'clusterName',
        name: '',
        cellRender: this.nameTpl
      }
    ];

    this.tableConfig = {
      pagination: null,
      table: {
        size: 'small',
        compareWith: 'clusterId',
        columns: cols,
        rows: {
          showSelector: false,
          selectionMode: 'single',
          selectionTrigger: 'row'
        },
        colDisplayControl: false,
        showLoading: true,
        fetchData: () => this.getData(),
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.clusterId;
        }
      }
    };
  }

  getData() {
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: this.startPage,
        pageSize: this.pageSize,
        akLoading: false,
        clustersId: '1',
        clustersType: '1',
        roleList: [
          DataMap.Target_Cluster_Role.managed.value,
          DataMap.Target_Cluster_Role.management.value
        ]
      })
      .subscribe((res: any) => {
        const allClusters =
          this.router.url !== '/home' && this.isDataBackup
            ? []
            : [
                {
                  clusterId: DataMap.Cluster_Type.local.value,
                  clusterName: this.i18n.get('common_all_clusters_label'),
                  clusterType: DataMap.Cluster_Type.local.value,
                  icon: 'aui-icon-all-cluster',
                  enableManage: true,
                  isAllCluster: true,
                  role: 2
                }
              ];

        const managementClusters = union(
          [
            {
              haveDashed: true,
              clusterName: this.i18n.get('common_management_label')
            }
          ],
          filter(
            res.records,
            item =>
              item.role === 2 &&
              includes(
                [
                  DataMap.Cluster_Type.local.value,
                  DataMap.Cluster_Type.target.value
                ],
                item.clusterType
              ) &&
              item.enableManage
          ).map(item => {
            item['icon'] = 'aui-icon-single-cluster';
            item['clusterName'] =
              item['clusterType'] === DataMap.Cluster_Type.local.value
                ? item['clusterName'] +
                  `(${this.i18n.get('common_local_label')})`
                : item['status'] === DataMap.Cluster_Status.offline.value
                ? `${item.clusterName}(${this.i18n.get('common_off_label')})`
                : item.clusterName;
            return item;
          })
        );

        const managedClusters = union(
          [
            {
              haveDashed: true,
              clusterName: this.i18n.get('common_managed_label')
            }
          ],
          filter(
            res.records,
            item =>
              item.role === 2 &&
              !(
                includes(
                  [
                    DataMap.Cluster_Type.local.value,
                    DataMap.Cluster_Type.target.value
                  ],
                  item.clusterType
                ) && item.enableManage
              )
          ).map(item => {
            item['icon'] = 'aui-icon-single-cluster';
            item['clusterName'] =
              item['status'] === DataMap.Cluster_Status.offline.value
                ? `${item.clusterName}(${this.i18n.get('common_off_label')})`
                : item.clusterName;
            return item;
          })
        );

        const data = !!size(
          find(managedClusters, item => item.enableManage === false)
        )
          ? union(allClusters, managementClusters, managedClusters)
          : union(allClusters, managementClusters);
        this.tableData = {
          data,
          total: size(data)
        };
        this.cdr.detectChanges();
      });
  }

  reload(item) {
    if (
      !item.clusterId ||
      item.status === DataMap.Cluster_Status.offline.value
    ) {
      return;
    }

    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: this.startPage,
        pageSize: 1,
        clustersId: item.clusterId,
        clustersType: item.clusterType,
        typeList: [DataMap.Cluster_Type.local.value]
      })
      .subscribe(() => {
        this.cookieService.set(
          'currentCluster',
          encodeURIComponent(JSON.stringify(item))
        );
        window.location.reload();
      });
  }
}
