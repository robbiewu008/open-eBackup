import { Component, ElementRef, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  CommonConsts,
  DataMap,
  DetectReportsService,
  FsFileExtensionFilterManagementService,
  IODETECTFILESYSTEMService,
  ProtectedResourceApiService,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import * as echarts from 'echarts';
import { isEmpty, isUndefined } from 'lodash';
@Component({
  selector: 'app-file-interception',
  templateUrl: './file-interception.component.html',
  styleUrls: ['./file-interception.component.less']
})
export class FileInterceptionComponent implements OnInit {
  cyberEngineFileExtensionFilterEnabledCount: number = 0; // 所有设备文件拦截开启检测数量
  cyberEngineFileExtensionFilterTotalCount = 0;
  indeviceFileExtensionFilterEnabledCount: number = 0; //	所有设备文件拦截关闭检测的数量
  indeviceFileExtensionFilterTotalCount = 0;

  cyberEngineIoDetectConfigEnabledCount: number = 0; // 所有设备事中开启检测数量
  cyberEngineIoDetectConfigTotalCount = 0;
  indeviceIoDetectConfigEnabledCount: number = 0; // 所有设备事中关闭检测的数量
  indeviceIoDetectConfigTotalCount = 0;

  cyberEngineCopyDetectConfigEnabledCount: number = 0; // 所有设备事后开启检测数量
  cyberEngineCopyDetectConfigTotalCount = 0;
  indeviceCopyDetectConfigEnabledCount: number = 0; // 所有设备事后关闭检测的数量
  indeviceCopyDetectConfigTotalCount = 0;
  dataMap = DataMap;
  charthartOption; // 进度条配置
  chartInstanceMap = new Map();
  constructor(
    private router: Router,
    private el: ElementRef,
    private appUtilsService: AppUtilsService,
    private detectReportsService: DetectReportsService,
    private detectFilesystemService: IODETECTFILESYSTEMService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngOnInit() {
    this.loadData();
  }

  loadData(loading: boolean = true) {
    this.detectReportsService
      .getResourceDetectConfigs({ akLoading: loading })
      .subscribe(res => {
        this.getOpFileExtensionFilter(res, loading);
        this.getOpIoDetectConfig(res, loading);
        this.getOpCopyDetectConfig(res, loading);
      });
  }

  getOpFileExtensionFilter(res, loading) {
    this.fsFileExtensionFilterManagementService
      .getFsFileBlockConfigUsingGET({
        akLoading: loading,
        pageNum: CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE,
        configStatus: [DataMap.File_Extension_Status.enable.value]
      })
      .subscribe(resEn => {
        this.fsFileExtensionFilterManagementService
          .getFsFileBlockConfigUsingGET({
            akLoading: loading,
            pageNum: CommonConsts.PAGE_START + 1,
            pageSize: CommonConsts.PAGE_SIZE
          })
          .subscribe(resAll => {
            this.cyberEngineFileExtensionFilterEnabledCount = resEn.totalCount;
            this.cyberEngineFileExtensionFilterTotalCount = resAll.totalCount;
            this.indeviceFileExtensionFilterEnabledCount =
              res.fileExtensionFilterEnabledCount || 0;
            this.indeviceFileExtensionFilterTotalCount =
              res.fileExtensionFilterEnabledCount +
              res.fileExtensionFilterDisabledCount;
            this.createChart(
              'file-chart',
              this.cyberEngineFileExtensionFilterEnabledCount +
                this.indeviceFileExtensionFilterEnabledCount,
              this.cyberEngineFileExtensionFilterTotalCount +
                this.indeviceFileExtensionFilterTotalCount
            );
          });
      });
  }

  getIoDetectProtected(res, loading, resTotal) {
    this.detectFilesystemService
      .pageQueryProtectObject({
        pageNum: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        akLoading: loading,
        ioDetectStatus: [DataMap.ioDetectEnabled.protected.value]
      })
      .subscribe({
        next: resEnable => {
          this.cyberEngineIoDetectConfigEnabledCount = resEnable.totalCount;
          this.cyberEngineIoDetectConfigTotalCount = resTotal.totalCount;
          this.indeviceIoDetectConfigEnabledCount =
            res.ioDetectConfigEnabledCount - resEnable.totalCount;
          this.indeviceIoDetectConfigTotalCount =
            res.ioDetectConfigEnabledCount +
            res.ioDetectConfigDisabledCount -
            resTotal.totalCount;
          this.createChart(
            'io-chart',
            this.cyberEngineIoDetectConfigEnabledCount +
              this.indeviceIoDetectConfigEnabledCount,
            this.cyberEngineIoDetectConfigTotalCount +
              this.indeviceIoDetectConfigTotalCount
          );
        },
        error: () => {
          // 接口异常时界面应该展示圆环
          this.createChart('io-chart', 0, 0);
        }
      });
  }

  getOpIoDetectConfig(res, loading) {
    this.detectFilesystemService
      .pageQueryProtectObject({
        pageNum: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        akLoading: loading
      })
      .subscribe({
        next: resTotal => this.getIoDetectProtected(res, loading, resTotal),
        error: () => {
          // 接口异常时界面应该展示圆环
          this.createChart('io-chart', 0, 0);
        }
      });
  }

  getOpCopyDetectConfig(res, loading) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        akLoading: loading,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.LocalFileSystem.value]
        })
      })
      .subscribe(resTotal => {
        this.protectedResourceApiService
          .ListResources({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            akLoading: loading,
            conditions: JSON.stringify({
              subType: [DataMap.Resource_Type.LocalFileSystem.value],
              protectionStatus: [
                ['in'],
                DataMap.Protection_Status.protected.value
              ]
            })
          })
          .subscribe(resEnable => {
            this.cyberEngineCopyDetectConfigEnabledCount = resEnable.totalCount;
            this.cyberEngineCopyDetectConfigTotalCount = resTotal.totalCount;
            this.indeviceCopyDetectConfigEnabledCount =
              res.copyDetectConfigEnabledCount - resEnable.totalCount;
            this.indeviceCopyDetectConfigTotalCount =
              res.copyDetectConfigEnabledCount +
              res.copyDetectConfigDisabledCount -
              resTotal.totalCount;
            this.createChart(
              'copy-chart',
              this.cyberEngineCopyDetectConfigEnabledCount +
                this.indeviceCopyDetectConfigEnabledCount,
              this.cyberEngineCopyDetectConfigTotalCount +
                this.indeviceCopyDetectConfigTotalCount
            );
          });
      });
  }

  gotoFileInterception() {
    this.router.navigateByUrl(
      RouterUrl.ExploreAntiRansomwareProtectionFileInterception
    );
  }

  gotoIoDetection() {
    this.router.navigateByUrl(
      RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection
    );
  }

  gotoDetection($event, status?) {
    if ($event?.stopPropagation) {
      $event?.stopPropagation();
    }
    if (!isUndefined(status)) {
      this.appUtilsService.setCacheValue('protectionStatus', status);
    }
    this.router.navigateByUrl(
      RouterUrl.ExploreAntiRansomwareProtectionDataBackup
    );
  }

  createChart(chartId: string, enableCount: number, totalCount: number) {
    let percentage = (enableCount / totalCount) * 100;
    percentage = isNaN(percentage) ? 0 : percentage;
    const showValue = parseFloat(percentage.toFixed(1));
    let chart = this.chartInstanceMap.get(chartId);
    if (isEmpty(chart)) {
      chart = echarts.init(
        this.el.nativeElement.querySelector(`#${chartId}`),
        null,
        { renderer: 'svg' }
      );
      this.chartInstanceMap.set(chartId, chart);
    }

    const hideTickMark = {
      axisLine: {
        show: false
      },
      axisTick: {
        show: false
      },
      axisLabel: {
        show: false
      },
      splitLine: {
        show: false
      }
    };
    this.charthartOption = {
      legend: {},
      title: {
        text: `${showValue}%`,
        x: 'center',
        y: '37%',
        textStyle: {
          color: '#FFF',
          fontSize: 19,
          fontWeight: 'normal'
        }
      },
      angleAxis: {
        max: 100,
        // 隐藏刻度线
        ...hideTickMark
      },
      radiusAxis: {
        type: 'category',
        // 隐藏刻度线
        ...hideTickMark
      },
      polar: {
        center: ['50%', '50%'],
        radius: ['71%', '82%'] // 图形大小
      },
      series: [
        {
          type: 'bar',
          legendHoverLink: false,
          silent: true,
          cursor: 'default',
          data: [
            {
              value: percentage,
              itemStyle: {
                color: new echarts.graphic.LinearGradient(1, 0, 0, 0, [
                  {
                    offset: 0,
                    color: '#7FCE56'
                  },
                  {
                    offset: 1,
                    color: '#00AFDD'
                  }
                ])
              }
            }
          ],
          coordinateSystem: 'polar', // 变成极坐标系
          roundCap: true, // 两端变成 圆角
          barGap: '-100%', // 两环重叠
          z: 99 // 有数值的环置顶
        },
        {
          // 灰色环
          type: 'bar',
          legendHoverLink: false,
          silent: true,
          cursor: 'default',
          data: [
            {
              value: 100,
              itemStyle: {
                color: '#33373D' // 默认一个100%的圆环占位
              }
            }
          ],
          coordinateSystem: 'polar',
          roundCap: true,
          barGap: '-100%' // 两环重叠
        },
        {
          // 外层圆
          type: 'pie',
          radius: ['99%', '100%'],
          center: ['50%', '50%'],
          xAxis: false,
          data: [
            {
              value: 1,
              itemStyle: {
                color: '#33373d'
              }
            }
          ],
          label: {
            show: false
          }
        }
      ]
    };
    chart.setOption(this.charthartOption);
  }
}
