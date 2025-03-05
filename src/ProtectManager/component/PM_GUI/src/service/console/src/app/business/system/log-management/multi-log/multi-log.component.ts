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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild,
  AfterViewInit,
  ViewEncapsulation
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  ApiLogApiService,
  CookieService,
  BackupClustersApiService,
  ApiExportFilesApiService as ExportFileApiService,
  RouterUrl
} from 'app/shared';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  every,
  includes,
  isEmpty,
  isNil,
  isUndefined,
  map,
  now,
  set,
  size,
  some,
  toNumber,
  union,
  uniq
} from 'lodash';

@Component({
  selector: 'aui-multi-log',
  templateUrl: './multi-log.component.html',
  styleUrls: ['./multi-log.component.less'],
  encapsulation: ViewEncapsulation.None
})
export class MultiLogComponent implements OnInit, AfterViewInit {
  includes = includes;
  dataMap = DataMap;
  size = size;
  proConfig;
  selectionData = [];
  tableConfig: TableConfig;
  tableData: TableData;
  exportLogFormGroup: FormGroup;
  controllerNameOptions = [];
  cotrollerNames = [];
  componentOptions = [];
  nodeNameOptions = [];
  loading = false;
  controllerLoading = false;
  visible;
  settingVal;
  components = this.dataMapService
    .toArray('Component_Name')
    .filter(v => (v.isLeaf = true));

