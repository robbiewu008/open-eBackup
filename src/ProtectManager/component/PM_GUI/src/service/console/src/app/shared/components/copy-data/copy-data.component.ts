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
import { DatePipe } from '@angular/common';
import {
  Component,
  Input,
  OnInit,
  ViewChild,
  ViewEncapsulation
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  COPY_OPERATION,
  DATE_PICKER_MODE,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { assign, find, isEmpty, trim } from 'lodash';
import { CopyDataListComponent } from '../copy-data-list/copy-data-list.component';
import { CopyDataSearchComponent } from '../copy-data-search/copy-data-search.component';
import { DataMap } from './../../consts/data-map.config';

@Component({
  selector: 'aui-copy-data',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less'],
  providers: [DatePipe],
  encapsulation: ViewEncapsulation.None
})
export class CopyDataComponent implements OnInit {
  @Input() id: string; // 资源id
  @Input() opItems: any; // 操作项
  @Input() rowData: any; // 表格行数据
  @Input() resType;

  pointTime = new Date('2020/02/14 12:00:00');
  groupOptions = GROUP_COMMON;
  resourceResourceType = DataMap.Resource_Type;
  dataMap = DataMap;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isOpenGaussPanweiDB = false; // openGauss该数据库的实例支持日志备份

  copyDataInCalendar = [];

  // 日历类型选项
  datePickerMode = DATE_PICKER_MODE.DATE;
  showDayView = false;
  dateValue: Date = new Date();

  // 滑块
  val = 10;
  lines = [];
  points = [];
  valTips;
  realVal = '10:10:10';
  handleColor;
  timeValue = new Date();
  copyOperation = COPY_OPERATION;
  copyDataInDayView;

  @ViewChild(CopyDataListComponent, { static: false })
  copyDataListComponent: CopyDataListComponent;

  constructor(
    public i18n: I18NService,
    private drawModalService: DrawModalService,
    public copiesApiService: CopiesService,
    public datePipe: DatePipe,
    private infoMessageService: InfoMessageService,
    private cookieService: CookieService
  ) {}

  ngOnInit() {
    this.initData();
    this.isOpenGaussPanweiDB =
      this.rowData.subType ===
        this.resourceResourceType.OpenGauss_instance.value &&
      this.rowData.extendInfo.clusterVersion.includes('PanWeiDB');

    if (!this.rowData.datePickerMode) {
      this.getCopyData();
    }
  }

  initData() {
    if (!this.opItems) {
      this.opItems = [];
    }
    if (this.rowData.datePickerMode) {
      this.datePickerMode = this.rowData.datePickerMode;
      this.dateModeChange(this.datePickerMode);
    }
    if (this.rowData.checkedDate) {
      this.dateValue = this.rowData.checkedDate;
    }
  }

  clearIndex() {
    this.infoMessageService.create({
      content: this.i18n.get('protection_clear_index_info_label', [
        this.rowData.name
      ]),
      onOK: () => {
        this.copiesApiService
          .DeleteResourceIndex({
            resourceId: this.rowData.uuid
          })
          .subscribe(res => {
            if (this.datePickerMode === DATE_PICKER_MODE.SIMPLE) {
              this.copyDataListComponent.getCopyData();
            } else {
              this.getCopyData();
            }
          });
      }
    });
  }

  searchChange(source) {
    if (!trim(source)) {
      return;
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'copyDataSearchKey',
      lvWidth: MODAL_COMMON.xLargeWidth,
      lvHeader: this.i18n.get('protection_file_search_result_label'),
      lvContent: CopyDataSearchComponent,
      lvComponentParams: {
        resourceData: {
          ...this.rowData,
          searchKey: trim(source)
        }
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  // 恢复 立即恢复 挂载操作
  operationClick(operationType) {}

  // 获取View副本数据
  getCopyData(params?) {
    const defaultParams = {
      view: 'month',
      timePoint: this.datePipe.transform(this.dateValue, 'yyyy-MM'),
      resourceId: this.id
    };
    this.copiesApiService
      .queryCopyStatisticsV1CopiesStatisticsGet(
        assign({}, defaultParams, params)
      )
      .subscribe(res => {
        this.copyDataInCalendar = res;
      });
  }

  // Month视图下的副本数据数量
  getCopyDataInMonthView(cellContent) {
    const list = find(this.copyDataInCalendar, {
      index: this.datePipe.transform(cellContent, 'yyyy-MM-dd')
    });
    if (this.isCyberEngine) {
      return !isEmpty(list) && !list.infected_count ? list.count : 0;
    }
    return !isEmpty(list) ? list.count : 0;
  }

  // 安全一体机Month视图下的感染副本数据数量
  getInfectedCopyDataInMonthView(cellContent) {
    if (!this.isCyberEngine) {
      return 0;
    }
    const list = find(this.copyDataInCalendar, {
      index: this.datePipe.transform(cellContent, 'yyyy-MM-dd')
    });
    return !isEmpty(list) ? list.infected_count : 0;
  }

  // Year视图下的副本数据数量
  getCopyDataInYearView(cellContent) {
    const list = find(this.copyDataInCalendar, {
      index: this.datePipe.transform(cellContent, 'yyyy-MM')
    });
    if (this.isCyberEngine) {
      return !isEmpty(list) && !list.infected_count ? list.count : 0;
    }
    return !isEmpty(list) ? list.count : 0;
  }

  // Year视图下的感染副本数据数量
  getInfectedCopyDataInYearView(cellContent) {
    if (!this.isCyberEngine) {
      return 0;
    }
    const list = find(this.copyDataInCalendar, {
      index: this.datePipe.transform(cellContent, 'yyyy-MM')
    });
    return !isEmpty(list) ? list.infected_count : 0;
  }

  datePickerChange(date) {
    assign(this.rowData, {
      checkedDate: date
    });
    if (this.datePickerMode === DATE_PICKER_MODE.SIMPLE) {
      this.changeToDayView(date);
      return;
    }
    this.dateValue = date;
    if (this.datePickerMode === DATE_PICKER_MODE.MONTH) {
      this.getCopyData({
        view: 'year',
        timePoint: this.datePipe.transform(this.dateValue, 'yyyy')
      });
    } else {
      this.getCopyData();
    }
  }

  // 通过tab切换视图
  dateModeChange(mode) {
    assign(this.rowData, {
      datePickerMode: mode
    });
    this.datePickerMode = mode;
    if (mode === DATE_PICKER_MODE.SIMPLE) {
      !this.showDayView && this.changeToDayView(this.dateValue);
      this.showDayView = true;
    } else {
      this.showDayView = false;
      if (this.datePickerMode === DATE_PICKER_MODE.MONTH) {
        this.getCopyData({
          view: 'year',
          timePoint: this.datePipe.transform(this.dateValue, 'yyyy')
        });
      } else {
        this.getCopyData();
      }
    }
  }

  // 通过日历切换到Day视图
  changeToDayView(cellContent) {
    assign(this.rowData, {
      checkedDate: new Date(cellContent)
    });
    this.dateValue = new Date(cellContent);
    this.datePickerMode = DATE_PICKER_MODE.SIMPLE;
    this.showDayView = true;
    this.copyDataInDayView = this.dateValue;
  }

  dealPoints(value) {
    const time =
      value.getHours() * 3600 + value.getMinutes() * 60 + value.getSeconds();
    return (time / (24 * 3600)) * 100;
  }

  // 通过today按钮切换
  changeToTodayView() {
    this.datePickerMode = DATE_PICKER_MODE.SIMPLE;
    this.changeToDayView(new Date());
  }

  changeToMonthView(cellValue) {
    this.datePickerMode = DATE_PICKER_MODE.DATE;
    this.dateValue = new Date(cellValue);
    assign(this.rowData, {
      checkedDate: new Date(cellValue)
    });
    this.dateModeChange(DATE_PICKER_MODE.DATE);
  }

  calculatePoint(index) {
    return (100 / 24) * 4 * index;
  }

  sliderChange(event) {
    const value = event.value || event;
    this.realVal = this.valToTime(this.val);
    this.timeValue = this.valToDatetime(this.realVal); // 转换成时间组件能显示的时间格式
    this.dealTipAndColor(value);
  }

  dealTipAndColor(value) {
    if (value < 76 && value > 72) {
      this.handleColor = '#A97AF8';
      this.valTips = '18:00:00 Mountable';
    } else {
      this.handleColor = '#b8becc';
      this.valTips = `${this.realVal} Unavaliable`;
    }
  }
  // value与时间换算
  valToTime(val) {
    const time = ((24 * 3600) / 100) * val;
    const min = Math.floor(time % 3600);
    return (
      this.formatTime(Math.floor(time / 3600)) +
      ':' +
      this.formatTime(Math.floor(min / 60)) +
      ':' +
      this.formatTime(time % 60)
    );
  }

  valToDatetime(val) {
    const date = new Date();
    const arr = val.split(':');
    const currentHours = arr[0];
    const currentMinutes = arr[1];
    const currentSeconds = arr[2];
    date.setHours(Number(currentHours));
    date.setMinutes(Number(currentMinutes));
    date.setSeconds(Number(currentSeconds));
    return date;
  }

  formatTime(time) {
    if (time < 10) {
      return '0' + time;
    } else {
      return time;
    }
  }

  // 时分秒改变
  timeChange(realVal) {
    if (!realVal) {
      return;
    }
    const time =
      realVal.getHours() * 3600 +
      realVal.getMinutes() * 60 +
      realVal.getSeconds();
    this.val = (time / (24 * 3600)) * 100;
  }
}
