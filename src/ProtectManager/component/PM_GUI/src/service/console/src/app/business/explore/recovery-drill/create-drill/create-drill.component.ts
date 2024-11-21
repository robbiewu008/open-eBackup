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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import { ActivatedRoute, Router } from '@angular/router';
import { MessageService, OptionItem } from '@iux/live';
import {
  ApiService,
  BaseUtilService,
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  DrillExecuteType,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  RestoreType
} from 'app/shared';
import { CreateExerciseRequest } from 'app/shared/api/models';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import {
  RestoreParams,
  RestoreService
} from 'app/shared/services/restore.service';
import {
  assign,
  each,
  find,
  first,
  includes,
  isEmpty,
  map,
  reject,
  size,
  some,
  unionBy
} from 'lodash';
import { AddDrillResourceComponent } from './add-drill-resource/add-drill-resource.component';
import { AddScriptComponent } from './add-script/add-script.component';
import { DatePipe } from '@angular/common';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';

@Component({
  selector: 'aui-create-drill',
  templateUrl: './create-drill.component.html',
  styleUrls: ['./create-drill.component.less'],
  providers: [DatePipe]
})
export class CreateDrillComponent implements OnInit {
  activeIndex = 0;

  resourceTableConfig: TableConfig;
  resourceTableData: TableData;
  selectionData = [];

  formGroup: FormGroup;
  dataMap = DataMap;
  _find = find;
  drillExecuteType = DrillExecuteType;
  _isEn = this.i18n.isEn;

  configTableData = [];
  selectionConfigData = [];
  sizeOptions = [CommonConsts.PAGE_SIZE_SMALL];
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  pageIndex = 0;

  nextBtnDisabled = true;

  timeErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invaildTime: this.i18n.get('common_valid_executiontime_label')
  };

  // 支持挂载应用
  mountConfigShow = false;
  mountOptions = this.dataMapService
    .toArray('drillMountConfig')
    .filter(item => {
      return (item.isLeaf = true);
    });

  isModify = !isEmpty(this.route?.snapshot?.params?.uuid);
  drillPlan;

  durationUnitOptions = this.dataMapService
    .toArray('recoveryDrillUnit')
    .filter(item => {
      return (item.isLeaf = true);
    });
  mountTypeOptions: OptionItem[] = this.dataMapService
    .toArray('recoveryDrillType')
    .map(item => {
      item.isLeaf = true;
      return item;
    });

  maxHour = 23;
  maxDay = 365;
  maxMonth = 24;
  maxYear = 10;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  retentionDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, this.maxHour])
  });

  @ViewChild('resourceTable', { static: false })
  resourceTable: ProTableComponent;
  @ViewChild('resourceTypeTpl', { static: true }) resourceTypeTpl: TemplateRef<
    any
  >;

  constructor(
    private router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private route: ActivatedRoute,
    private message: MessageService,
    private exerciseService: ApiService,
    private restoreService: RestoreService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private manualMountService: ManualMountService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private systemTimeService?: SystemTimeService
  ) {}

  ngOnInit() {
    this.initTableConfig();
    this.initForm();
    this.getDrill();
    this.getDrillDetail();
  }

  getDrill() {
    if (!this.isModify) {
      return;
    }
    this.exerciseService
      .queryExercise({
        pageNo: 0,
        pageSize: 1,
        conditions: JSON.stringify({
          uuid: this.route?.snapshot?.params?.uuid
        })
      })
      .subscribe(res => {
        this.drillPlan = first(res.records);
        this.formGroup.patchValue({
          type: this.drillPlan.type,
          name: this.drillPlan.name,
          executeType: this.drillPlan.startTime
            ? DrillExecuteType.Specified
            : DrillExecuteType.Immediately,
          executeTime: this.drillPlan.startTime
            ? new Date(this.drillPlan.startTime)
            : null,
          retentionDuration: this.drillPlan.interval,
          durationUnit:
            this.drillPlan.intervalUnit ?? DataMap.recoveryDrillUnit.hour.value,
          startTime: this.drillPlan.startTime
            ? new Date(this.drillPlan.startTime)
            : null
        });
        if (this.drillPlan.type === DataMap.drillType.single.value) {
          this.formGroup.get('retentionDuration').clearValidators();
          this.formGroup.get('retentionDuration').updateValueAndValidity();
        }
        this.nextBtnDisabled = false;
      });
  }

  patchResource(item) {
    const resource = {
      name: item.resourceName,
      uuid: item.resourceId,
      subType: item.resourceType,
      path: item.location,
      recoveryTargetLocation: item.targetLocation
    };
    if (this.isSupportMount(item.resourceType)) {
      assign(resource, {
        mountConfig: item.mountConfig ?? true,
        mountType: item.type ?? DataMap.recoveryDrillType.liveMount.value
      });
    }
    // 演练参数回显
    if (item.subTaskParam) {
      const subTaskParam = JSON.parse(item.subTaskParam);
      assign(resource, {
        copyConfig: subTaskParam.copyId || subTaskParam.copy_id,
        recoveryConfig: { ...subTaskParam },
        mountConfig: item.shouldDestroy ?? true
      });
    }
    // 脚本回显
    if (
      !isEmpty(item.preScript) ||
      !isEmpty(item.postScript) ||
      !isEmpty(item.executeScript)
    ) {
      assign(resource, {
        scriptConfig: {
          preScript: item.preScript,
          postScript: item.postScript,
          executeScript: item.executeScript
        }
      });
    }
    return resource;
  }

  getDrillDetail() {
    if (!this.isModify) {
      return;
    }
    this.exerciseService
      .queryExerciseResourceDetail({
        exerciseId: this.route?.snapshot?.params?.uuid
      })
      .subscribe((res: any) => {
        const resourceData = [];
        each(res, item => {
          this.protectedResourceApiService
            .ShowResource({ resourceId: item.resourceId, akDoException: false })
            .subscribe({
              next: detail => {
                resourceData.push({ ...this.patchResource(item), ...detail });
                this.resourceTableData = {
                  data: [...resourceData],
                  total: size(resourceData)
                };
                this.configTableData = [...resourceData];
                this.formGroup.updateValueAndValidity();
              },
              error: () => {
                resourceData.push(this.patchResource(item));
                this.resourceTableData = {
                  data: [...resourceData],
                  total: size(resourceData)
                };
                this.configTableData = [...resourceData];
                this.formGroup.updateValueAndValidity();
              }
            });
        });
        this.nextBtnDisabled = false;
      });
  }

  initTableConfig() {
    this.resourceTableConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'type',
            name: this.i18n.get('common_resource_type_label'),
            cellRender: this.resourceTypeTpl
          },
          {
            key: 'path',
            name: this.i18n.get('common_location_label')
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 2,
                items: [
                  {
                    id: 'delete',
                    permission: OperateItems.CreateDrillPlan,
                    label: this.i18n.get('common_delete_label'),
                    onClick: ([data]) => this.deleteResource(data)
                  }
                ]
              }
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        pageSize: 10
      }
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.drillType.period.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      retentionDuration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxHour)
        ]
      }),
      durationUnit: new FormControl(DataMap.recoveryDrillUnit.hour.value),
      startTime: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      executeType: new FormControl(DrillExecuteType.Immediately),
      executeTime: new FormControl('')
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.drillType.period.value) {
        this.validRetentionDuration(this.formGroup.value.durationUnit);
        this.formGroup
          .get('startTime')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('executeTime').clearValidators();
        this.formGroup.get('executeTime').clearAsyncValidators();
      } else {
        this.formGroup.get('retentionDuration').clearValidators();
        this.formGroup.get('startTime').clearValidators();
        if (this.formGroup.value.executeType === DrillExecuteType.Immediately) {
          this.formGroup.get('executeTime').clearValidators();
          this.formGroup.get('executeTime').clearAsyncValidators();
        } else {
          this.formGroup
            .get('executeTime')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('executeTime')
            .setAsyncValidators([this.validTimeRange()]);
        }
      }
      this.formGroup.get('retentionDuration').updateValueAndValidity();
      this.formGroup.get('executeTime').updateValueAndValidity();
      this.formGroup.get('startTime').updateValueAndValidity();
    });

    this.formGroup
      .get('durationUnit')
      .valueChanges.subscribe(res => this.validRetentionDuration(res));

    this.formGroup.get('executeType').valueChanges.subscribe(res => {
      if (
        res === DrillExecuteType.Specified &&
        this.formGroup.value.type === DataMap.drillType.single.value
      ) {
        this.formGroup
          .get('executeTime')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('executeTime')
          .setAsyncValidators([this.validTimeRange()]);
      } else {
        this.formGroup.get('executeTime').clearValidators();
        this.formGroup.get('executeTime').clearAsyncValidators();
      }
      this.formGroup.get('executeTime').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(() => this.disableNextBtn());
  }

  validTimeRange() {
    return (control: AbstractControl): Promise<{ [key: string]: any }> => {
      return new Promise(resolve => {
        this.vertifyTime(control, resolve);
      });
    };
  }

  vertifyTime(control, resolve) {
    this.systemTimeService.getSystemTime(false).subscribe(value => {
      const now = new Date(value.time).getTime();
      const selectTime = control?.value?.getTime();
      if (selectTime < now) {
        resolve({ invaildTime: { value: control.value } });
      }
      resolve(null);
    });
  }

  validRetentionDuration(type) {
    if (type === DataMap.recoveryDrillUnit.hour.value) {
      this.formGroup
        .get('retentionDuration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxHour)
        ]);
      assign(this.retentionDurationErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, this.maxHour])
      });
    } else if (type === DataMap.recoveryDrillUnit.day.value) {
      this.formGroup
        .get('retentionDuration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxDay)
        ]);
      assign(this.retentionDurationErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, this.maxDay])
      });
    } else if (type === DataMap.recoveryDrillUnit.month.value) {
      this.formGroup
        .get('retentionDuration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxMonth)
        ]);
      assign(this.retentionDurationErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [
          1,
          this.maxMonth
        ])
      });
    } else {
      this.formGroup
        .get('retentionDuration')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxYear)
        ]);
      assign(this.retentionDurationErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, this.maxYear])
      });
    }
    this.formGroup.get('retentionDuration').updateValueAndValidity();
  }

  // 判断副本是否存在
  isCopyExist(resource): boolean {
    if (this.formGroup.value.type === DataMap.drillType.period.value) {
      return true;
    }
    return !isEmpty(find(resource.copyOptions, { value: resource.copyConfig }));
  }

  validDrillResource(resource): boolean {
    if (this.formGroup.value.type === DataMap.drillType.period.value) {
      return !isEmpty(resource.recoveryConfig);
    }
    return (
      !isEmpty(resource.recoveryConfig) &&
      !isEmpty(resource.copyConfig) &&
      this.isCopyExist(resource)
    );
  }

  disableNextBtn() {
    this.nextBtnDisabled =
      this.activeIndex === 0
        ? isEmpty(this.configTableData)
        : this.formGroup.invalid ||
          isEmpty(this.configTableData) ||
          some(this.configTableData, item => !this.validDrillResource(item));
  }

  gotoRecoveryDrill() {
    this.router.navigateByUrl('/explore/recovery-drill');
  }

  isSupportMount(subType: string) {
    return includes(
      [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value,
        DataMap.Resource_Type.MySQLInstance.value,
        DataMap.Resource_Type.tdsqlInstance.value
      ],
      subType
    );
  }

  isUseLivemountType(item): boolean {
    // 不支持及时挂载的应用
    if (!this.isSupportMount(item.subType)) {
      return false;
    }
    if (this.formGroup.value.type === DataMap.drillType.period.value) {
      return item.mountType === DataMap.recoveryDrillType.liveMount.value;
    }
    // 日志副本不能使用及时挂载演练
    return !(
      item.copyConfig &&
      item.copyOptions &&
      find(item.copyOptions, { value: item.copyConfig })?.source_copy_type ===
        DataMap.CopyData_Backup_Type.log.value
    );
  }

  getResourceType(resource): string {
    return this.appUtilsService.getResourceType(resource.subType);
  }

  getResourceEnv(existData) {
    // 查询环境信息
    each(existData, item => {
      if (isEmpty(item.environment)) {
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.uuid,
            akDoException: false
          })
          .subscribe(res => {
            assign(item, {
              environment: res?.environment,
              version: res?.version,
              parentName: res?.parentName,
              parentUuid: res?.parentUuid,
              extendInfo: res?.extendInfo
            });
            this.resourceTableData = {
              data: existData,
              total: size(existData)
            };
            this.configTableData = [...existData];
          });
      }
    });
  }

  addResource() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-drill-resource-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_add_label'),
        lvContent: AddDrillResourceComponent,
        lvOkDisabled: true,
        lvComponentParams: {},
        lvOk: modal => {
          const content = modal.getContentComponent() as AddDrillResourceComponent;
          let existData = this.resourceTableData?.data || [];
          existData = unionBy(existData, content.selectionData, 'uuid');
          // 最大支持选择5个资源
          if (size(existData) > 5) {
            this.message.error(
              this.i18n.get('explore_drill_max_resource_label')
            );
            return false;
          }
          // 支持及时挂载的应用
          this.mountConfigShow = !isEmpty(
            find(existData, item => this.isSupportMount(item.subType))
          );
          // 默认演练后销毁
          each(existData, item => {
            if (this.isSupportMount(item.subType)) {
              assign(item, {
                mountType: DataMap.recoveryDrillType.liveMount.value,
                mountConfig: true
              });
            }
          });
          this.getResourceEnv(existData);
          this.resourceTableData = {
            data: existData,
            total: size(existData)
          };
          this.configTableData = [...existData];
          this.nextBtnDisabled = isEmpty(this.resourceTableData?.data);
        }
      })
    );
  }

  deleteResource(resource?) {
    const leftData = reject(this.resourceTableData?.data, item => {
      return !isEmpty(resource)
        ? resource.uuid === item.uuid
        : includes(map(this.selectionData, 'uuid'), item.uuid);
    });
    this.resourceTableData = {
      data: leftData,
      total: size(leftData)
    };
    this.configTableData = [...leftData];
    this.selectionData = [];
    this.resourceTable.setSelections([]);
    this.disableNextBtn();
  }

  getTargetLocation(item, resourceId) {
    if (!resourceId) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId, akDoException: false })
      .subscribe((res: any) => {
        assign(item, {
          recoveryTargetLocation: `${res.name}(${res.path ||
            res.endpoint ||
            ''})`.replace('()', '')
        });
      });
  }

  setRecovery(item) {
    if (
      this.formGroup.value.type === DataMap.drillType.single.value &&
      !item.copyConfig
    ) {
      return;
    }

    const copy =
      this.formGroup.value.type === DataMap.drillType.single.value
        ? find(item.copyOptions, { value: item.copyConfig })
        : first(item.copyOptions) || {
            uuid: '',
            resource_name: item.name,
            resource_id: item.uuid,
            resource_sub_type: item.subType,
            resource_properties: JSON.stringify(item),
            properties: JSON.stringify({})
          };
    if (!isEmpty(item.recoveryConfig)) {
      assign(copy, {
        drillRecoveryConfig: item.recoveryConfig
      });
    }

    if (this.isUseLivemountType(item)) {
      this.manualMountService.create(
        {
          item: copy,
          resType: item.subType,
          onOk: () => {},
          warning: true
        },
        (mountParams, targetLocation) => {
          assign(item, {
            recoveryConfig: mountParams,
            recoveryTargetLocation: targetLocation
          });
          this.disableNextBtn();
        }
      );
    } else {
      const params: RestoreParams = {
        childResType: item.subType,
        copyData: {
          ...copy,
          resource_status: DataMap.Resource_Status.notExist.value,
          generated_by: DataMap.CopyData_generatedType.replicate.value
        }
      };
      // AD域适配
      if (
        includes([DataMap.Resource_Type.ActiveDirectory.value], item.subType)
      ) {
        assign(params, {
          restoreType: RestoreType.CommonRestore,
          copyData: { ...copy }
        });
      }
      // 通用数据库适配
      if (
        includes([DataMap.Resource_Type.generalDatabase.value], item.subType)
      ) {
        assign(params, {
          copyData: { ...copy }
        });
      }
      this.restoreService.restore(params, recoveryParams => {
        assign(item, {
          recoveryConfig: recoveryParams
        });
        // 查询恢复目标位置
        this.getTargetLocation(item, recoveryParams?.targetEnv);
        this.disableNextBtn();
      });
    }
  }

  hasNoDestoryMount(): boolean {
    return some(
      this.configTableData,
      item => this.isUseLivemountType(item) && !item.mountConfig
    );
  }

  hasTdsqlRecovery(): boolean {
    return some(
      this.configTableData,
      item =>
        item.subType === DataMap.Resource_Type.tdsqlDistributedInstance.value
    );
  }

  setScript(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'set-script-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('explore_drill_add_script_label'),
        lvContent: AddScriptComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddScriptComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: { item },
        lvOk: modal => {
          const content = modal.getContentComponent() as AddScriptComponent;
          if (
            !content.formGroup.value.preScript &&
            !content.formGroup.value.postScript &&
            !content.formGroup.value.executeScript
          ) {
            delete item.scriptConfig;
            return true;
          }
          assign(item, {
            scriptConfig: {
              preScript: content.formGroup.value.preScript,
              postScript: content.formGroup.value.postScript,
              executeScript: content.formGroup.value.executeScript
            }
          });
        }
      })
    );
  }

  getResourceCopies() {
    each(this.configTableData, item => {
      if (!isEmpty(item.copyOptions)) {
        return;
      }
      this.copiesApiService
        .queryResourcesV1CopiesGet({
          pageNo: CommonConsts.PAGE_START,
          pageSize: 50,
          orders: ['-display_timestamp'],
          conditions: JSON.stringify({
            resource_id: item.uuid,
            generated_by: [
              DataMap.CopyData_generatedType.backup.value,
              DataMap.CopyData_generatedType.replicate.value
            ],
            source_copy_type: [
              DataMap.CopyData_Backup_Type.full.value,
              DataMap.CopyData_Backup_Type.incremental.value,
              DataMap.CopyData_Backup_Type.log.value,
              DataMap.CopyData_Backup_Type.diff.value,
              DataMap.CopyData_Backup_Type.permanent.value
            ],
            status: [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.mounting.value,
              DataMap.copydata_validStatus.unmounting.value,
              DataMap.copydata_validStatus.mounted.value
            ]
          })
        })
        .subscribe(res => {
          assign(item, {
            copyOptions: map(res.items, copy => {
              return {
                ...copy,
                isLeaf: true,
                value: copy.uuid,
                label: this.datePipe.transform(
                  copy.display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                )
              };
            })
          });
          this.disableNextBtn();
        });
    });
  }

  copyChange(item) {
    assign(item, { recoveryConfig: null });
    this.disableNextBtn();
  }

  mountTypeChange(item) {
    assign(item, { recoveryConfig: null });
    this.disableNextBtn();
  }

  next() {
    this.activeIndex++;
    this.disableNextBtn();
    if (this.activeIndex === 1) {
      this.getResourceCopies();
    }
  }

  previous() {
    this.activeIndex--;
    if (this.activeIndex === 0) {
      this.nextBtnDisabled = isEmpty(this.configTableData);
    }
    if (this.activeIndex === 1) {
      this.nextBtnDisabled =
        isEmpty(this.configTableData) || this.formGroup.invalid;
    }
  }

  canel() {
    this.router.navigateByUrl('/explore/recovery-drill');
  }

  getParams() {
    const params: CreateExerciseRequest = {
      name: this.formGroup.value.name,
      type: this.formGroup.value.type
    };
    if (this.isModify) {
      assign(params, {
        exerciseId: this.route?.snapshot?.params?.uuid
      });
    }
    if (this.formGroup.value.type === DataMap.drillType.period.value) {
      assign(params, {
        interval: Number(this.formGroup.value.retentionDuration),
        intervalUnit: this.formGroup.value.durationUnit,
        startTime: new Date(this.formGroup.value.startTime).getTime()
      });
    } else {
      assign(params, {
        startTime:
          this.formGroup.value.executeType === DrillExecuteType.Immediately
            ? null
            : new Date(this.formGroup.value.executeTime).getTime()
      });
    }
    const resources = [];
    each(this.configTableData, item => {
      if (this.isUseLivemountType(item)) {
        assign(item.recoveryConfig?.parameters, {
          preScript: item.scriptConfig?.preScript || '',
          postScript: item.scriptConfig?.postScript || '',
          failPostScript: item.scriptConfig?.executeScript || ''
        });
      } else {
        assign(item.recoveryConfig, {
          scripts: {
            ...item.recoveryConfig?.scripts,
            preScript: item.scriptConfig?.preScript || '',
            postScript: item.scriptConfig?.postScript || '',
            failPostScript: item.scriptConfig?.executeScript || ''
          }
        });
      }
      // tdsql非集中式脚本特殊处理
      if (includes([DataMap.Resource_Type.tdsqlInstance.value], item.subType)) {
        assign(item.recoveryConfig?.parameters, {
          pre_script: item.scriptConfig?.preScript || '',
          post_script: item.scriptConfig?.postScript || '',
          failed_script: item.scriptConfig?.executeScript || ''
        });
      }
      // tdsql集中式脚本特殊处理
      if (
        includes(
          [DataMap.Resource_Type.tdsqlDistributedInstance.value],
          item.subType
        )
      ) {
        assign(item.recoveryConfig?.scripts, {
          pre_script: item.scriptConfig?.preScript || '',
          post_script: item.scriptConfig?.postScript || '',
          failed_script: item.scriptConfig?.executeScript || ''
        });
      }
      const config = {
        copyId:
          this.formGroup.value.type === DataMap.drillType.single.value
            ? item.copyConfig
            : '',
        shouldDestroy: this.isUseLivemountType(item) ? item.mountConfig : false,
        sourceId: item.uuid,
        sourceName: item.name,
        sourceSubType: item.subType,
        type: this.isUseLivemountType(item)
          ? DataMap.recoveryDrillType.liveMount.value
          : DataMap.recoveryDrillType.restore.value,
        subTaskParam: JSON.stringify(item.recoveryConfig),
        location: item.path,
        targetLocation: item.recoveryTargetLocation
      };
      resources.push(config);
    });
    assign(params, { resources });
    return params;
  }

  finish() {
    if (this.isModify) {
      this.exerciseService
        .modifyExercise({
          createExerciseRequest: this.getParams()
        })
        .subscribe(() => this.router.navigateByUrl('/explore/recovery-drill'));
    } else {
      this.exerciseService
        .createExercise({
          CreateExerciseRequest: this.getParams()
        })
        .subscribe(() => this.router.navigateByUrl('/explore/recovery-drill'));
    }
  }
}
