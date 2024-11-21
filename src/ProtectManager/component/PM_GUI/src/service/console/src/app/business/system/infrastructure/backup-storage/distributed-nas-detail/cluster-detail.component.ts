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
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CAPACITY_UNIT,
  ColorConsts,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  NasDistributionStoragesApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, isEmpty, isUndefined } from 'lodash';

@Component({
  selector: 'aui-cluster-detail',
  templateUrl: './cluster-detail.component.html',
  styleUrls: ['./cluster-detail.component.less']
})
export class ClusterDetailComponent implements OnInit, AfterViewInit {
  hosts;
  formItems = [];
  unitconst = CAPACITY_UNIT;
  datamap = DataMap;
  tableData;
  resSubType;
  tableConfig: TableConfig;
  progressBarColor = [[0, ColorConsts.NORMAL]];
  protected readonly Math = Math;
  backupUnitStatus: any = this.dataMapService.toArray(
    'StoragePoolRunningStatus'
  );
  health_Status = this.dataMapService.toArray('HealthStatus');
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('capacity', { static: true })
  capacity: TemplateRef<any>;
  @ViewChild('thresholdTpl', { static: true })
  thresholdTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  @Input() data;
  constructor(
    public appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMap: DataMapService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'deviceId',
        name: 'deviceId',
        hidden: true
      },
      {
        key: 'unitName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'healthStatus',
        name: this.i18n.get('common_health_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.health_Status
        },
        cellRender: {
          type: 'status',
          config: this.dataMap.toArray('HealthStatus')
        }
      },
      {
        key: 'runningStatus',
        name: this.i18n.get('protection_running_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.backupUnitStatus
        },
        cellRender: {
          type: 'status',
          config: this.dataMap.toArray('StoragePoolRunningStatus')
        }
      },
      {
        key: 'capacity',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.capacity
      },
      {
        key: 'threshold',
        name: this.i18n.get('common_alarm_threshold_label'),
        cellRender: this.thresholdTpl
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'clusterId',
        size: 'default',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '420px' },
        colDisplayControl: false,
        autoPolling: CommonConsts.TIME_INTERVAL,
        fetchData: (filter: Filters, args) => this.getData(filter, args)
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  getData(filters: Filters, args) {
    const params = {
      id: this.data.uuid,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (filters && !isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.unitName) {
        assign(params, {
          name: conditionsTemp.unitName
        });
      }
      if (conditionsTemp.healthStatus) {
        assign(params, {
          healthStatusList: conditionsTemp.healthStatus
        });
      }
      if (conditionsTemp.runningStatus) {
        assign(params, {
          runningStatusList: conditionsTemp.runningStatus
        });
      }
    }

    this.nasDistributionStoragesApiService
      .NasDistributionStorageInfo(params)
      .subscribe(res => {
        each(res.unitList, item => {
          assign(item, {
            usedPercent: (item.usedCapacity * 100) / item.totalCapacity
          });
        });
        this.tableData = {
          data: res.unitList,
          total: res.unitList.length
        };
        this.cdr.detectChanges();
      });
  }
}
