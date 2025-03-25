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
  SimpleChanges,
  ViewChild
} from '@angular/core';
import {
  CopiesService,
  CopyControllerService,
  getGeneralDatabaseConf,
  isIncompleteOracleCopy,
  isJson,
  SystemApiService,
  TextMapPipe
} from 'app/shared';
import { ModifyRetentionPolicyComponent } from 'app/shared/components';
import { CopyDataDetailComponent } from 'app/shared/components/copy-data-detail/copy-data-detail.component';
import { CopyDataListComponent } from 'app/shared/components/copy-data-list/copy-data-list.component';
import {
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  OperateItems,
  RestoreType,
  RestoreV2Type,
  SYSTEM_TIME
} from 'app/shared/consts';
import {
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import { RestoreService } from 'app/shared/services/restore.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  find,
  first,
  get,
  includes,
  indexOf,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  lastIndexOf,
  mapValues,
  omit,
  range,
  replace,
  set,
  size,
  toString,
  uniqBy
} from 'lodash';
import { finalize } from 'rxjs';

@Component({
  selector: 'aui-copy-data-dbtoday',
  templateUrl: './today.component.html',
  styleUrls: ['./today.component.less'],
  providers: [TextMapPipe]
})
export class TodayComponent implements OnInit {
  @Input() id: string;
  @Input() resType;
  @Input() rowData: any;
  @Input() currentDate;
  _toString = toString;
  _first = first;
  listView = 0;
  copyNum = 0;
  timeZone;
  extTimeZone = SYSTEM_TIME.timeZone;
  standardTimeZone = 'UTC';
  resourceResourceType = DataMap.Resource_Type;
  copyDataBackupType = DataMap.CopyData_Backup_Type;
  columns = [
    {
      key: 'display_timestamp',
      label: this.i18n.get('common_time_stamp_label'),
      width: '180px',
      sort: true
    },
    {
      key: 'status',
      label: this.i18n.get('common_status_label'),
      width: '100px'
    },
    {
      key: 'location',
      label: this.i18n.get('common_location_label')
    },
    {
      key: 'generated_by',
      label: this.i18n.get('common_generated_type_label')
    },
    {
      key: 'backup_type',
      label: this.i18n.get('common_copy_type_label')
    },
    {
      key: 'resource_name',
      label: this.i18n.get('protection_resource_name_label')
    },
    {
      key: 'expiration_time',
      width: '170px',
      label: this.i18n.get('common_expriration_time_label')
    },
    {
      key: 'op',
      width: '110px',
      label: this.i18n.get('common_operation_label')
    }
  ];

  sliderVal = 0;
  sliderLines = [];
  sliderPoints = [];
  HbaseTimeRanges = [];
  sliderHandleColor = '#b8becc';
  sliderValTips;
  archiveLogCopy = false;
  treeData = [];
  copyData;
  archiveLogCopyData = [];
  logData;
  timePickerValue;
  optItems;

  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE * 10;
  orders = ['-display_timestamp'];

  readonly initPointLen = 7;

  // 令时修改，一天可能不止24小时或者小于24小时
  slideDivision = 24;
  slideMax = this.slideDivision * 3600;
  isDayLightTimeBegin = false;
  isDayLightTimeEnd = false;
  repeatHour = 1;
  repeatHourLong = 0;
  dayLightTimeEndLastHourLong = 0;
  lostHour = 2;
  orderTimeMap = {
    summer: 'summer',
    winter: 'winter'
  };
  orderTimeSelected = this.orderTimeMap.summer;
  disabledHour = hour => {
    return false;
  };
  daylinghtLineLeft = '0px';
  daylinghtTextLeft = '0px';

  @ViewChild(CopyDataListComponent, { static: false })
  copyDataListComponent: CopyDataListComponent;

  constructor(
    public dataMapService: DataMapService,
    private i18n: I18NService,
    private copiesApiService: CopiesService,
    private datePipe: DatePipe,
    private textMapPipe: TextMapPipe,
    private restoreService: RestoreService,
    private manualMountService: ManualMountService,
    private drawModalService: DrawModalService,
    private systemApiService: SystemApiService,
    private copyControllerService: CopyControllerService,
    private warningMessageService: WarningMessageService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.archiveLogCopy = [
      this.resourceResourceType.tdsqlInstance.value,
      this.resourceResourceType.tdsqlDistributedInstance.value
    ].includes(this.resType);
  }

  // 夏令时开始找到哪个小时缺失
  findLostHour(timeStr) {
    const hourArr = range(24);
    each(timeStr, str => {
      const hour = new Date(str).getHours();
      if (indexOf(hourArr, hour) === -1) {
        this.lostHour = hour;
        return false;
      }
    });
  }

  // 夏令时结束找到重复的小时
  findRepeatHour(timeStr, timeStamp) {
    each(timeStr, (str: string) => {
      if (indexOf(timeStr, str) !== lastIndexOf(timeStr, str)) {
        this.repeatHour = new Date(str).getHours();
        this.repeatHourLong = timeStamp[lastIndexOf(timeStr, str)] * 1000;
        // 获取当天最后一个小时时间戳
        this.dayLightTimeEndLastHourLong =
          timeStamp[timeStamp.length - 2] * 1000;
        return false;
      }
    });
  }

  setTipLineOffsetLeft() {
    const pointCells = document.getElementsByClassName('lv-slider-cell');
    if (isEmpty(pointCells)) {
      return;
    }
    const targetCell: any =
      pointCells[
        this.isDayLightTimeBegin ? this.lostHour : this.repeatHour + 1
      ];
    this.daylinghtLineLeft = `${targetCell.offsetLeft}px`;
    this.daylinghtTextLeft = `${targetCell.offsetLeft - 30}px`;
  }

  // 获取一天多少小时，适配夏令时
  getSystemTimeLine() {
    this.systemApiService
      .getSystemTimelineUsingGET({
        date: this.datePipe.transform(this.currentDate, 'yyyy-MM-dd')
      })
      .pipe(finalize(() => this.initSlider()))
      .subscribe((res: any) => {
        if (isJson(res)) {
          res = JSON.parse(res);
        }
        const hours = size(res.timeStr);
        this.slideDivision = hours - 1;
        this.slideMax = this.slideDivision * 3600;
        // 夏令时开始，时间拨快1小时
        this.isDayLightTimeBegin = hours < 25;
        // 夏令时结束，时间回拨1小时
        this.isDayLightTimeEnd = hours > 25;
        // 获取重复的小时
        if (this.isDayLightTimeEnd) {
          this.findRepeatHour(res.timeStr, res.timeStamp);
          this.changeTimePicker();
        }
        if (this.isDayLightTimeBegin) {
          this.findLostHour(res.timeStr);
          // 禁用丢失的小时
          this.disabledHour = hour => {
            return hour === this.lostHour;
          };
        }
        // 设置冬夏令时提示位置
        defer(() => this.setTipLineOffsetLeft());
      });
  }