  levelOptions = this.dataMapService
    .toArray('Log_Level')
    .filter(v => (v.isLeaf = true));
  periodOptions = this.dataMapService.toArray('exportLogRange');
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [251])
  };
  customTimeRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10000])
  };
  selectTimeRangeType = {
    custom: 'custom',
    day: 'day'
  };
  @ViewChild('logLevelTpl', { static: true }) logLevelTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private cdr: ChangeDetectorRef,
    private batchOperateService: BatchOperateService,
    private infoMessageService: InfoMessageService,
    private logManagerApiService: LogManagerApiService,
    private exportFilesService: ExportFilesService,
    private virtualScroll: VirtualScrollService,
    private apiLogApiService: ApiLogApiService,
    private backupClustersApiService: BackupClustersApiService,
    public cookieService: CookieService,
    private exportFilesApi?: ExportFileApiService,
    private message?: MessageService,
    public router?: Router
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initConfig();
    this.getNodes();
  }

  onChange() {
    this.ngOnInit();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initForm() {
    const time = new Date();
    const year = time.getFullYear();
    const month =
      time.getMonth() < 9 ? `0${time.getMonth() + 1}` : time.getMonth() + 1;
    const date = time.getDate() < 10 ? `0${time.getDate()}` : time.getDate();

    this.exportLogFormGroup = this.fb.group({
      type: new FormControl([DataMap.Export_Query_Type.log.value], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      taskID: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.UUID,
            false,
            'invalidInput'
          )
        ]
      }),
      clusterNodeName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }),
      controllerName: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      componentName: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      selectTimeRangeType: new FormControl(this.selectTimeRangeType.custom),
      customRangeDate: new FormControl(
        {
          value: [],
          disabled: true
        },
        {
          validators: [this.checkDateValid()]
        }
      ),
      specifyDay: new FormControl(
        {
          value: 1,
          disabled: true
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 10000)
          ]
        }
      ),
      exportTimeType: new FormControl(DataMap.exportLogRange.all.value, {
        validators: [this.baseUtilService.VALID.required()]
      }), // 接口默认不下发timeRange，此时等价于恢复全部日志
      logName: new FormControl(`Logfile_${year}${month}${date}_${now()}`, {
        validators: [
          this.baseUtilService.VALID.name(
            this.i18n.isEn
              ? CommonConsts.REGEX.dataBaseName
              : CommonConsts.REGEX.nameCombination,
            true,
            'invalidNameCombination'
          ),
          this.baseUtilService.VALID.maxLength(251)
        ]
      }),
      configName: new FormControl(`Configfile_${year}${month}${date}_${now()}`)
    });

    this.listenForm();
  }

  checkDateValid() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (some(control.value, isNil) || isEmpty(control.value)) {
        return { required: { value: control.value } };
      }
      return null;
    };
  }

  listenForm() {
    // 监听导出内容值变化
    this.listenType();
    this.listenExportRange();
    // 根据节点数据获取控制器下拉列表
    this.exportLogFormGroup
      .get('clusterNodeName')
      .valueChanges.subscribe(res => {
        this.getController(res);
        this.exportLogFormGroup.get('controllerName').setValue([]);
        this.exportLogFormGroup.get('componentName').setValue([]);
      });

    // 根据控制器数据获取模块下拉列表
    this.exportLogFormGroup
      .get('controllerName')
      .valueChanges.subscribe(res => {
        this.exportLogFormGroup.get('componentName').setValue([]);
        let components = [];
        each(res, v => {
          const destNode = this.cotrollerNames.find(node => {
            return node.nodeName === v;
          });

          if (!destNode) {
            return;
          }
          components = union(components, destNode.componentList);
        });

        this.componentOptions = this.components.filter(component => {
          return includes(uniq(components), component.value);
        });
      });
  }

  listenType() {
    this.exportLogFormGroup.get('type').valueChanges.subscribe(res => {
      const validName = [
        this.baseUtilService.VALID.name(
          this.i18n.isEn
            ? CommonConsts.REGEX.dataBaseName
            : CommonConsts.REGEX.nameCombination,
          true,
          'invalidNameCombination'
        ),
        this.baseUtilService.VALID.maxLength(251)
      ];
      if (includes(res, DataMap.Export_Query_Type.log.value)) {
        this.exportLogFormGroup.get('logName').setValidators(validName);
        this.exportLogFormGroup
          .get('controllerName')
          .setValidators([this.baseUtilService.VALID.minLength(1)]);
        this.exportLogFormGroup
          .get('componentName')
          .setValidators([this.baseUtilService.VALID.minLength(1)]);
        this.exportLogFormGroup
          .get('exportTimeType')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.enableFormGroupByTimeType(
          this.exportLogFormGroup.get('exportTimeType').value
        );
      }
      if (!includes(res, DataMap.Export_Query_Type.log.value)) {
        this.exportLogFormGroup.get('logName').clearValidators();
        this.exportLogFormGroup.get('controllerName').clearValidators();
        this.exportLogFormGroup.get('componentName').clearValidators();
        this.exportLogFormGroup.get('exportTimeType').clearValidators();
        this.exportLogFormGroup.get('customRangeDate').disable();
        this.exportLogFormGroup.get('specifyDay').disable();
      }

      if (includes(res, DataMap.Export_Query_Type.config.value)) {
        this.exportLogFormGroup.get('configName').setValidators(validName);
      }
      if (!includes(res, DataMap.Export_Query_Type.config.value)) {
        this.exportLogFormGroup.get('configName').clearValidators();
      }
      this.exportLogFormGroup
        .get('controllerName')
        .updateValueAndValidity({ emitEvent: false });
      this.exportLogFormGroup.get('componentName').updateValueAndValidity();
      this.exportLogFormGroup.get('exportTimeType').updateValueAndValidity();
      this.exportLogFormGroup.get('configName').updateValueAndValidity();
    });
  }

  listenExportRange() {
    this.exportLogFormGroup
      .get('exportTimeType')
      .valueChanges.subscribe(res => {
        if (
          !includes(
            this.exportLogFormGroup.get('type').value,
            DataMap.Export_Query_Type.log.value
          )
        ) {
          return;
        }
        this.enableFormGroupByTimeType(res);
      });

    this.exportLogFormGroup
      .get('selectTimeRangeType')
      .valueChanges.subscribe(res => {
        this.selectTimeRangeTypeChange(res);
      });
  }

  private selectTimeRangeTypeChange(res) {
    if (res === this.selectTimeRangeType.custom) {
      this.exportLogFormGroup.get('customRangeDate').enable();
      this.exportLogFormGroup.get('specifyDay').disable();
    } else {
      this.exportLogFormGroup.get('specifyDay').enable();
      this.exportLogFormGroup.get('customRangeDate').disable();
    }
  }

  private enableFormGroupByTimeType(res) {
    if (res === DataMap.exportLogRange.customRange.value) {
      this.selectTimeRangeTypeChange(
        this.exportLogFormGroup.get('selectTimeRangeType').value
      );
    } else {
      this.exportLogFormGroup.get('specifyDay').disable();
      this.exportLogFormGroup.get('customRangeDate').disable();
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'esn',
        name: 'esn',
        hidden: true
      },
      {
        key: 'nodeName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'logLevel',
        name: this.i18n.get('system_log_level_label'),
        cellRender: this.logLevelTpl
      },
      {
        key: 'nodeIp',
        name: 'IP'
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'esn',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        autoPolling: CommonConsts.TIME_INTERVAL,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getDebugLog(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.cdr.detectChanges();
        },
        trackByFn: (index, item) => {
          return item.esn;
        }
      }
    };
  }

  settingAll() {
    this.visible = true;
  }

  cancel() {
    this.visible = false;
  }

  confirm() {
    if (size(this.selectionData) === 1) {
      this.settingLog(this.settingVal, 'single', this.selectionData[0]?.esn);
    } else {
      this.settingLog(this.settingVal, 'multiple');
    }

    this.visible = false;
  }

  logChange(val, esn) {
    this.settingLog(val, 'single', esn);
  }

  settingLog(val, type, esn?) {
    if (val !== DataMap.Log_Level.debug.value) {
      this.setLogApi(val, type, esn);
    } else {
      this.infoMessageService.create({
        content: this.i18n.get('system_debug_log_info_label', [
          this.dataMapService.getLabel(
            'Log_Level',
            DataMap.Log_Level.debug.value
          )
        ]),
        onOK: () => {
          this.setLogApi(val, type, esn);
        },
        onCancel: () => {
          const filter = {
            paginator: {
              pageIndex: 0,
              pageSize: 20
            }
          };
          const args = {
            akLoading: false
          };
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.getDebugLog(filter, args);
        }
      });
    }
  }

  // 查询表格信息
  getDebugLog(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.nodeName) {
        assign(params, {
          nodeName: conditionsTemp.nodeName
        });
      }
    }
    this.apiLogApiService.getLevelInfoList(params).subscribe(res => {
      const newArr = [];
      map(res.records, item => {
        if (item.logLevel === '') {
          newArr.push(Object.assign(item, { disabled: true }));
        } else {
          newArr.push(Object.assign(item, { disabled: false }));
        }
      });
      this.tableData = {
        data: newArr || [],
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  // 修改日志级别保存接口
  setLogApi(val, type, esn?) {
    if (type === 'single') {
      this.logManagerApiService
        .setLogLevel({
          body: { log_level: val },
          akDoException: false,
          akLoading: false,
          memberEsn: esn
        })
        .subscribe(res => {
          const filter = {
            paginator: {
              pageIndex: 0,
              pageSize: 20
            }
          };
          const args = {
            isAutoPolling: false
          };
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.getDebugLog(filter, args);
        });
    } else {
      this.batchOperateService.selfGetResults(
        item => {
          return this.logManagerApiService.setLogLevel({
            body: { log_level: val },
            akDoException: false,
            akLoading: false,
            memberEsn: item.esn
          });
        },
        map(cloneDeep(this.selectionData), item => {
          return assign(item, {
            name: item.nodeName,
            isAsyn: false
          });
        }),
        () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      );
    }
  }

  // 日志导出--节点数据查询
  getNodes() {
    this.backupClustersApiService.queryAllMembers({}).subscribe(res => {
      this.nodeNameOptions = map(res, (v: any) => {
        return {
          disabled: v.status !== DataMap.Cluster_Status.online.value,
          isLeaf: true,
          label: v.clusterName,
          value: v.remoteEsn,
          ...v
        };
      });
    });
  }

  // 日志导出--控制器数据查询
  getController(esn) {
    this.controllerLoading = true;
    this.logManagerApiService
      .collectNodeInfo({ akLoading: false, memberEsn: esn })
      .subscribe({
        next: res => {
          this.controllerLoading = false;
          this.cotrollerNames = res.data;
          this.controllerNameOptions =
            map(res.data, (v: any) => {
              v.isLeaf = true;
              v.label = v.nodeName;
              return v;
            }) || [];
        },
        error: err => {
          this.controllerLoading = false;
        }
      });
  }

  // 导出
  export() {
    const isSpecifyDay =
      this.exportLogFormGroup.get('exportTimeType').value ===
        DataMap.exportLogRange.customRange.value &&
      this.exportLogFormGroup.get('selectTimeRangeType').value ===
        this.selectTimeRangeType.day;
    const isCustom =
      this.exportLogFormGroup.get('exportTimeType').value ===
        DataMap.exportLogRange.customRange.value &&
      this.exportLogFormGroup.get('selectTimeRangeType').value ===
        this.selectTimeRangeType.custom;
    if (
      includes(
        this.exportLogFormGroup.value.type,
        DataMap.Export_Query_Type.log.value
      )
    ) {
      const params = {
        request: {
          params: {
            nodeName: this.exportLogFormGroup.get('controllerName').value,
            componentName: this.exportLogFormGroup.get('componentName').value,
            taskId: this.exportLogFormGroup.get('taskID').value,
            timeRange: isSpecifyDay
              ? toNumber(this.exportLogFormGroup.get('specifyDay').value)
              : DataMap.exportLogRange.all.value
          },
          type: DataMap.Export_Query_Type.log.value,
          name: this.exportLogFormGroup.get('logName').value
        },
        akOperationTips: false,
        memberEsn: this.exportLogFormGroup.value.clusterNodeName
      };
      if (isCustom) {
        const rangeDate = this.exportLogFormGroup.get('customRangeDate').value;
        delete params.request.params.timeRange;
        set(
          params,
          'request.params.startTimestamp',
          Math.floor(new Date(rangeDate[0]).getTime() / 1e3)
        );
        set(
          params,
          'request.params.endTimestamp',
          Math.floor(new Date(rangeDate[1]).getTime() / 1e3)
        );
      }
      this.dealExport(params);
    }

    if (
      includes(
        this.exportLogFormGroup.value.type,
        DataMap.Export_Query_Type.config.value
      )
    ) {
      this.dealExport({
        request: {
          type: DataMap.Export_Query_Type.config.value,
          name: this.exportLogFormGroup.get('configName').value
        },
        akOperationTips: false,
        memberEsn: this.exportLogFormGroup.value.clusterNodeName
      });
    }
  }

  dealExport(params) {
    this.exportFilesApi.CreateExportFile(params).subscribe(res => {
      this.message.success(this.i18n.get('common_export_files_result_label'), {
        lvShowCloseButton: true,
        lvDuration: this.isHyperdetect ? 10 * 1e3 : 60 * 1e3,
        lvOnShow: () => {
          this.dealResult();
        }
      });
    });
  }

  dealResult() {
    const exportFilesResult = document.getElementsByClassName(
      'export-files-result'
    );
    if (exportFilesResult) {
      each(exportFilesResult, item => {
        item.addEventListener('click', () => {
          this.router.navigate(['system/export-query']);
        });
      });
    }
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.InsightJobs);
  }
}
