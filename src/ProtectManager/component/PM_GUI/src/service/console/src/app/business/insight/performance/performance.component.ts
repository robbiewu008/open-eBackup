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
  ElementRef,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  CommonConsts,
  CookieService,
  DataMap,
  getAppTheme,
  GlobalService,
  I18NService,
  isRBACDPAdmin,
  MODAL_COMMON,
  SYSTEM_TIME,
  THEME_TRIGGER_ACTION,
  ThemeEnum,
  WarningMessageService
} from 'app/shared';
import {
  ClustersApiService,
  PerformanceApiDescService
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import * as echarts from 'echarts';
import {
  assign,
  defer,
  each,
  filter,
  find,
  first,
  flatten,
  get,
  includes,
  isEmpty,
  isNaN,
  isUndefined,
  map,
  set,
  zip
} from 'lodash';
import { combineLatest, Subject, Subscription, takeUntil, timer } from 'rxjs';
import { MultiSettingComponent } from './multi-setting/multi-setting.component';

@Component({
  selector: 'aui-performance',
  templateUrl: './performance.component.html',
  styleUrls: ['./performance.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class PerformanceComponent implements OnInit, OnDestroy {
  isEmpty = isEmpty;
  dataMap = DataMap;
  allClose = false;
  popoverFlag = false;
  configData = [];
  cluster = JSON.parse(
    decodeURIComponent(this.cookieService.get('currentCluster'))
  ) || {
    clusterId: DataMap.Cluster_Type.local.value,
    clusterType: DataMap.Cluster_Type.local.value
  };
  currentCluster;
  isDataBackupOrDecouple = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value,
      DataMap.Deploy_Type.decouple.value,
      DataMap.Deploy_Type.openServer.value
    ],
    this.i18n.get('deploy_type')
  );
  clusters = [];
  onResizeCallback = [];
  performanceTimeOptions = [];
  timeSeleted;
  showMonitor = false;
  loading = false;
  collapsed;
  performanceSub: Subscription = new Subscription();
  showLatency = false;
  showIops = false;
  showBindwidth = false;
  showCopyLatency = false;
  showCopyIops = false;
  showCopyBindwidth = false;
  showNasBindwidth = false;
  showCfsLatency = false;
  showNfsLatency = false;
  showObjectServiceLatency = false;
  activeNode = '';
  currentNode = '';
  selectedNodeIp = '';
  staticsMode = 'avg';
  deviceOrStoragePoolModel = 'device';
  capacityTabId = 'backup';
  storagePoolHiddenChart = false;
  timeType = {
    lastMinute: 0,
    lastHour: 1,
    last24Hour: 2,
    lastWeek: 3,
    lastMonth: 4,
    lastYear: 5
  };
  startTime: number;
  endTime: number;
  isDeleteHistory = false;
  hasRemoveHistoryData;
  performanceMonitorLabel = this.i18n.get('insight_performance_monitor_label');
  maxLabel = this.i18n.get('common_max_label');
  avgLabel = this.i18n.get('common_avg_label');
  device = this.i18n.get('common_equipment_label');
  storagePool = this.i18n.get('common_storage_pool_label');
  latencyLabel = this.i18n.get('insight_latency_label');
  iopsLabel = this.i18n.get('common_iops_label');
  bindwidthLabel = this.i18n.get('common_bindwidth_kb_label');
  noData = this.i18n.get('common_no_data_label');
  switchHelp = this.i18n.get('insight_performance_switch_help_label');
  switchOffContent = this.i18n.get('insight_performance_switch_off_label');
  isCloudbackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  tipShow = false;
  chartConfigMap = new Map();
  chartInstanceMap = new Map();
  destroy$ = new Subject();

  @ViewChild('tipContentTpl', { static: false }) tipContentTpl: TemplateRef<
    any
  >;

  constructor(
    public i18n: I18NService,
    public el: ElementRef,
    public performanceApiService: PerformanceApiDescService,
    public datePipe: DatePipe,
    public clusterApiService: ClustersApiService,
    public warningMessageService: WarningMessageService,
    public systemTimeService: SystemTimeService,
    private messageBox: MessageboxService,
    private infoMessageService: InfoMessageService,
    public capacityCalculateLabel: CapacityCalculateLabel,
    private cookieService: CookieService,
    private drawModalService: DrawModalService,
    public appUtilsService: AppUtilsService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy() {
    this.performanceSub.unsubscribe();
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  onChange() {
    this.currentNode = '';
    this.activeNode = '';
    this.selectedNodeIp = '';
    this.clusters = [];
    this.ngOnInit();
  }

  isThemeLight(): boolean {
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  // 关闭与开启性能监控按钮
  switchMonitor() {
    if (this.showMonitor) {
      this.isDeleteHistory = false;
      this.messageBox.info({
        lvHeader: this.i18n.get('insight_switch_off_label'),
        lvContent: this.tipContentTpl,
        lvWidth: 600,
        lvFooter: [
          {
            label: this.i18n.get('common_ok_label'),
            onClick: modal => {
              if (this.isDeleteHistory) {
                this.warningMessageService.create({
                  content: this.i18n.get(
                    'insight_performance_remove_history_label'
                  ),
                  onOK: () => {
                    this.performanceApiService
                      .enablePerformanceUsingPUT({
                        enable: false,
                        hasRemoveHistoryData: true,
                        memberEsn: this.currentCluster
                      })
                      .subscribe(res => {
                        this.loading = false;
                        this.showMonitor = !this.showMonitor;
                        this.hasRemoveHistoryData = true;
                      });
                  },
                  onCancel: () => {
                    this.loading = false;
                  },
                  lvAfterClose: result => {
                    if (result && result.trigger === 'close') {
                      this.loading = false;
                    }
                  }
                });
              } else {
                this.performanceApiService
                  .enablePerformanceUsingPUT({
                    enable: false,
                    hasRemoveHistoryData: false,
                    memberEsn: this.currentCluster
                  })
                  .subscribe(res => {
                    this.loading = false;
                    this.showMonitor = !this.showMonitor;
                    this.hasRemoveHistoryData = false;
                    this.reloadView();
                  });
              }
              modal.close();
            }
          },
          {
            type: 'primary',
            label: this.i18n.get('common_cancel_label'),
            onClick: modal => {
              this.loading = false;
              modal.close();
            }
          }
        ]
      });
    } else {
      this.infoMessageService.create({
        content: this.i18n.get('insight_enable_performance_label'),
        onOK: () => {
          this.performanceApiService
            .enablePerformanceUsingPUT({
              enable: true,
              hasRemoveHistoryData: false,
              memberEsn: this.currentCluster
            })
            .subscribe(res => {
              this.loading = false;
              this.showMonitor = !this.showMonitor;
              this.autoReload();
              if (this.isDataBackupOrDecouple) {
                setTimeout(() => {
                  this.queryAllConfig();
                }, 2000);
              }
            });
        },
        onCancel: () => {
          this.loading = false;
        }
      });
    }
  }

  clusterItemClick(e) {
    this.currentCluster = e.data.memberEsn;
    this.currentNode = e.data.label;
    this.activeNode = e.data.id;
    if (!isEmpty(e.data.items)) {
      this.selectedNodeIp = '';
    } else {
      this.selectedNodeIp = e.data.ip;
    }
    this.getPerformanceConfig();
  }

  toDateTime(ms) {
    return this.datePipe.transform(
      ms,
      'yyyy-MM-dd HH:mm:ss',
      SYSTEM_TIME.timeZone
    );
  }

  lvTabChange(id) {
    this.storagePoolHiddenChart =
      this.deviceOrStoragePoolModel === 'storagePool';
    this.capacityTabId = 'backup';
    this.reloadView();
  }

  reloadView(mask = true) {
    if (!this.showMonitor) {
      if (this.hasRemoveHistoryData) {
        this.chartInstanceMap?.forEach((item, key) => {
          if (!item.isDisposed()) {
            item.dispose();
          }
        });
        this.chartInstanceMap?.clear();
        return;
      }
    }

    this.systemTimeService.getSystemTime(false).subscribe(res => {
      this.endTime = this.appUtilsService.toSystemTimeLong(res.time);
      if (!this.endTime) {
        this.endTime = this.appUtilsService.toSystemTimeLong(
          res.time.replace(/-/g, '/')
        );
      }
      switch (this.timeSeleted) {
        case 0:
          this.startTime = this.endTime - 300 * 1000;
          break;
        case 1:
          this.startTime = this.endTime - 1800 * 1000;
          break;
        case 2:
          this.startTime = this.endTime - 24 * 3600 * 1000;
          break;
        case 3:
          this.startTime = this.endTime - 7 * 24 * 3600 * 1000;
          break;
        case 4:
          this.startTime = this.endTime - 30 * 24 * 3600 * 1000;
          break;
        case 5:
          this.startTime = this.endTime - 365 * 24 * 3600 * 1000;
          break;
        default:
          break;
      }
      this.getMonitoringData(mask);
    });
  }

  refreshView() {
    this.reloadView();
  }

  staticsModeChange() {
    this.reloadView();
  }

  deviceOrStoragePoolModelChange(id) {
    this.storagePoolHiddenChart = id === 'storagePool';
    this.capacityTabId = 'backup';
    this.reloadView();
  }

  performanceTimeChange(e) {
    this.reloadView();
  }

  getTipTitle(dom): string {
    switch (dom) {
      case 'latency-chart':
        return this.i18n.get('insight_backup_latency_tip_label');
      case 'cfs-latency-chart-distributed':
        return this.i18n.get('insight_cfs_backup_latency_tip_label');
      case 'nfs-latency-chart-distributed':
        return this.i18n.get('insight_nfs_backup_latency_tip_label');
      case 'object-service-latency-chart-distributed':
        return this.i18n.get('insight_object_service_backup_latency_tip_label');
      case 'iops-chart':
        return this.i18n.get('common_iops_label');
      case 'bindwidth-chart':
        return this.i18n.get('common_bindwidth_no_unit_label');
      case 'copy-latency-chart':
        return this.i18n.get('insight_copy_latency_tip_label');
      case 'copy-iops-chart':
        return this.i18n.get('insight_copy_iops_tip_label');
      case 'copy-bindwidth-chart':
        return this.i18n.get('insight_copy_bindwidth_tip_label');
      case 'copy-nas-bindwidth-chart':
        return this.i18n.get('insight_copy_get_bindwidth_tip_label');
      default:
        return this.i18n.get('insight_backup_latency_tip_label');
    }
  }

  createChart(xAxisDataTemp, seriesData, dom, unit, writeBindData?) {
    const node: any = document.getElementById(`${dom}`);
    if (!node) {
      return;
    }
    let capacityChart = this.chartInstanceMap.get(dom);
    if (isEmpty(capacityChart)) {
      // map里没取到不代表chart没有实例化
      let chart = echarts.getInstanceByDom(node);
      if (!chart) {
        chart = echarts.init(this.el.nativeElement.querySelector(`#${dom}`));
      }
      this.chartInstanceMap.set(dom, chart);
      capacityChart = chart;
    } else {
      this.resizeChart(capacityChart);
    }
    if (
      [
        'copy-latency-chart',
        'copy-iops-chart',
        'copy-nas-bindwidth-chart',
        'copy-bindwidth-chart'
      ].includes(dom)
    ) {
      if (!this.storagePoolHiddenChart) {
        if (!capacityChart.isDisposed()) {
          capacityChart.dispose();
        }
        capacityChart = echarts.init(
          this.el.nativeElement.querySelector(`#${dom}`)
        );
        this.chartInstanceMap.set(dom, capacityChart);
      }
    }
    window.addEventListener('resize', () => {
      if (!capacityChart.isDisposed()) {
        capacityChart.resize();
      }
    });
    const xAxisData = [];
    xAxisDataTemp.forEach(item => {
      xAxisData.push(item);
    });

    let seriesDataTemp = [];
    if (this.deviceOrStoragePoolModel === 'storagePool') {
      seriesDataTemp = map(seriesData, item => {
        return filter(item.values, dataItem => {
          return !isNaN(+dataItem);
        });
      });
    } else {
      seriesDataTemp = filter(seriesData, item => {
        return !isNaN(+item);
      });
    }
    const writeBindDataTemp = filter(writeBindData, item => {
      return !isNaN(+item);
    });
    let maxYAxis = isEmpty(seriesDataTemp)
      ? 1
      : Math.max.apply(null, flatten(seriesDataTemp));
    let writeArrData = [];
    if (
      includes(
        [
          'latency-chart',
          'copy-latency-chart',
          'cfs-latency-chart-distributed',
          'nfs-latency-chart-distributed',
          'object-service-latency-chart-distributed'
        ],
        dom
      )
    ) {
      maxYAxis = (+maxYAxis.toFixed(3) * 1.5).toFixed(3);
    } else if (includes(['iops-chart', 'copy-iops-chart'], dom)) {
      maxYAxis = Math.ceil(Math.ceil(+maxYAxis) * 1.5);
    } else {
      const maxRead = Math.ceil(
        +Math.max.apply(null, flatten(seriesDataTemp)) * 1.5
      );
      const maxWrite = isEmpty(writeBindDataTemp)
        ? 1
        : Math.ceil(+Math.max.apply(null, writeBindDataTemp) * 1.5);
      maxYAxis = Math.max(maxRead, maxWrite);
      // 写数据
      if (this.selectedNodeIp) {
        writeArrData = zip(xAxisData, writeBindData);
      }
      unit = 'KB/s';
    }
    const dataArr = zip(xAxisData, seriesData);
    let storagePoolSeries = [];
    if (this.deviceOrStoragePoolModel === 'storagePool') {
      map(seriesData, item => {
        storagePoolSeries.push(
          assign(
            {},
            {
              data: zip(xAxisData, item.values),
              name: item.name,
              color: item.color,
              type: 'line',
              showSymbol: false,
              itemStyle: {
                borderWidth: 3
              },
              lineStyle: {
                width: 2
              }
            }
          )
        );
      });
    }
    const capacityOption = {
      grid: {
        y: 40,
        y2: 40,
        x: 10,
        x2: 40,
        containLabel: true
      },
      noDataLoadingOption: {
        text: 'Chat\nUnavailable',
        textStyle: {
          color: '#000',
          fontSize: '18'
        },
        effect: 'bubble'
      },
      graphic: [
        {
          id: 'unit',
          type: 'text',
          left: 8,
          top: 16,
          cursor: 'unset',
          zlevel: 100,
          style: {
            text: `${unit}`,
            textAlign: 'right',
            fill: this.isThemeLight() ? '#4D4D4D' : '#B3B3B3',
            width: 31,
            height: 14,
            fontSize: 12
          }
        }
      ],
      tooltip: {
        trigger: 'axis',
        axisPointer: {
          type: 'line',
          z: 10,
          lineStyle: {
            color: this.isThemeLight() ? '#d9d9d9' : '#4D4D4D'
          }
        },
        backgroundColor: this.isThemeLight() ? '#fff' : '#272933',
        extraCssText: `border-radius: 0; padding: 12px 16px; border:1px solid ${
          this.isThemeLight() ? '#fff' : '#272933'
        }`,
        formatter: params => {
          let tip = `<div style='font-size: 12px; color: #808080'>${this.toDateTime(
            params[0].axisValue
          )}</div>`;
          params.forEach(res => {
            let value = '';
            let resData = res.data[1];
            if (
              includes(
                [
                  'bindwidth-chart',
                  'copy-bindwidth-chart',
                  'copy-nas-bindwidth-chart'
                ],
                dom
              )
            ) {
              resData = this.capacityCalculateLabel.transform(
                resData,
                '1.3-3',
                CAPACITY_UNIT.KB,
                true
              );
            }
            if (this.deviceOrStoragePoolModel === 'storagePool') {
              value = res.seriesName || '';
            } else {
              if (res.componentIndex === 1) {
                value = this.i18n.get('common_bindwidth_write_label');
              }
              if (res.componentIndex === 0) {
                value = this.getTipTitle(dom);
              }
            }
            tip += `<div style='font-size: 12px; color: #808080; padding-right:10px;'>
                  <span style='margin-right: 6px;display: inline-block;width: 8px;height:8px;border-radius:8px;background:${
                    res.color
                  }'></span>${value}
                  <span style='font-size: 12px; color: #808080;'>${resData}<span>${
              !includes(
                [
                  'bindwidth-chart',
                  'copy-bindwidth-chart',
                  'copy-nas-bindwidth-chart'
                ],
                dom
              )
                ? ' ' + unit
                : '/s'
            }</span></span>
                  </div>`;
          });
          return tip;
        }
      },
      xAxis: {
        splitLine: {
          show: false
        },
        interval: (() => {
          switch (this.timeSeleted) {
            case 0:
              return 50 * 1000;
            case 1:
              return 5 * 60 * 1000;
            case 2:
              return 4 * 3600 * 1000;
            case 3:
              return 28 * 3600 * 1000;
            case 4:
              return 5 * 24 * 3600 * 1000;
            case 5:
              return 61 * 24 * 3600 * 1000;
            default:
              return 50 * 1000;
          }
        })(),
        min: (() => {
          switch (this.timeSeleted) {
            case 0:
              return this.endTime - 6 * 50 * 1000;
            case 1:
              return this.endTime - 6 * 5 * 60 * 1000;
            case 2:
              return this.endTime - 6 * 4 * 3600 * 1000;
            case 3:
              return this.endTime - 6 * 28 * 3600 * 1000;
            case 4:
              return this.endTime - 6 * 5 * 24 * 3600 * 1000;
            case 5:
              return this.endTime - 365 * 24 * 3600 * 1000;
            default:
              return this.endTime - 6 * 50 * 1000;
          }
        })(),
        max: this.endTime,
        boundaryGap: false,
        axisTick: {
          alignWithLabel: true,
          lineStyle: {
            width: 2
          }
        },
        axisLine: {
          show: true,
          lineStyle: {
            color: this.isThemeLight() ? '#E6E6E6' : '#262626',
            width: 1
          }
        },
        axisLabel: {
          color: this.isThemeLight() ? '#4D4D4D' : '#B3B3B3',
          formatter: params => {
            params = this.toDateTime(params);
            let newParamsName = '';
            const paramsNameNumber = params.length;
            const provideNumber = 10;
            const rowNumber = Math.ceil(paramsNameNumber / provideNumber);
            if (paramsNameNumber > provideNumber) {
              for (let p = 0; p < rowNumber; p++) {
                let tempStr = '';
                const start = p * provideNumber;
                const end = start + provideNumber;
                if (p === rowNumber - 1) {
                  tempStr = params.substring(start, paramsNameNumber);
                } else {
                  tempStr = params.substring(start, end) + '\n';
                }
                newParamsName += tempStr;
              }
            } else {
              newParamsName = params;
            }
            return newParamsName;
          }
        }
      },
      yAxis: {
        type: 'value',
        min: 0,
        max: maxYAxis,
        minInterval: 0.01,
        axisLabel: {
          formatter: item => {
            return item + '';
          },
          show: true,
          color: this.isThemeLight() ? '#4D4D4D' : '#B3B3B3'
        },
        splitLine: {
          lineStyle: {
            type: 'solid',
            color: this.isThemeLight() ? '#E6E6E6' : '#262626',
            width: 1
          }
        },
        axisLine: {
          lineStyle: {
            color: 'transparent'
          }
        }
      },
      series: [
        {
          data: dataArr,
          type: 'line',
          color: '#3388FF',
          showSymbol: dataArr.length === 1,
          itemStyle: {
            borderWidth: 3
          },
          lineStyle: {
            width: 2
          }
        }
      ]
    };

    if (this.deviceOrStoragePoolModel === 'storagePool') {
      capacityChart.on('legendselectchanged', (legend: any) => {
        this.chartConfigMap.set(dom, legend.selected);
      });
      let legendData = [];
      map(seriesData, item => {
        legendData.push(item.name);
      });
      set(
        capacityOption,
        'legend',
        assign(
          {},
          {
            data: legendData,
            selected: this.chartConfigMap.get(dom)
          }
        )
      );
      set(capacityOption, 'series', storagePoolSeries);
    }
    // 第二个参数为true时，切换图表不会合并配置项
    capacityChart?.setOption(capacityOption, true);
  }

  private resizeChart(capacityChart) {
    if (isEmpty(capacityChart) || capacityChart.isDisposed()) {
      return;
    }
    const canvasHeight = capacityChart.getDom().firstChild.offsetHeight;
    if (!canvasHeight) {
      // 延迟resize
      setTimeout(() => {
        capacityChart.clear();
        capacityChart?.resize();
      }, 100);
    }
    // 这里也可以立即resize,出于性能考虑，只有在渲染异常时进行resize()
  }

  createChartWithData(res, data, dom, unit) {
    if (isEmpty(res) || isEmpty(data)) {
      this.createChart([], [], dom, unit);
    } else {
      this.createChart(
        data.timestamps || [],
        data.indicatorValues || [],
        dom,
        data.unit || unit
      );
    }
  }

  getPerformanceConfig(isModify = false) {
    const params = {
      memberEsn: this.currentCluster
    };
    if (this.appUtilsService.isDecouple) {
      assign(params, {
        targetEsn: this.currentCluster
      });
    }
    this.performanceApiService
      .getPerformanceConfigUsingGET(params)
      .subscribe(res => {
        this.showMonitor = isModify
          ? find(this.configData, { esn: this.currentCluster }).open
          : res.isPerformanceConfigOpen;
        this.hasRemoveHistoryData = isModify
          ? this.hasRemoveHistoryData
          : res.hasRemoveHistoryData === 1;
        this.autoReload();
      });
  }

  autoReload() {
    this.reloadView(true);
    this.performanceSub.unsubscribe();
    this.performanceSub = timer(0, 10 * 1e3).subscribe(res => {
      this.reloadView(false);
    });
  }

  processEchartData(res) {
    this.showIops = true;
    this.showLatency = true;
    this.showBindwidth = true;
    this.showCopyIops = true;
    this.showCopyLatency = true;
    this.showCopyBindwidth = !this.storagePoolHiddenChart;
    this.showNasBindwidth = !this.storagePoolHiddenChart;

    // 备份/恢复：IOPS、I/O响应时间、带宽性能数据
    const iopsData = find(res, { indicator: 1 }) as any;
    const latencyData = find(res, { indicator: 2 }) as any;
    const bindwidthData = find(res, { indicator: 3 }) as any;

    // 复制：I/O响应时间、IOPS、带宽性能数据
    const copyLatencyData = find(res, { indicator: 4 }) as any;
    const copyIopsData = find(res, { indicator: 5 }) as any;
    const copyBindwidthData = find(res, { indicator: 6 }) as any;
    const copyNasBindwidthData = find(res, { indicator: 7 }) as any;

    // 重绘图表
    this.createChartWithData(res, iopsData, 'iops-chart', 'IO/s');
    this.createChartWithData(res, bindwidthData, 'bindwidth-chart', 'KB');
    if (isEmpty(res) || isEmpty(latencyData)) {
      this.createChart([], [], 'latency-chart', []);
    } else {
      let indicatorValues = [];
      // 如果是storagePool模式，indicatorValues数据结构不同
      if (this.deviceOrStoragePoolModel === 'storagePool') {
        indicatorValues = map(latencyData.indicatorValues || [], item => {
          return {
            name: item.name,
            color: item.color,
            values: map(item.values || [], dataItem => {
              return isNaN(+dataItem) ? '-' : (+dataItem / 1000).toFixed(3);
            })
          };
        });
      } else {
        indicatorValues = map(latencyData.indicatorValues || [], item => {
          return isNaN(+item) ? '-' : (+item / 1000).toFixed(3);
        });
      }
      this.createChart(
        latencyData.timestamps || [],
        indicatorValues,
        'latency-chart',
        'ms'
      );
    }

    if (!this.storagePoolHiddenChart) {
      // 复制响应时间
      if (isEmpty(res) || isEmpty(copyLatencyData)) {
        this.createChart([], [], 'copy-latency-chart', 'us');
      } else {
        let indicatorValues = [];
        // 如果是storagePool模式，indicatorValues数据结构不同
        if (this.deviceOrStoragePoolModel === 'storagePool') {
          indicatorValues = map(latencyData.indicatorValues || [], item => {
            return {
              name: item.name,
              color: item.color,
              values: map(item.values || [], dataItem => {
                return isNaN(+dataItem) ? '-' : (+dataItem).toFixed(3);
              })
            };
          });
        } else {
          indicatorValues = map(copyLatencyData.indicatorValues || [], item => {
            return isNaN(+item) ? '-' : (+item).toFixed(3);
          });
        }
        this.createChart(
          copyLatencyData.timestamps || [],
          indicatorValues,
          'copy-latency-chart',
          copyLatencyData.unit || 'us'
        );
      }

      // 复制IOPS
      this.createChartWithData(res, copyIopsData, 'copy-iops-chart', 'IO/s');
      // 复制带宽
      this.createChartWithData(
        res,
        copyBindwidthData,
        'copy-bindwidth-chart',
        'KB'
      );
      // 复制接收带宽
      this.createChartWithData(
        res,
        copyNasBindwidthData,
        'copy-nas-bindwidth-chart',
        'KB'
      );
    }
  }

  getMonitoringData(mask = true) {
    if (isUndefined(this.currentCluster) && this.isDataBackupOrDecouple) {
      return;
    }

    const params = {
      indicatorList: [1, 2, 3, 4, 5, 6, 7],
      staticsMode: this.staticsMode,
      startTime: this.startTime,
      endTime: this.endTime,
      akLoading: mask,
      nodeIp: this.selectedNodeIp,
      akDoException: false
    };

    set(params, 'memberEsn', this.currentCluster);
    if (this.appUtilsService.isDecouple) {
      assign(params, { targetEsn: this.currentCluster });
    }

    if (this.deviceOrStoragePoolModel === 'storagePool') {
      set(params, 'deviceOrStoragePoolModel', this.deviceOrStoragePoolModel);
      this.performanceApiService
        .getStoragePoolPerformanceUsingGET(params)
        .subscribe(res => {
          this.processEchartData(res);
        });
    } else {
      this.performanceApiService
        .getPerformanceUsingGET(params)
        .subscribe(res => {
          this.processEchartData(res);
        });
    }
  }

  initTimeOptios() {
    this.performanceTimeOptions = [
      {
        label: this.i18n.get('common_last_five_minute_label'),
        value: this.timeType.lastMinute,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_half_hour_label'),
        value: this.timeType.lastHour,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_day_label'),
        value: this.timeType.last24Hour,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_week_label'),
        value: this.timeType.lastWeek,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_month_label'),
        value: this.timeType.lastMonth,
        isLeaf: true
      },
      {
        label: this.i18n.get('common_last_year_label'),
        value: this.timeType.lastYear,
        isLeaf: true
      }
    ];
    this.timeSeleted = this.performanceTimeOptions[0].value;
  }

  initConditions() {
    const currentTime = new Date().getTime();
    this.startTime = currentTime - 300 * 1000;
    this.endTime = currentTime;
  }

  initClusters() {
    const params = this.isDataBackupOrDecouple
      ? {
          startPage: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          roleList: this.appUtilsService.isDecouple
            ? [DataMap.Target_Cluster_Role.backupStorage.value]
            : [
                DataMap.Target_Cluster_Role.primaryNode.value,
                DataMap.Target_Cluster_Role.backupNode.value,
                DataMap.Target_Cluster_Role.memberNode.value
              ]
        }
      : {
          startPage: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          typeList: [1]
        };

    this.clusterApiService.getClustersInfoUsingGET(params).subscribe(res => {
      get(res, 'records', []);
      // 排序规则：
      // 第一层:按照节点角色：主节点、备、成员节点
      // 第二层：按照节点状态：在线、设置中、离线、删除中
      const rule = [
        DataMap.Node_Status.online.value,
        DataMap.Node_Status.setting.value,
        DataMap.Node_Status.offline.value,
        DataMap.Node_Status.deleting.value
      ];
      res.records = res.records.filter(
        item => item.deviceType !== DataMap.poolStorageDeviceType.Server.value
      );
      res.records.sort((a, b) => {
        if (a.role !== b.role) {
          return b.role - a.role;
        } else {
          return rule.indexOf(a.status) - rule.indexOf(b.status);
        }
      });

      combineLatest(
        map(res.records, item => {
          const parentNode = {
            id: item.clusterId,
            ip: item.ip,
            label: item.clusterName,
            expanded: true,
            items: [],
            clusterId: item.clusterId,
            memberEsn: item.storageEsn,
            status: item.status,
            disabled: item.status !== DataMap.Cluster_Status.online.value
          };
          this.clusters.push(parentNode);
          if (!isRBACDPAdmin(this.cookieService.role)) {
            const nodeParams = {
              clusterId: item.clusterId,
              memberEsn: item.storageEsn,
              startPage: CommonConsts.PAGE_START,
              pageSize: CommonConsts.PAGE_SIZE
            };

            return this.clusterApiService.queryClusterNodeInfoUsingGET(
              nodeParams
            );
          }
        })
      ).subscribe(nodesArr => {
        each(nodesArr, (nodes, idx) => {
          if (nodes.records.length > 0) {
            each(nodes.records, n => {
              this.clusters[idx].items.push({
                id: `${this.clusters[idx].clusterId}-${n.managementIPv4 ||
                  n.managementIPv6}`,
                ip: n.managementIPv4 || n.managementIPv6,
                label: n.nodeName,
                tips: n.nodeName,
                memberEsn: this.clusters[idx].memberEsn
              });
            });
          }
        });
        this.currentCluster = first(this.clusters).memberEsn;
        this.activeNode = first(this.clusters).id;
        this.currentNode = first(this.clusters).label;
        this.getPerformanceConfig();
      });
    });
  }

  ngOnInit() {
    this.initTimeOptios();
    this.initConditions();
    this.initClusters();
    if (this.isDataBackupOrDecouple) {
      this.queryAllConfig();
    }
    this.globalService
      .getState(THEME_TRIGGER_ACTION)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.reloadView(true);
      });
  }

  queryAllConfig(loading?) {
    this.performanceApiService
      .getNodesPerformanceConfigUsingGET({ akLoading: loading })
      .subscribe(res => {
        this.configData = res;
        if (find(this.configData, { open: true })) {
          this.allClose = false;
          this.popoverFlag = false;
        } else {
          this.popoverFlag = true;
          this.allClose = true;
        }
        this.showTipFn();
      });
  }

  close = () => {
    this.popoverFlag = false;
    this.allClose = true;
    this.showTipFn();
  };

  showTipFn() {
    defer(() => {
      this.tipShow = this.popoverFlag && this.allClose;
    });
  }

  setting() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_setting_label'),
        lvContent: MultiSettingComponent,
        lvComponentParams: {
          data: this.configData,
          performanceSub: this.performanceSub,
          updateLoad: options => this.updateLoad(options)
        },
        lvFooter: [
          {
            id: 'close',
            label: this.i18n.get('common_close_label'),
            onClick: modal => {
              modal.close();
            }
          }
        ]
      }
    });
  }

  updateLoad(options: any) {
    this.hasRemoveHistoryData = isUndefined(options.hasRemoveHistoryData)
      ? this.hasRemoveHistoryData
      : options.hasRemoveHistoryData;
    setTimeout(() => {
      this.getPerformanceConfig(true);
      this.queryAllConfig(false);
    }, 3000);
  }
}