  setSysTime() {
    this.systemApiService
      .getSystemTimeUsingGET({
        akDoException: false
      })
      .subscribe(res => {
        const sysDate = new Date(res.time);
        this.timePickerValue = new Date(sysDate);
        this.timePickerChange(sysDate);
      });
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.currentDate.currentValue) {
      this.timePickerValue = new Date(
        changes.currentDate.currentValue.setHours(0, 0, 0, 0)
      );
      // 改为调接口查询，每次需要重绘
      this.sliderPoints = [];
      this.getSystemTimeLine();
      this.getSliderLines();
      this.queryCopyPoint({ value: 0 });
    }
  }

  changeTimePicker(triggerPicker = true) {
    if (this.orderTimeSelected === this.orderTimeMap.summer) {
      this.disabledHour = hour => {
        return hour > this.repeatHour;
      };
    } else {
      this.disabledHour = hour => {
        return hour < this.repeatHour;
      };
    }
    const timeHour = this.timePickerValue.getHours();
    if (
      (this.orderTimeSelected === this.orderTimeMap.summer &&
        timeHour > this.repeatHour) ||
      (this.orderTimeSelected === this.orderTimeMap.winter &&
        timeHour < this.repeatHour)
    ) {
      this.timePickerValue = new Date(
        this.timePickerValue.setHours(this.repeatHour)
      );
    }
    if (triggerPicker) {
      this.timePickerChange(this.timePickerValue);
    }
  }

  getPointValue(index): number {
    if (this.isDayLightTimeEnd && index * 4 > this.repeatHour) {
      return this.calculatePoint(index) + 3600;
    }
    if (this.isDayLightTimeBegin && index * 4 > this.lostHour) {
      return this.calculatePoint(index) - 3600;
    }
    return this.calculatePoint(index);
  }

  initSlider() {
    each(range(this.initPointLen), index => {
      this.sliderPoints.push({
        value: this.getPointValue(index),
        label: `${this.formatTime(index * 4)}:00:00`,
        pointStyle: {
          'background-color': 'transparent',
          'border-color': 'transparent'
        }
      });
    });
    this.sliderPoints = [...this.sliderPoints];
  }

  calculatePoint(index) {
    return 3600 * 4 * index;
  }

  isReplicateOrArchival(item) {
    return includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.reverseReplication.value,
        DataMap.CopyData_generatedType.cascadedReplication.value,
        DataMap.CopyData_generatedType.cloudArchival.value,
        DataMap.CopyData_generatedType.tapeArchival.value
      ],
      item?.generated_by
    );
  }

  // 复制副本或者归档副本取副本备份时间
  getDisplayTimestamp(item) {
    if (this.isReplicateOrArchival(item)) {
      return item.origin_copy_time_stamp;
    }
    return item.display_timestamp;
  }

  getCopyData() {
    const params = {
      resource_id: this.id
    };
    assign(params, {
      origin_date: this.datePipe.transform(this.currentDate, 'yyyy-MM-dd')
    });
    this.copiesApiService
      .queryResourcesV1CopiesGet({
        pageNo: this.pageIndex,
        pageSize: this.pageSize,
        orders: this.orders,
        conditions: JSON.stringify(params)
      })
      .subscribe(res => {
        const points = res.items.filter(item => {
          return item.backup_type !== DataMap.CopyData_Backup_Type.log.value;
        });
        // 清空数据
        this.sliderPoints.push({});
        this.sliderPoints.splice(this.initPointLen);
        this.copyNum = res.total;
        points.forEach((item: any) => {
          this.sliderPoints.push({
            value: this.calculateCopyPoint(
              this.datePipe.transform(
                new Date(this.getDisplayTimestamp(item)),
                'yyyy/MM/dd HH:mm:ss'
              ),
              false,
              item.timestamp / 1000
            ),
            tip: this.getTip(item),
            handleColor: '#779bfa',
            pointStyle: {
              'border-color': '#779bfa',
              'background-color': '#779bfa'
            },
            data: item
          });
        });
        this.sliderPoints = [...this.sliderPoints];
        this.fetchHbaseBackupLines();
      });
  }

  private getTip(item: any) {
    let dataMapKey = 'CopyData_Backup_Type';
    if (
      item?.resource_sub_type === this.resourceResourceType.HBaseBackupSet.value
    ) {
      // Hbase备份集的增量备份展示为永久增量，修改时html也需要同步修改
      dataMapKey = 'specialBackUpType';
    }
    return this.textMapPipe.transform(
      !isUndefined(item.source_copy_type)
        ? item.source_copy_type
        : item.backup_type,
      dataMapKey
    );
  }

  getSliderLines() {
    this.getHbaseBackupLines();
  }

  fetchHbaseBackupLines() {
    const newSliderLines = [];
    this.sliderLines.forEach(item => {
      const oldStartTime = item.value[0];
      const oldEndTime = item.value[1];
      if (
        !size(newSliderLines) ||
        isUndefined(
          find(
            newSliderLines,
            slider =>
              oldEndTime > slider.value[0] && oldStartTime < slider.value[1]
          )
        ) ||
        this.archiveLogCopy
      ) {
        // 时间段与当前时间段轴上某时间段没有重叠时（重叠包括两段时间首尾相接）
        newSliderLines.push(item);
      }
      newSliderLines.filter(item => {
        const newStartTime = item.value[0];
        const newEndTime = item.value[1];
        if (
          oldStartTime >= newStartTime &&
          oldEndTime >= newEndTime &&
          oldStartTime < newEndTime
        ) {
          // 时间段头部与当前时间段轴上某时间段尾部重叠（包括两段时间完成重叠和首尾相接）
          item.value = [newStartTime, oldEndTime];
        }
        if (
          oldStartTime <= newStartTime &&
          oldEndTime <= newEndTime &&
          oldEndTime > newStartTime
        ) {
          // 时间段头部与当前时间段轴上某时间段头部重叠（包括两段时间完成重叠和首尾相接）
          item.value = [oldStartTime, newEndTime];
        }
        if (oldStartTime > newStartTime && oldEndTime < newEndTime) {
          // 时间段被当前时间段轴上某时间段包含（包括两段时间完成重叠）
          item.value = [newStartTime, newEndTime];
        }
      });
    });
    newSliderLines.filter(item => {
      let startTime = item.value[0];
      if (startTime <= 0) {
        return;
      }
      const endTime = item.value[1];

      // 获取该时间段之前的数据副本
      const pointsBefor = this.sliderPoints.filter(item => {
        return (
          item.value < endTime &&
          item.data?.generated_by ===
            DataMap.CopyData_generatedType.backup.value &&
          includes(
            [
              DataMap.CopyData_Backup_Type.full.value,
              DataMap.CopyData_Backup_Type.incremental.value,
              DataMap.CopyData_Backup_Type.diff.value
            ],
            item.data?.backup_type
          )
        );
      });
      pointsBefor.sort((point1, point2) => point1.value - point2.value);

      // 找到该时间段内最早生成的数据副本，将该数据副本的生成时间作为该时间段的开始时间，若未找到数据副本，则不做裁剪。
      // OceanBase、TiDB、lightGauss、oracle 除了首次日志备份 两次日志备份之间不需要裁剪
      if (
        !includes(
          [
            DataMap.Resource_Type.Dameng_singleNode.value,
            DataMap.Resource_Type.generalDatabase.value,
            DataMap.Resource_Type.SQLServerDatabase.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.MySQLClusterInstance.value,
            DataMap.Resource_Type.MySQLDatabase.value,
            DataMap.Resource_Type.goldendbInstance.value,
            DataMap.Resource_Type.PostgreSQLInstance.value,
            DataMap.Resource_Type.PostgreSQLClusterInstance.value,
            DataMap.Resource_Type.OceanBaseCluster.value,
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.lightCloudGaussdbInstance.value,
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value,
            DataMap.Resource_Type.oraclePDB.value,
            DataMap.Resource_Type.tdsqlInstance.value,
            DataMap.Resource_Type.tdsqlDistributedInstance.value,
            DataMap.Resource_Type.ExchangeSingle.value,
            DataMap.Resource_Type.ExchangeGroup.value,
            DataMap.Resource_Type.ExchangeDataBase.value,
            DataMap.Resource_Type.saphanaDatabase.value,
            DataMap.Resource_Type.AntDBClusterInstance.value,
            DataMap.Resource_Type.AntDBInstance.value,
            DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
            DataMap.Resource_Type.MongodbSingleInstance.value,
            DataMap.Resource_Type.dbTwoDatabase.value,
            DataMap.Resource_Type.OpenGauss_instance.value,
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.informixClusterInstance.value,
            DataMap.Resource_Type.saponoracleDatabase.value,
            DataMap.Resource_Type.gaussdbTSingle.value,
            DataMap.Resource_Type.GaussDB_T.value,
            DataMap.Resource_Type.KingBaseInstance.value,
            DataMap.Resource_Type.KingBaseClusterInstance.value
          ],
          this.rowData.sub_type
        )
      ) {
        for (let i = 0; i < size(pointsBefor); i++) {
          const copyTime = pointsBefor[i];
          if (copyTime.value >= startTime) {
            startTime = copyTime.value;
            break;
          }
        }
      }
      item.value = [startTime, endTime];
    });
    this.sliderLines = [...newSliderLines];
  }

  getHbaseBackupLines(recordsTemp?, startPage?) {
    this.sliderLines = [];
    const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
    const startTime =
      this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
    const endTime =
      this.appUtilsService.toSystemTimeLong(`${date} 23:59:59`) / 1000;
    const params = {
      startTime,
      endTime,
      resourceId: this.rowData.uuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX
    };
    this.copyControllerService
      .ListAvailableTimeRanges(params)
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
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) ||
          !res.totalCount
        ) {
          const timeRanges = [];

          each(recordsTemp, recode => {
            const timeRange = JSON.parse(
              replace(get(recode, 'timeRange', '[]'), /\'/g, '"')
            );

            if (!!size(timeRange)) {
              each(timeRange, range => {
                const data = {
                  ...recode,
                  startTime: range[0],
                  endTime: range[1]
                };
                const lineStartTime = Date.parse(
                  this.datePipe.transform(
                    parseInt(data.startTime, 10) * 1000,
                    'yyyy/MM/dd HH:mm:ss'
                  )
                );
                const lineEndTime = Date.parse(
                  this.datePipe.transform(
                    parseInt(data.endTime, 10) * 1000,
                    'yyyy/MM/dd HH:mm:ss'
                  )
                );
                if (
                  !(
                    lineEndTime / 1000 < startTime ||
                    lineStartTime / 1000 > endTime
                  )
                ) {
                  timeRanges.push(data);
                }
              });
            } else {
              timeRanges.push(recode);
            }
          });

          each(timeRanges, timeRange => {
            const lineStartTime = Date.parse(
              this.datePipe.transform(
                parseInt(timeRange.startTime, 10) * 1000,
                'yyyy/MM/dd HH:mm:ss'
              )
            );
            const lineEndTime = Date.parse(
              this.datePipe.transform(
                parseInt(timeRange.endTime, 10) * 1000,
                'yyyy/MM/dd HH:mm:ss'
              )
            );
            this.sliderLines.push({
              value: [
                this.calculateCopyPoint(
                  lineStartTime / 1000 < startTime
                    ? startTime * 1000
                    : lineStartTime,
                  true
                ),
                this.calculateCopyPoint(
                  lineEndTime / 1000 >= endTime ? endTime * 1000 : lineEndTime,
                  true
                )
              ],
              data: timeRange,
              tip: this.i18n.get('common_log_label'),
              handleColor: '#779bfa',
              style: {
                'background-color': '#779bfa',
                'z-index': 4
              }
            });
          });
          this.HbaseTimeRanges = timeRanges;
          this.sliderLines = [...this.sliderLines];
          this.sliderChange({ value: 0 });
          this.getCopyData();
          return;
        }
        this.getHbaseBackupLines(recordsTemp, startPage);
      });
  }

  changeListView(data) {
    if (!data) {
      this.archiveLogCopyData = [];
      this.getSliderLines();
      this.queryCopyPoint({ value: 0 });
    }
  }

  refreshCopy(index) {
    if (!index) {
      this.changeListView(index);
    } else {
      this.copyDataListComponent.getCopyData();
    }
  }

  calculateCopyPoint(time, isLong = false, copyTimeLong?: number) {
    if (!time) {
      return;
    }
    const timeDate = isLong
      ? new Date(
          this.datePipe.transform(time, 'yyyy/MM/dd HH:mm:ss', this.extTimeZone)
        )
      : new Date(time);

    let hour = timeDate.getHours();
    // 夏令时开始后面的时间需要减一小时
    if (!isLong && this.isDayLightTimeBegin && hour > this.lostHour) {
      hour--;
    }
    // 2个相同的时间需要通过时间戳来判断夏令时还是冬令时
    if (
      !isLong &&
      this.isDayLightTimeEnd &&
      hour === this.repeatHour &&
      copyTimeLong >= this.repeatHourLong
    ) {
      hour++;
    }
    // 时间戳的话需要处理的是超过当天的1小时
    if (
      isLong &&
      this.isDayLightTimeEnd &&
      time >= this.dayLightTimeEndLastHourLong
    ) {
      hour++;
    }
    return hour * 60 * 60 + timeDate.getMinutes() * 60 + timeDate.getSeconds();
  }

  sliderChange(data, isHover = false) {
    const timeVal = this.valToTime(data.value);
    if (!isHover) {
      this.timePickerValue = this.valToDatetime(timeVal);
      this.sliderHandleColor = '#b8becc';
      this.timePickerChange(this.timePickerValue);
    }
    this.sliderValTips = `${timeVal}`;
    if (this.archiveLogCopy) {
      this.initArchiveLogDataRange(data.value, isHover);
    } else {
      this.isInLogDataRange(data.value, isHover);
    }
    this.isCopyDataPointInSlider(data.value, isHover);
    return this.sliderValTips;
  }

  // 查询就近copy点进行吸附显示树表
  queryCopyPoint(data) {
    const sliderPointLen = this.sliderPoints.length;
    // 初始刻度7个点
    if (sliderPointLen <= this.initPointLen) {
      return;
    }

    // 用户只能以百分之一进行拖拽，所以当绝对值小于1时吸附最近的copy点
    let copyPoint;
    for (let i = this.initPointLen; i < sliderPointLen; i++) {
      if (Math.abs(this.sliderPoints[i].value - data.value) <= 1 * 864) {
        if (
          !copyPoint ||
          Math.abs(this.sliderPoints[i].value - data.value) <
            Math.abs(copyPoint.value - data.value)
        ) {
          copyPoint = this.sliderPoints[i];
        }
      }
    }

    if (!copyPoint) {
      return;
    }

    this.sliderVal = copyPoint.value;
    data.value = copyPoint.value;
    this.sliderChange(data);
  }

  valToTime(val) {
    const time = val;
    const min = Math.floor(time % 3600);
    let hour = Math.floor(time / 3600);

    // 夏令时开始
    if (this.isDayLightTimeBegin && hour >= this.lostHour) {
      hour++;
    }

    // 夏令时结束
    if (this.isDayLightTimeEnd) {
      if (hour > this.repeatHour) {
        this.orderTimeSelected = this.orderTimeMap.winter;
        hour--;
      } else {
        this.orderTimeSelected = this.orderTimeMap.summer;
      }
      this.changeTimePicker(false);
    }

    return (
      this.formatTime(hour) +
      ':' +
      this.formatTime(Math.floor(min / 60)) +
      ':' +
      this.formatTime(time % 60)
    );
  }

  formatTime(time) {
    if (time < 10) {
      return '0' + time;
    } else {
      return time;
    }
  }

  valToDatetime(val) {
    const date = cloneDeep(this.currentDate);
    const arr = val.split(':');
    const currentHours = arr[0];
    const currentMinutes = arr[1];
    const currentSeconds = arr[2];
    date.setHours(Number(currentHours));
    date.setMinutes(Number(currentMinutes));
    date.setSeconds(Number(currentSeconds));
    return date;
  }

  timePickerChange(data) {
    if (!data) {
      return;
    }
    let time =
      data.getHours() * 3600 + data.getMinutes() * 60 + data.getSeconds();
    // 夏令时开始
    if (this.isDayLightTimeBegin && data.getHours() > this.lostHour) {
      time -= 3600;
    }
    // 夏令时结束
    if (
      this.isDayLightTimeEnd &&
      data.getHours() >= this.repeatHour &&
      this.orderTimeSelected === this.orderTimeMap.winter
    ) {
      time += 3600;
    }
    this.sliderVal = time;
    this.copyData = this.isCopyDataPoint(data);
    if (this.archiveLogCopy) {
      this.initArchiveLogDataRange(this.sliderVal);
    } else {
      this.isInLogDataRange(this.sliderVal);
    }
  }

  isInLogDataRange(value, isHover = false) {
    let flag = true;
    this.sliderLines.forEach(item => {
      if (value >= item.value[0] && value <= item.value[1]) {
        this.sliderValTips = `${this.valToTime(value)} ${item.tip}`;
        if (!isHover) {
          this.sliderHandleColor = item.handleColor;
          if (!this.logData || this.logData?.uuid !== item.data?.copyId) {
            this.copiesApiService
              .queryResourcesV1CopiesGet({
                pageNo: this.pageIndex,
                pageSize: this.pageSize,
                conditions: JSON.stringify({
                  uuid: item.data?.copyId
                })
              })
              .subscribe(res => {
                this.logData = { ...first(res.items) };
              });
          }
        }
        flag = false;
      }
    });
    if (flag && !isHover) {
      this.logData = null;
    }
  }

  initArchiveLogDataRange(value, isHover = false) {
    let flag = true;
    this.sliderLines.forEach(item => {
      if (value >= item.value[0] && value <= item.value[1]) {
        this.sliderValTips = `${this.valToTime(value)} ${item.tip}`;
        this.formatArchiveCopyData(isHover, item);
        flag = false;
      }
    });
    this.archiveLogCopyData = this.archiveLogCopyData.filter(item => {
      const copy = find(this.HbaseTimeRanges, { copyId: item.uuid });
      return (
        copy &&
        value >= this.calculateCopyPoint(copy.startTime * 1e3) &&
        value <= this.calculateCopyPoint(copy.endTime * 1e3)
      );
    });
    if (flag && !isHover) {
      this.archiveLogCopyData = [];
    }
  }

  private formatArchiveCopyData(isHover: boolean, item) {
    if (isHover) {
      return;
    }
    this.sliderHandleColor = item.handleColor;
    if (
      isEmpty(this.archiveLogCopyData) ||
      !find(this.archiveLogCopyData, { uuid: item.data?.copyId })
    ) {
      this.copiesApiService
        .queryResourcesV1CopiesGet({
          pageNo: this.pageIndex,
          pageSize: this.pageSize,
          conditions: JSON.stringify({
            uuid: item.data?.copyId
          })
        })
        .subscribe(res => {
          this.logData = { ...first(res.items) };
          this.archiveLogCopyData.push(this.logData);
          this.archiveLogCopyData = [
            ...uniqBy(this.archiveLogCopyData, 'uuid')
          ];
        });
    }
  }

  isCopyDataPointInSlider(value, isHover = false) {
    this.sliderPoints.forEach(item => {
      if (value === item.value && item.tip) {
        this.sliderValTips = `${this.valToTime(value)} ${item.tip}`;
        if (!isHover) {
          this.sliderHandleColor = item.handleColor;
        }
      }
    });
  }

  // 夏令时场景找副本，存在2个相同的时间
  dayLightTime(copyTimeLong: number): boolean {
    if (!this.isDayLightTimeEnd) {
      return true;
    }
    return this.orderTimeSelected === this.orderTimeMap.summer
      ? copyTimeLong < this.repeatHourLong
      : copyTimeLong >= this.repeatHourLong;
  }

  isCopyDataPoint(data: Date) {
    const sliderPointLen = this.sliderPoints.length;
    // 初始刻度7个点
    if (sliderPointLen <= this.initPointLen) {
      return null;
    }

    for (let i = this.initPointLen; i < sliderPointLen; i++) {
      const copyTime = new Date(
        this.datePipe.transform(
          this.getDisplayTimestamp(this.sliderPoints[i].data),
          'yyyy/MM/dd HH:mm:ss'
        )
      );
      if (
        data.getHours() === copyTime.getHours() &&
        data.getMinutes() === copyTime.getMinutes() &&
        data.getSeconds() === copyTime.getSeconds() &&
        this.dayLightTime(this.sliderPoints[i].data?.timestamp / 1000)
      ) {
        return this.resType !== this.resourceResourceType.oracle.value
          ? this.getHbaseCopyData(this.sliderPoints[i].data)
          : this.sliderPoints[i].data;
      }
    }

    return null;
  }

  hideLiveMountOpt() {
    if (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.tdsqlInstance.value,
          DataMap.Resource_Type.MySQL.value
        ],
        this.resType
      )
    ) {
      return false;
    }
    return true;
  }

  hideOracleWinodwsOpt(data?) {
    return (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        this.rowData.sub_type
      ) &&
      (this.rowData.environment?.osType === DataMap.Os_Type.windows.value ||
        !!data?.storage_snapshot_flag)
    );
  }

  optsCallback = data => {
    if (this.logData && !this.copyData) {
      return this.getLogOptsItems(data);
    } else {
      return this.getOptsItems(data);
    }
  };

  getLogOptsItems(data) {
    return !(
      this.rowData.version && this.rowData.version.substring(0, 2) === '11'
    ) && this.resType !== this.resourceResourceType.HBaseBackupSet.value
      ? [
          {
            id: 'restore',
            label: this.getCommonRestoreLabel(this.resType),
            disabled: data.status !== DataMap.copydata_validStatus.normal.value,
            hidden:
              getGeneralDatabaseConf(
                data,
                RestoreV2Type.CommonRestore,
                this.dataMapService
              ) ||
              (includes(
                [DataMap.Resource_Type.saphanaDatabase.value],
                data.resource_sub_type
              ) &&
                includes(
                  [
                    DataMap.CopyData_Backup_Type.incremental.value,
                    DataMap.CopyData_Backup_Type.diff.value
                  ],
                  data.backup_type
                )),
            permission: OperateItems.RestoreCopy,
            onClick: () => {
              includes(
                [
                  this.resourceResourceType.OceanBaseCluster.value,
                  this.resourceResourceType.tdsqlInstance.value,
                  this.resourceResourceType.tdsqlDistributedInstance.value,
                  this.resourceResourceType.OpenGauss_instance.value,
                  this.resourceResourceType.oraclePDB.value
                ],
                this.resType
              )
                ? this.restore(data)
                : this.restore();
            }
          },
          {
            id: 'instantRestore',
            label: this.i18n.get('common_live_restore_job_label'),
            permission: OperateItems.InstanceRecovery,
            hidden:
              this.appUtilsService.isDistributed ||
              includes(
                [
                  DataMap.Resource_Type.Dameng_singleNode.value,
                  DataMap.Resource_Type.GaussDB_T.value,
                  DataMap.Resource_Type.gaussdbTSingle.value,
                  DataMap.Resource_Type.MySQL.value,
                  DataMap.Resource_Type.PostgreSQLInstance.value,
                  DataMap.Resource_Type.PostgreSQLClusterInstance.value,
                  DataMap.Resource_Type.dbTwoDatabase.value,
                  DataMap.Resource_Type.dbTwoTableSet.value,
                  DataMap.Resource_Type.KingBaseInstance.value,
                  DataMap.Resource_Type.KingBaseClusterInstance.value,
                  DataMap.Resource_Type.SQLServerClusterInstance.value,
                  DataMap.Resource_Type.SQLServerInstance.value,
                  DataMap.Resource_Type.SQLServerGroup.value,
                  DataMap.Resource_Type.SQLServerDatabase.value,
                  DataMap.Resource_Type.generalDatabase.value,
                  DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
                  DataMap.Resource_Type.informixInstance.value,
                  DataMap.Resource_Type.informixClusterInstance.value,
                  DataMap.Resource_Type.lightCloudGaussdbInstance.value,
                  DataMap.Resource_Type.tdsqlInstance.value,
                  DataMap.Resource_Type.tdsqlDistributedInstance.value,
                  DataMap.Resource_Type.tidbCluster.value,
                  DataMap.Resource_Type.OceanBaseCluster.value,
                  DataMap.Resource_Type.ExchangeSingle.value,
                  DataMap.Resource_Type.ExchangeGroup.value,
                  DataMap.Resource_Type.ExchangeDataBase.value,
                  DataMap.Resource_Type.goldendbInstance.value,
                  DataMap.Resource_Type.saphanaDatabase.value,
                  DataMap.Resource_Type.OpenGauss_instance.value,
                  DataMap.Resource_Type.MongodbSingleInstance.value,
                  DataMap.Resource_Type.MongodbClusterInstance.value,
                  DataMap.Resource_Type.AntDBInstance.value,
                  DataMap.Resource_Type.AntDBClusterInstance.value,
                  DataMap.Resource_Type.oraclePDB.value
                ],
                this.resType
              ) ||
              (includes(
                [
                  DataMap.Resource_Type.oracle.value,
                  DataMap.Resource_Type.oracleCluster.value
                ],
                this.resType
              ) &&
                data.generated_by ===
                  DataMap.CopyData_generatedType.liveMount.value) ||
              this.hideOracleWinodwsOpt(data),
            onClick: () => {
              this.instantRestore();
            }
          }
        ]
      : [
          {
            id: 'restore',
            label: this.i18n.get('common_restore_label'),
            hidden:
              getGeneralDatabaseConf(
                data,
                RestoreV2Type.CommonRestore,
                this.dataMapService
              ) ||
              (includes(
                [DataMap.Resource_Type.saphanaDatabase.value],
                data.resource_sub_type
              ) &&
                includes(
                  [
                    DataMap.CopyData_Backup_Type.incremental.value,
                    DataMap.CopyData_Backup_Type.diff.value
                  ],
                  data.backup_type
                )),
            disabled: data.status !== DataMap.copydata_validStatus.normal.value,
            permission: OperateItems.RestoreCopy,
            onClick: () => {
              this.restore();
            }
          },
          {
            id: 'fileLevelRestore',
            disabled: data.status !== DataMap.copydata_validStatus.normal.value,
            label: this.i18n.get('protection_table_level_restore_label'),
            hidden:
              !includes(
                [this.resourceResourceType.HBaseBackupSet.value],
                this.resType
              ) ||
              includes(
                [
                  DataMap.CopyData_generatedType.tapeArchival.value,
                  DataMap.CopyData_generatedType.import.value,
                  DataMap.CopyData_generatedType.Imported.value,
                  DataMap.CopyData_generatedType.cloudArchival.value
                ],
                data.generated_by
              ),
            permission: OperateItems.FileLevelRestore,
            onClick: () =>
              this.restoreService.fileLevelRestore({
                header: this.i18n.get('protection_table_level_restore_label'),
                childResType: this.rowData.sub_type,
                copyData:
                  this.rowData.sub_type ===
                  DataMap.Resource_Type.HBaseBackupSet.value
                    ? assign(data, {
                        environment_uuid: JSON.parse(data.resource_properties)
                          ?.environment_uuid
                      })
                    : data,
                restoreType: RestoreType.FileRestore,
                onOk: () => this.getSliderLines()
              })
          }
        ];
  }

  getOptsItems(data) {
    const features = data.features
      .toString(2)
      .split('')
      .reverse();
    const properties = isJson(data?.properties)
      ? JSON.parse(data.properties)
      : data.properties;
    return [
      {
        id: 'restore',
        disabled:
          data.status !== DataMap.copydata_validStatus.normal.value ||
          isIncompleteOracleCopy(data, properties, data?.resource_sub_type),
        label: this.getCommonRestoreLabel(this.resType),
        hidden:
          getGeneralDatabaseConf(
            data,
            RestoreV2Type.CommonRestore,
            this.dataMapService
          ) ||
          (data.features ? features[1] !== '1' : false) ||
          (includes(
            [DataMap.Resource_Type.saphanaDatabase.value],
            data.resource_sub_type
          ) &&
            includes(
              [
                DataMap.CopyData_Backup_Type.incremental.value,
                DataMap.CopyData_Backup_Type.diff.value
              ],
              data.backup_type
            )),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.restore(data);
        }
      },
      {
        id: 'fileLevelRestore',
        disabled: data.status !== DataMap.copydata_validStatus.normal.value,
        label: this.i18n.get('protection_table_level_restore_label'),
        hidden:
          !includes(
            [
              this.resourceResourceType.HBaseBackupSet.value,
              this.resourceResourceType.OceanBaseCluster.value
            ],
            this.resType
          ) ||
          includes(
            [
              DataMap.CopyData_generatedType.tapeArchival.value,
              DataMap.CopyData_generatedType.import.value,
              DataMap.CopyData_generatedType.Imported.value,
              DataMap.CopyData_generatedType.cloudArchival.value
            ],
            data.generated_by
          ),
        permission: OperateItems.FileLevelRestore,
        onClick: () =>
          this.restoreService.fileLevelRestore({
            header: this.i18n.get('protection_table_level_restore_label'),
            childResType: this.rowData.sub_type,
            copyData:
              this.rowData.sub_type ===
              DataMap.Resource_Type.HBaseBackupSet.value
                ? assign(data, {
                    environment_uuid: JSON.parse(data.resource_properties)
                      ?.environment_uuid
                  })
                : data,
            restoreType: RestoreType.FileRestore,
            onOk: () => this.getSliderLines()
          })
      },
      {
        id: 'instantRestore',
        disabled:
          data.status !== DataMap.copydata_validStatus.normal.value ||
          isIncompleteOracleCopy(data, properties, data?.resource_sub_type),
        label: this.i18n.get('common_live_restore_job_label'),
        hidden:
          this.appUtilsService.isDistributed ||
          (data.features ? features[2] !== '1' : false) ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value ||
          this.resType !== this.resourceResourceType.oracle.value ||
          includes(
            [
              DataMap.Resource_Type.MySQL.value,
              DataMap.Resource_Type.oraclePDB.value,
              DataMap.Resource_Type.AntDBInstance.value,
              DataMap.Resource_Type.AntDBClusterInstance.value
            ],
            this.resType
          ) ||
          (includes(
            [
              DataMap.Resource_Type.oracle.value,
              DataMap.Resource_Type.oracleCluster.value
            ],
            this.resType
          ) &&
            ((this.rowData.version &&
              this.rowData.version.substring(0, 2) === '11') ||
              data.generated_by ===
                DataMap.CopyData_generatedType.liveMount.value)) ||
          this.hideOracleWinodwsOpt(data),
        permission: OperateItems.InstanceRecovery,
        onClick: () => {
          this.instantRestore(data);
        }
      },
      {
        id: 'mount',
        label: this.i18n.get('common_live_mount_label'),
        disabled:
          DataMap.copydata_validStatus.normal.value !== data.status ||
          (data.generated_by === DataMap.CopyData_generatedType.liveMount.value
            ? data.generation > DataMap.CopyData_Generation.two.value
            : data.generation >= DataMap.CopyData_Generation.two.value) ||
          isIncompleteOracleCopy(data, properties, data?.resource_sub_type),
        hidden:
          (data.features ? features[3] !== '1' : false) ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value ||
          this.hideLiveMountOpt() ||
          this.hideOracleWinodwsOpt(data),
        permission: OperateItems.MountingCopy,
        onClick: () => {
          this.mount(data);
        }
      },
      {
        id: 'copyVerify',
        label: this.i18n.get('common_copies_verification_label'),
        disabled: data.status !== DataMap.copydata_validStatus.normal.value,
        hidden:
          !includes(
            [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value
            ],
            data.resource_sub_type
          ) ||
          (includes(
            [
              DataMap.Resource_Type.dbTwoDatabase.value,
              DataMap.Resource_Type.dbTwoTableSet.value
            ],
            data.resource_sub_type
          ) &&
            (!includes(
              [DataMap.CopyData_generatedType.backup.value],
              data.generated_by
            ) ||
              !includes(
                [
                  DataMap.CopyData_Backup_Type.full.value,
                  DataMap.CopyData_Backup_Type.incremental.value,
                  DataMap.CopyData_Backup_Type.diff.value
                ],
                data.backup_type
              ))),
        permission: OperateItems.RestoreCopy,
        onClick: () => {
          this.copyControllerService
            .ExecuteCopyVerifyTask({
              copyId: data.uuid,
              copyVerifyRequest: {
                agents: ''
              }
            })
            .subscribe(() => this.getCopyData());
        }
      },
      {
        id: 'modifyRetentionPolicy',
        disabled: data.status !== DataMap.copydata_validStatus.normal.value,
        label: this.i18n.get('common_modify_retention_policy_label'),
        hidden:
          data.generated_by ===
          DataMap.CopyData_generatedType.tapeArchival.value,
        permission: OperateItems.ModifyingCopyRetentionPolicy,
        onClick: () => {
          this.modifyRetention(data);
        }
      },
      {
        id: 'delete',
        permission: OperateItems.DeletingCopy,
        label: this.i18n.get('common_delete_label'),
        disabled: !includes(
          [
            DataMap.copydata_validStatus.normal.value,
            DataMap.copydata_validStatus.invalid.value,
            DataMap.copydata_validStatus.deleteFailed.value
          ],
          data.status
        ),
        hidden:
          data.backup_type === DataMap.CopyData_Backup_Type.log.value ||
          data.generated_by ===
            DataMap.CopyData_generatedType.tapeArchival.value ||
          (data.resource_sub_type ===
            DataMap.Resource_Type.lightCloudGaussdbInstance.value &&
            data.backup_type ===
              DataMap.CopyData_Backup_Type.incremental.value),
        onClick: () => {
          this.deleteCopy(data);
        }
      }
    ];
  }

  getTodayTime(data?) {
    const selectTimeStamp = (cloneDeep(this.currentDate) as Date).setHours(
      (data ? (data as Date) : (this.timePickerValue as Date)).getHours(),
      (data ? (data as Date) : (this.timePickerValue as Date)).getMinutes(),
      (data ? (data as Date) : (this.timePickerValue as Date)).getSeconds(),
      0
    );
    const timeOffset =
      selectTimeStamp -
      Date.parse(
        this.datePipe.transform(
          selectTimeStamp,
          'yyyy/MM/dd HH:mm:ss',
          this.timeZone
        )
      );
    return (
      this.appUtilsService.toSystemTimeLong(selectTimeStamp + timeOffset) / 1000
    );
  }

  restore(data?) {
    this.restoreService.restore({
      childResType: this.rowData.sub_type,
      copyData: includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value,
          DataMap.Resource_Type.SQLServerDatabase.value
        ],
        this.resType
      )
        ? this.getSQLServerData(data)
        : [
            this.resourceResourceType.PostgreSQLInstance.value,
            this.resourceResourceType.PostgreSQLClusterInstance.value,
            this.resourceResourceType.KingBaseInstance.value,
            this.resourceResourceType.KingBaseClusterInstance.value,
            this.resourceResourceType.ExchangeSingle.value,
            this.resourceResourceType.ExchangeGroup.value,
            this.resourceResourceType.ExchangeDataBase.value,
            this.resourceResourceType.AntDBInstance.value,
            this.resourceResourceType.AntDBClusterInstance.value
          ].includes(this.resType)
        ? this.getPostgreData(data)
        : [
            this.resourceResourceType.OceanBaseCluster.value,
            this.resourceResourceType.tdsqlInstance.value,
            this.resourceResourceType.OpenGauss_instance.value,
            this.resourceResourceType.tdsqlDistributedInstance.value,
            this.resourceResourceType.oraclePDB.value
          ].includes(this.resType)
        ? this.getCopyDataTimeStamp(data)
        : this.resType !== this.resourceResourceType.oracle.value
        ? this.getHbaseCopyData(data)
        : this.getOracleCopyData(data),
      isMessageBox: !includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        this.rowData.sub_type
      ),
      restoreType: RestoreType.CommonRestore,
      onOk: () => {
        this.getSliderLines();
      }
    });
  }

  getCopyDataTimeStamp(data) {
    // 这函数只会将选中的时间点作为timestamp往下传递
    // 其余的副本信息都来自接口查询
    assign(data, {
      restoreTimeStamp: this.getTodayTime().toString()
    });
    return data;
  }

  getOracleCopyData(data?) {
    const oracleData = {
      ...data,
      restoreTimeStamp: data ? '' : this.getTodayTime().toString(),
      dbUuid: this.rowData.uuid,
      resource_type: this.rowData.sub_type,
      resource_sub_type: this.rowData.sub_type,
      environment_os_type:
        this.rowData.environment_os_type || this.rowData.environment?.osType,
      osType:
        this.rowData.environment_os_type || this.rowData.environment?.osType,
      environment_uuid:
        this.rowData.environment_uuid ||
        this.rowData.environment?.uuid ||
        this.rowData.rootUuid,
      parent_uuid: this.rowData.parent_uuid || this.rowData.parentUuid,
      path: this.rowData.path,
      name: this.rowData.name,
      resource_id: this.rowData.uuid,
      resource_properties: JSON.stringify(omit(this.rowData, 'globalSearch')),
      storage_snapshot_flag: get(
        data || this.logData,
        'storage_snapshot_flag',
        false
      ),
      generated_by: get(data || this.logData, 'generated_by'),
      is_replicated: get(data || this.logData, 'is_replicated', false),
      resource_status: get(
        data || this.logData,
        'resource_status',
        DataMap.Resource_Status.exist.value
      ),
      properties: JSON.stringify({
        oracle_metadata: {
          ORACLEPARAM: {
            is_cluster:
              this.rowData.subType ===
              DataMap.Resource_Type.oracleCluster.value,
            db_name: this.rowData.name,
            version: this.rowData.version
          }
        },
        // 日志副本点击恢复时data为空，恢复组件中需要手动查询最近的数据副本
        disk_infos: data
          ? get(JSON.parse(data.properties || '{}'), 'disk_infos', {})
          : {},
        new_loc_restore: JSON.parse(
          get(data || this.logData, 'properties', '{}')
        )?.new_loc_restore
      })
    };
    if (!data) {
      const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
      const startTime =
        this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
      const timeRange = find(this.HbaseTimeRanges, item => {
        return (
          item.endTime >= this.sliderVal + startTime &&
          item.startTime <= this.sliderVal + startTime
        );
      });
      assign(oracleData, {
        uuid: timeRange.copyId,
        backup_type: DataMap.CopyData_Backup_Type.log.value
      });
    }
    return oracleData;
  }

  getSQLServerData(data?) {
    const resource_properties = {
      environment_uuid: this.rowData.rootUuid,
      root_uuid: this.rowData.rootUuid
    };
    if (
      includes(
        [
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value
        ],
        this.rowData.subType
      )
    ) {
      assign(resource_properties, {
        environment_is_cluster:
          this.rowData.subType === DataMap.Resource_Type.SQLServerInstance.value
            ? 'False'
            : 'True'
      });
    } else if (
      this.rowData.subType === DataMap.Resource_Type.SQLServerDatabase.value
    ) {
      assign(resource_properties, {
        environment_is_cluster: this.rowData.environment.cluster
          ? 'True'
          : 'False'
      });
    }

    const sqlServerCopyData = {
      ...data,
      restoreTimeStamp: data ? '' : this.getTodayTime().toString(),
      resource_type: this.rowData.sub_type,
      resource_sub_type: this.rowData.sub_type,
      environment_os_type: this.rowData.environment_os_type,
      environment_uuid:
        this.rowData.environment_uuid || this.rowData.environment?.uuid,
      parent_uuid: this.rowData.parent_uuid,
      resource_properties: JSON.stringify(resource_properties),
      path: this.rowData.path,
      name: this.rowData.name,
      resource_id: this.rowData.uuid
    };
    if (!data) {
      const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
      const startTime =
        this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
      const timeRange = find(this.HbaseTimeRanges, item => {
        return (
          item.endTime >= this.sliderVal + startTime &&
          item.startTime <= this.sliderVal + startTime
        );
      });
      assign(sqlServerCopyData, {
        ...this.logData,
        uuid: timeRange.copyId,
        backup_type: DataMap.CopyData_Backup_Type.log.value
      });
    }
    return sqlServerCopyData;
  }

  getHbaseCopyData(data?) {
    const hbaseCopyData = {
      ...data,
      restoreTimeStamp: data ? '' : this.getTodayTime().toString(),
      resource_type: this.rowData.sub_type,
      resource_sub_type: this.rowData.sub_type,
      environment_os_type: this.rowData.environment_os_type,
      environment_uuid:
        this.rowData.environment_uuid || this.rowData.environment?.uuid,
      parent_uuid: this.rowData.parent_uuid,
      resource_properties: JSON.stringify(omit(this.rowData, 'globalSearch')),
      path: this.rowData.path,
      name: this.rowData.name,
      resource_id: this.rowData.uuid
    };
    if (!data && this.resType !== this.resourceResourceType.oracle.value) {
      const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
      const startTime =
        this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
      const timeRange = find(this.HbaseTimeRanges, item => {
        return (
          item.endTime >= this.sliderVal + startTime &&
          item.startTime <= this.sliderVal + startTime
        );
      });
      assign(hbaseCopyData, {
        uuid: timeRange.copyId,
        backup_type: DataMap.CopyData_Backup_Type.log.value
      });
    }

    if (
      !data &&
      this.rowData.sub_type === DataMap.Resource_Type.MySQLDatabase.value
    ) {
      assign(hbaseCopyData, {
        resource_name: this.rowData.name
      });
    }

    if (
      includes(
        [
          DataMap.Resource_Type.GaussDB_T.value,
          DataMap.Resource_Type.gaussdbTSingle.value
        ],
        this.rowData.sub_type
      )
    ) {
      set(
        hbaseCopyData,
        'resource_properties',
        get(
          !!data ? data : this.logData,
          'resource_properties',
          hbaseCopyData.resource_properties
        )
      );
    }
    if (
      includes(
        [
          DataMap.Resource_Type.dbTwoDatabase.value,
          DataMap.Resource_Type.DB2.value
        ],
        this.rowData.sub_type
      )
    ) {
      set(
        hbaseCopyData,
        'generated_by',
        get(
          !!data ? data : this.logData,
          'generated_by',
          hbaseCopyData.generated_by
        )
      );
      set(
        hbaseCopyData,
        'resource_status',
        get(
          !!data ? data : this.logData,
          'resource_status',
          hbaseCopyData.resource_status
        )
      );
    }
    return hbaseCopyData;
  }

  getPostgreData(data?) {
    const resource_properties = cloneDeep(this.rowData);
    assign(resource_properties, {
      environment_name:
        this.rowData.environment?.name || this.rowData.environment_name,
      environment_endpoint:
        this.rowData.environment?.endpoint || this.rowData.environment_endpoint,
      environment_uuid:
        this.rowData.environment?.uuid || this.rowData.environment_uuid
    });

    const restoreData = {
      ...data,
      restoreTimeStamp: data ? '' : this.getTodayTime().toString(),
      resource_type: this.rowData.sub_type,
      resource_sub_type: this.rowData.sub_type,
      parent_uuid: this.rowData.parent_uuid || this.rowData.parentUuid,
      path: this.rowData.path,
      name: this.rowData.name,
      resource_id: this.rowData.uuid,
      resource_properties: JSON.stringify(resource_properties)
    };
    if (!data) {
      const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
      const startTime =
        this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
      const timeRange = find(this.HbaseTimeRanges, item => {
        return (
          item.endTime >= this.sliderVal + startTime &&
          item.startTime <= this.sliderVal + startTime
        );
      });
      assign(restoreData, {
        uuid: timeRange.copyId,
        backup_type: DataMap.CopyData_Backup_Type.log.value
      });
    }
    if (
      includes(
        [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeDataBase.value
        ],
        this.rowData.sub_type
      ) &&
      !data
    ) {
      assign(data, {
        restoreTimeStamp: this.getTodayTime().toString()
      });
    }
    return restoreData;
  }

  instantRestore(data?) {
    const copyData = {
      ...data,
      restoreTimeStamp: data ? '' : this.getTodayTime().toString(),
      dbUuid: this.rowData.uuid,
      resource_type: this.rowData.sub_type,
      resource_sub_type: this.rowData.sub_type,
      environment_os_type: this.rowData.environment_os_type,
      environment_uuid: this.rowData.environment_uuid,
      parent_uuid: this.rowData.parent_uuid,
      path: this.rowData.path,
      name: this.rowData.name,
      resource_id: this.rowData.uuid,
      resource_properties: JSON.stringify(omit(this.rowData, 'globalSearch')),
      properties: JSON.stringify({
        oracle_metadata: {
          ORACLEPARAM: {
            is_cluster: this.rowData.environment_is_cluster !== 'False',
            db_name: this.rowData.name,
            version: this.rowData.version
          }
        }
      })
    };
    if (
      !data &&
      !copyData.uuid &&
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value,
          DataMap.Resource_Type.oraclePDB.value
        ],
        this.rowData.subType
      )
    ) {
      const date = this.datePipe.transform(this.currentDate, 'yyyy/MM/dd');
      const startTime =
        this.appUtilsService.toSystemTimeLong(`${date} 00:00:00`) / 1000;
      const timeRange = find(this.HbaseTimeRanges, item => {
        return (
          item.endTime >= this.sliderVal + startTime &&
          item.startTime <= this.sliderVal + startTime
        );
      });
      assign(copyData, {
        uuid: timeRange.copyId,
        backup_type: DataMap.CopyData_Backup_Type.log.value
      });
    }
    this.restoreService.restore({
      childResType: this.rowData.sub_type,
      copyData: copyData,
      restoreType: RestoreType.InstanceRestore,
      onOk: () => {
        this.getSliderLines();
      }
    });
  }

  mount(data) {
    this.manualMountService.create({
      item: data,
      resType: this.resType,
      onOk: () => {
        this.getSliderLines();
      }
    });
  }

  modifyRetention(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_modify_retention_policy_label'),
        lvModalKey: 'modify_retention_policy',
        lvOkLoadingText: this.i18n.get('common_loading_label'),
        lvWidth: MODAL_COMMON.smallModal,
        lvContent: ModifyRetentionPolicyComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.getCopyData();
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }

  deleteCopy(data) {
    this.warningMessageService.create({
      rowData: data,
      actionId: OperateItems.DeletingCopy,
      content: this.i18n.get('common_copy_delete_label', [
        this.datePipe.transform(data.display_timestamp, 'yyyy-MM-dd HH:mm:ss')
      ]),
      onOK: modal => {
        this.copiesApiService
          .deleteCopyV1CopiesCopyIdDelete({
            copyId: data.uuid,
            isForced: get(modal, 'contentInstance.forciblyDeleteCopy', null)
          })
          .subscribe(res => {
            if (
              includes(
                mapValues(this.drawModalService.modals, 'key'),
                'copy-detail-modal'
              )
            ) {
              this.drawModalService.destroyModal('copy-detail-modal');
            }
            this.getCopyData();
          });
      }
    });
  }

  onOk = () => {
    this.getSliderLines();
  };

  getDetail(data) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvWidth: MODAL_COMMON.largeWidth,
        lvModalKey: 'copy-detail-modal',
        lvContent: CopyDataDetailComponent,
        lvComponentParams: {
          data: assign(data, {
            ...data,
            optItems: this.getOptsItems(data),
            resType: this.resType,
            name: this.datePipe.transform(
              data.display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            ),
            rowData: this.rowData
          })
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  getCommonRestoreLabel(subType) {
    if (subType === this.resourceResourceType.oraclePDB.value) {
      return this.i18n.get('common_pdb_set_restore_label');
    } else {
      return this.i18n.get('common_restore_label');
    }
  }
}
