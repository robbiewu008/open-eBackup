import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap, I18NService, CommonConsts } from 'app/shared';
import { DatePipe } from '@angular/common';
import {
  ProTableComponent,
  TableData,
  TableConfig,
  TableCols
} from 'app/shared/components/pro-table';
import { size } from 'lodash';

@Component({
  selector: 'aui-anti-policy-detail',
  templateUrl: './anti-policy-detail.component.html',
  styleUrls: ['./anti-policy-detail.component.less'],
  providers: [DatePipe]
})
export class AntiPolicyDetailComponent implements OnInit {
  data;
  interval;
  copyTime;
  intervalUnit;
  detectionType;
  schedulePolicy;
  startDetectionTime;
  setWorm;
  needDetect;
  antiTamperingSetting;
  dataMap = DataMap;
  tableData: TableData;
  tableConfig: TableConfig;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.initTable();
    this.initTableData();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'resourceName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'resourceLocation',
        name: this.i18n.get('common_location_label')
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        virtualScroll: true,
        colDisplayControl: false,
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '320px' }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initTableData() {
    if (this.data && this.data.schedule) {
      this.copyTime = this.data.schedule.copyTime;
      this.interval = this.data.schedule.interval;
      this.intervalUnit = this.data.schedule.intervalUnit;
      this.detectionType = this.data.schedule.detectionType;
      this.schedulePolicy = this.data.schedule.schedulePolicy;
      this.startDetectionTime = this.data.schedule.startDetectionTime;
      this.tableData = {
        data: this.data.selectedResources ? this.data.selectedResources : [],
        total: size(this.data.selectedResources)
      };
      this.needDetect = this.data.schedule.needDetect;
      this.setWorm = this.data.schedule.setWorm;
      this.antiTamperingSetting = this.needDetect
        ? this.i18n.get('explore_anti_setting_02_label')
        : this.i18n.get('explore_anti_setting_01_label');
    }
  }
}
