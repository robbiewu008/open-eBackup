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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild,
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
  ApiExportFilesApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  RouterUrl
} from 'app/shared';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  each,
  includes,
  isEmpty,
  isNil,
  map,
  now,
  set,
  some,
  toNumber,
  union,
  uniq
} from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-debug-log',
  templateUrl: './debug-log.component.html',
  styleUrls: ['./debug-log.component.less'],
  encapsulation: ViewEncapsulation.None
})
export class DebugLogComponent implements OnInit, OnDestroy {
  includes = includes;
  dataMap = DataMap;
  debugLogFormGroup: FormGroup;
  exportLogFormGroup: FormGroup;
  originalLoglevel;
  nodeNames = [];
  nodeNameOptions = [];
  componentOptions = [];
  periodOptions = this.dataMapService.toArray('exportLogRange');
  debugLogDisable = true;
  components =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
      ? this.dataMapService
          .toArray('cyberComponentName')
          .filter(v => (v.isLeaf = true))
      : this.dataMapService
          .toArray('Component_Name')
          .filter(v => (v.isLeaf = true));
  levelOptions = this.dataMapService
    .toArray('Log_Level')
    .filter(v => (v.isLeaf = true));
  timeSub$: Subscription;
  destroy$ = new Subject();
  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;
  fileDownloadProcessingLabel = this.i18n.get(
    'common_file_download_processing_label'
  );
  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );
  fileDownloadErrorLabel = this.i18n.get('common_file_download_error_label');
  isDecouple = this.appUtilsService.isDecouple;
  isDistributed = this.appUtilsService.isDistributed;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  headerLabel = this.i18n.get('system_export_log_label');
  descLabel = this.i18n.get('system_export_only_log_desc_label');

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.e6000.value,
      DataMap.Deploy_Type.decouple.value,
      DataMap.Deploy_Type.openServer.value
    ],
    this.i18n.get('deploy_type')
  );

  time = new Date();
  year = this.time.getFullYear();
  month =
    this.time.getMonth() < 9
      ? `0${this.time.getMonth() + 1}`
      : this.time.getMonth() + 1;
  date =
    this.time.getDate() < 10 ? `0${this.time.getDate()}` : this.time.getDate();
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
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private infoMessageService: InfoMessageService,
    private logManagerApiService: LogManagerApiService,
    public appUtilsService: AppUtilsService,
    private exportFilesApi?: ApiExportFilesApiService,
    private message?: MessageService,
    public router?: Router
  ) {}

  ngOnInit() {
    this.headerLabel = this.appUtilsService.isDecouple
      ? this.i18n.get('system_export_export_info_label')
      : this.i18n.get('system_export_log_label');
    this.descLabel = this.appUtilsService.isDistributed
      ? this.i18n.get('system_export_log_desc_label')
      : this.i18n.get('system_export_only_log_desc_label');
    this.initForm();
    this.getDebugLog();
    this.getNodeNames();
  }

  onChange() {
    this.ngOnInit();
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  initForm() {
    this.debugLogFormGroup = this.fb.group({
      log_level: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });

    this.debugLogFormGroup.get('log_level').valueChanges.subscribe(res => {
      this.debugLogDisable = res === this.originalLoglevel;
    });

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
      nodeName: new FormControl([], {
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
      logName: new FormControl(
        `Logfile_${this.year}${this.month}${this.date}_${now()}`,
        {
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
        }
      )
    });

    this.listenType();
    this.listenExportRange();
    this.exportLogFormGroup
      .get('nodeName')
      .valueChanges.subscribe(nodeNames => {
        let components = [];
        each(nodeNames, nodeName => {
          const destNode = this.nodeNames.find(node => {
            return node.nodeName === nodeName;
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

  checkDateValid() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (some(control.value, isNil) || isEmpty(control.value)) {
        return { required: { value: control.value } };
      }
      return null;
    };
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
        this.exportLogFormGroup.get('componentName').clearValidators();
        this.exportLogFormGroup.get('exportTimeType').clearValidators();
        this.exportLogFormGroup.get('customRangeDate').disable();
        this.exportLogFormGroup.get('specifyDay').disable();
      }

      if (includes(res, DataMap.Export_Query_Type.config.value)) {
        this.exportLogFormGroup.addControl(
          'configName',
          new FormControl(
            `Configfile_${this.year}${this.month}${this.date}_${now()}`
          )
        );
        this.exportLogFormGroup.get('configName').setValidators(validName);
      }
      if (!includes(res, DataMap.Export_Query_Type.config.value)) {
        this.exportLogFormGroup.get('configName').clearValidators();
      }
      this.exportLogFormGroup.get('logName').updateValueAndValidity();
      this.exportLogFormGroup.get('componentName').updateValueAndValidity();
      this.exportLogFormGroup.get('exportTimeType').updateValueAndValidity();
      this.exportLogFormGroup.get('logName').updateValueAndValidity();
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

  getDebugLog() {
    this.logManagerApiService.queryCurrentLevel({}).subscribe(res => {
      this.originalLoglevel = res.data.logLevel;
      this.debugLogFormGroup.get('log_level').setValue(res.data.logLevel);
    });
  }

  getNodeNames() {
    this.logManagerApiService.collectNodeInfo({}).subscribe(res => {
      this.nodeNames = res.data;
      this.nodeNameOptions =
        map(res.data, (v: any) => {
          v.isLeaf = true;
          v.label = v.nodeName;
          if (this.isDistributed || this.isDecouple) {
            v.label = v.hostname;
            v.disabled = v.nodeStatus !== 'Ready';
          }
          return v;
        }) || [];
    });
  }

  ok() {
    if (
      this.debugLogFormGroup.value.log_level !== DataMap.Log_Level.debug.value
    ) {
      this.logManagerApiService
        .setLogLevel({ body: this.debugLogFormGroup.value })
        .subscribe(res => this.getDebugLog());
    } else {
      this.infoMessageService.create({
        content: this.i18n.get('system_debug_log_info_label', [
          this.dataMapService.getLabel(
            'Log_Level',
            DataMap.Log_Level.debug.value
          )
        ]),
        onOK: () => {
          this.logManagerApiService
            .setLogLevel({ body: this.debugLogFormGroup.value })
            .subscribe(res => this.getDebugLog());
        },
        onCancel: () => this.getDebugLog()
      });
    }
  }

  export(exportParams?) {
    let isSpecifyDay = false;
    let isCustom = false;
    if (
      this.exportLogFormGroup.get('exportTimeType').value ===
      DataMap.exportLogRange.customRange.value
    ) {
      if (
        this.exportLogFormGroup.get('selectTimeRangeType').value ===
        this.selectTimeRangeType.day
      ) {
        isSpecifyDay = true;
      } else if (
        this.exportLogFormGroup.get('selectTimeRangeType').value ===
        this.selectTimeRangeType.custom
      ) {
        isCustom = true;
      }
    }
    if (
      includes(
        this.exportLogFormGroup.value.type,
        DataMap.Export_Query_Type.log.value
      )
    ) {
      const params = {
        request: {
          params: {
            nodeName: this.exportLogFormGroup.get('nodeName').value,
            componentName: this.exportLogFormGroup.get('componentName').value,
            taskId: this.exportLogFormGroup.get('taskID').value,
            timeRange: isSpecifyDay
              ? toNumber(this.exportLogFormGroup.get('specifyDay').value)
              : DataMap.exportLogRange.all.value
          },
          type: DataMap.Export_Query_Type.log.value,
          name: this.exportLogFormGroup.get('logName').value
        },
        akOperationTips: false
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
          params: {
            nodeName: this.exportLogFormGroup.get('nodeName').value
          },
          type: DataMap.Export_Query_Type.config.value,
          name: this.exportLogFormGroup.get('configName').value
        },
        akOperationTips: false
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
