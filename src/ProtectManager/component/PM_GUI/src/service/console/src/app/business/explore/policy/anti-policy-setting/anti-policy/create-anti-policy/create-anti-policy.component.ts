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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageboxService, MessageService, OptionItem } from '@iux/live';
import {
  AntiRansomwarePolicyApiService,
  BaseUtilService,
  CommonConsts,
  CookieService,
  CopiesService,
  CopyControllerService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  extendSummaryCopiesParams,
  GenConditionsService,
  I18NService,
  ProtectedResourceApiService,
  ResourceService,
  WarningMessageService
} from 'app/shared';
import { AntiRansomwarePolicyReq } from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  find,
  includes,
  isEmpty,
  isNil,
  isUndefined,
  map,
  size,
  toString as _toString
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { pairwise } from 'rxjs/operators';
interface ExcludeItem {
  value: string;
  label: string;
}
/**
 *  防勒索、防篡改策略排除的资源类型
 */
export const EXCLUDE_RESOURCE_TYPES: ExcludeItem[] = [
  DataMap.Job_Target_Type.local,
  DataMap.Job_Target_Type.ldap,
  DataMap.Job_Target_Type.ldapGroup,
  DataMap.Job_Target_Type.saml,
  DataMap.Job_Target_Type.ProtectAgent,
  DataMap.Job_Target_Type.DBBackupAgent,
  DataMap.Job_Target_Type.VMBackupAgent,
  DataMap.Job_Target_Type.DWSBackupAgent,
  DataMap.Job_Target_Type.UBackupAgent,
  DataMap.Job_Target_Type.SBackupAgent,
  DataMap.Job_Target_Type.SQLServerCluster,
  DataMap.Job_Target_Type.VMware,
  DataMap.Job_Target_Type.VMwarevCenterServer,
  DataMap.Job_Target_Type.vmwareEsx,
  DataMap.Job_Target_Type.vmwareEsxi,
  DataMap.Job_Target_Type.kubernetes,
  DataMap.Job_Target_Type.OpenGauss,
  DataMap.Job_Target_Type.ImportCopy,
  DataMap.Job_Target_Type.LocalFileSystem,
  DataMap.Job_Target_Type.LocalLun,
  DataMap.Job_Target_Type.OceanStorDorado_6_1_3,
  DataMap.Job_Target_Type.OceanStor_6_1_3,
  DataMap.Job_Target_Type.OceanStor_v5,
  DataMap.Job_Target_Type.OceanStorPacific,
  DataMap.Job_Target_Type.NetApp,
  DataMap.Job_Target_Type.s3Storage,
  DataMap.Job_Target_Type.HCSContainer,
  DataMap.Job_Target_Type.FusionComputePlatform,
  DataMap.Job_Target_Type.HCSTenant,
  DataMap.Job_Target_Type.dwsCluster,
  DataMap.Job_Target_Type.dwsSchema,
  DataMap.Job_Target_Type.dwsTable,
  DataMap.Job_Target_Type.MySQLCluster,
  DataMap.Job_Target_Type.kubernetesNamespace,
  DataMap.Job_Target_Type.kubernetesClusterCommon,
  DataMap.Job_Target_Type.HCSProject,
  DataMap.Job_Target_Type.hcsEnvOp,
  DataMap.Job_Target_Type.FusionComputeCluster,
  DataMap.Job_Target_Type.FusionComputeHost,
  DataMap.Job_Target_Type.FusionOneComputePlatform,
  DataMap.Job_Target_Type.FusionOneComputeCluster,
  DataMap.Job_Target_Type.FusionOneComputeHost,
  DataMap.Job_Target_Type.vmwareHostSystem,
  DataMap.Job_Target_Type.cNware,
  DataMap.Job_Target_Type.cNwareHostPool,
  DataMap.Job_Target_Type.cNwareCluster,
  DataMap.Job_Target_Type.cNwareHost,
  DataMap.Job_Target_Type.clusterComputeResource,
  DataMap.Job_Target_Type.Openstack,
  DataMap.Job_Target_Type.OpenStackProject,
  DataMap.Job_Target_Type.cyberOceanStorPacific,
  DataMap.Job_Target_Type.gaussdbForOpengaussProject,
  DataMap.Job_Target_Type.lightCloudGaussdbProject,
  DataMap.Job_Target_Type.OceanStorDorado,
  DataMap.Job_Target_Type.OceanProtect,
  DataMap.Job_Target_Type.fileSystem,
  DataMap.Job_Target_Type.informixService,
  DataMap.Job_Target_Type.BackupMemberCluster,
  DataMap.Job_Target_Type.tdsqlCluster,
  DataMap.Job_Target_Type.commonShare,
  DataMap.Job_Target_Type.certificate,
  DataMap.Job_Target_Type.drillDatabase,
  DataMap.Job_Target_Type.ndmpServer,
  DataMap.Job_Target_Type.saphanaInstance,
  DataMap.Job_Target_Type.ApsaraStack,
  DataMap.Job_Target_Type.APSResourceSet,
  DataMap.Job_Target_Type.APSZone
];

/**
 *  防勒索支持的资源类型
 */
const ANTI_RESOURCE_TYPES = [
  DataMap.Resource_Type.NASFileSystem,
  DataMap.Resource_Type.virtualMachine,
  DataMap.Resource_Type.NASShare,
  DataMap.Resource_Type.fileset,
  DataMap.Resource_Type.cNwareVm,
  DataMap.Resource_Type.FusionCompute,
  DataMap.Resource_Type.fusionOne,
  DataMap.Resource_Type.HCSCloudHost,
  DataMap.Resource_Type.openStackCloudServer,
  DataMap.Resource_Type.hyperVVm
];

@Component({
  selector: 'aui-create-anti-policy',
  templateUrl: './create-anti-policy.component.html',
  styleUrls: ['./create-anti-policy.component.less'],
  providers: [DatePipe]
})
export class CreateAntiPolicyComponent implements OnInit {
  data;
  dataMap = DataMap;
  activeIndex = 'total';
  currentTotal = 0;
  currentSelect = 0;
  tableData: TableData;
  tableConfig: TableConfig;
  selectedTableData: TableData;
  selectedTableConfig: TableConfig;
  formGroup: FormGroup;
  resourceTypes = [];
  antiTamperingSettingOptions = [];
  antiTamperingSetting;
  antiPolicySwitchDisabled = false;
  isInit = false;
  isRecover = false;
  intervalUnit: OptionItem[];
  language = this.i18n.language;
  resourceChangeTimer;
  resourceCheckFaild: boolean;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  typeChangeLabel = '';
  intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
  });
  backupGenerationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  });
  firstTimeErrorTip = assign({}, this.baseUtilService.requiredErrorTip);
  specifiedTimeErrorTip = assign({}, this.baseUtilService.requiredErrorTip);
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  @ViewChild('resourceTable', { static: false })
  resourceTable: ProTableComponent;
  @ViewChild('seletedTable', { static: false })
  seletedTable: ProTableComponent;
  @ViewChild('tipContentTpl', { static: false }) tipContentTpl: TemplateRef<
    any
  >;

  // selectedResources 做代理触发表单状态更新
  _selectedResources = [];

  get selectedResources() {
    return this._selectedResources;
  }

  set selectedResources(val) {
    this.formGroup.updateValueAndValidity();
    this._selectedResources = cloneDeep(val);
  }

  constructor(
    private fb: FormBuilder,
    private datePipe: DatePipe,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    private resourceService: ResourceService,
    private genConditionsServie: GenConditionsService,
    private baseUtilService: BaseUtilService,
    private copiesApiService: CopiesService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService,
    private copyControllerService: CopyControllerService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initResourceTypes();
    this.initForm();
    this.initOptions();
    this.initTable();
    this.updateData();
  }

  updateData() {
    if (isNil(this.data)) {
      return;
    }

    this.selectedResources = this.data.selectedResources;
    this.formGroup.patchValue(this.data);
    if (!this.data.schedule) {
      return;
    }
    this.formGroup.patchValue(this.data.schedule);
    if (this.data.schedule.copyTime) {
      this.formGroup
        .get('copyTime')
        .setValue(new Date(this.data.schedule.copyTime));
    }
    if (this.data.schedule.startDetectionTime) {
      this.formGroup
        .get('startDetectionTime')
        .setValue(new Date(this.data.schedule.startDetectionTime));
    }
    if (!this.data.schedule.intervalUnit) {
      this.formGroup
        .get('intervalUnit')
        .setValue(DataMap.Interval_Unit.hour.value);
    }
    if (
      !this.data.schedule.detectionType &&
      this.data.schedule.detectionType !== 0
    ) {
      this.formGroup
        .get('detectionType')
        .setValue(DataMap.Detecting_Range.last.value);
    }
    defer(() => {
      if (this.data.schedule?.schedulePolicy) {
        this.formGroup
          .get('schedulePolicy')
          .setValue(this.data.schedule.schedulePolicy);
      }
    });
  }

  /**
   *  初始化资源类型options
   */
  initResourceTypes() {
    this.resourceTypes = this.dataMapService
      .toArray('Job_Target_Type')
      .filter((v: OptionItem) => {
        if (
          this.appUtilsService.isDistributed ||
          this.appUtilsService.isDecouple
        ) {
          EXCLUDE_RESOURCE_TYPES.push(DataMap.Job_Target_Type.NASFileSystem);
        }
        return (
          (v.isLeaf = true) &&
          !includes(
            EXCLUDE_RESOURCE_TYPES.map(i => i.value),
            v.value
          )
        );
      })
      .map(item => {
        if (item.value === 'Host__and__FusionCompute')
          return { ...item, value: 'FusionComputeHost' };
        if (item.value === 'Cluster__and__FusionCompute')
          return { ...item, value: 'FusionComputeCluster' };
        return item;
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      policyName: new FormControl(
        { value: '', disabled: !!this.data },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name()
          ]
        }
      ),
      description: new FormControl(''),
      schedulePolicy: new FormControl(
        DataMap.Scheduling_Plan.immediately.value
      ),
      needDetect: new FormControl(false),
      setWorm: new FormControl(false),
      interval: new FormControl(''),
      intervalUnit: new FormControl(DataMap.Interval_Unit.hour.value),
      startDetectionTime: new FormControl(''),
      detectionType: new FormControl(DataMap.Detecting_Range.last.value),
      copyTime: new FormControl(''),
      antiTamperingSetting: new FormControl(''),
      dataSourceType: new FormControl(
        DataMap.Detecting_Data_Source.local.value
      ),
      resourceSubType: new FormControl('')
    });
    this.formGroup.get('schedulePolicy').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (
        res === DataMap.Scheduling_Plan.immediately.value ||
        res === 'afterTaskCompleteSet'
      ) {
        this.formGroup.get('interval').clearValidators();
        this.formGroup.get('startDetectionTime').clearValidators();
      } else {
        this.changeTimeUnits(this.formGroup.value.intervalUnit);
        this.formGroup
          .get('startDetectionTime')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('interval').updateValueAndValidity();
      this.formGroup.get('startDetectionTime').updateValueAndValidity();
    });
    this.formGroup.get('detectionType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res === DataMap.Detecting_Range.specified.value) {
        this.formGroup
          .get('copyTime')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('copyTime').clearValidators();
      }
      this.formGroup.get('copyTime').updateValueAndValidity();
    });
    this.formGroup.get('dataSourceType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      defer(() => {
        // 区分修改和创建场景
        this.configSourceSelect();
      });
    });

    this.formGroup.get('resourceSubType').valueChanges.subscribe(res => {
      const _res = this._checkPolicyOnResouceChange(res);
      this.resourceCheckFaild = !_res;

      if (!size(this.selectedResources)) {
        defer(() => {
          this.restTable();
        });
        return;
      }
    });

    this.formGroup
      .get('resourceSubType')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        // 规避短时间内多次 resourceSubType 变化
        if (!isNil(this.resourceChangeTimer)) return;
        this.resourceChangeTimer = setTimeout(() => {
          this.resourceChangeTimer = null;
        }, 100);

        if (
          _toString(res[0]) === _toString(res[1]) ||
          !size(this.selectedResources)
        ) {
          defer(() => {
            this.resourceTable?.fetchData();
          });
          return;
        }
        if (this.isRecover) {
          this.isRecover = false;
          return;
        }
        this.typeChangeLabel = this.i18n.get('explore_resource_change_label', [
          this.dataMapService.getLabel('Job_Target_Type', res[0])
        ]);
        this.messageBox.info({
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.tipContentTpl,
          lvFooter: [
            {
              label: this.i18n.get('common_ok_label'),
              onClick: modal => {
                this.isRecover = false;
                this.restTable();
                modal.close();
              }
            },
            {
              label: this.i18n.get('common_cancel_label'),
              onClick: modal => {
                this.isRecover = true;
                this.formGroup
                  .get('resourceSubType')
                  .setValue(_toString(res[0]));
                modal.close();
              }
            }
          ],
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              this.resourceTable?.fetchData();
            }
          }
        });
      });

    this.formGroup.get('antiTamperingSetting').valueChanges.subscribe(val => {
      defer(() => {
        this.antiPolicySwitchDisabled = !isNil(val);
      });
    });

    this.formGroup.get('needDetect').valueChanges.subscribe(val => {
      defer(() => {
        this.antiTamperingSettingOptions[0].disabled = val;
        this.antiTamperingSettingOptions[1].disabled = !val;
        this.formGroup.get('antiTamperingSetting').setValue(val ? '1' : '0');
        this.formGroup
          .get('schedulePolicy')
          .setValue(DataMap.Scheduling_Plan.immediately.value);
        const _res = this._checkResourceOnPolicyChange(val);
        this.resourceCheckFaild = !_res;
      });
    });

    this.formGroup.get('setWorm').valueChanges.subscribe(val => {
      const needDetect = this.formGroup.get('needDetect').value;

      defer(() => {
        this.formGroup
          .get('antiTamperingSetting')
          .setValue(
            val ? (this.formGroup.get('needDetect').value ? '1' : '0') : null
          );
      });
    });
  }

  configSourceSelect() {
    // 选中资源类型后处理
    if (!this.data && !!this.formGroup.get('resourceSubType').value) {
      this.restTable();
    }
    if (this.isInit) {
      this.restTable();
    }
    if (this.data && !this.isInit) {
      this.isInit = true;
      if (this.resourceTable) {
        this.resourceTable?.fetchData();
      } else {
        this.getData({
          paginator: {
            pageIndex: 0,
            pageSize: 10
          }
        });
      }
    }
  }

  restTable() {
    this[
      this.activeIndex === 'total' ? 'resourceTable' : 'seletedTable'
    ]?.setSelections([]);
    this.selectedResources = [];
    this.activeIndex = 'total';
    this.selectedTableData = {
      data: [],
      total: 0
    };
    this.currentSelect = 0;
    if (this.resourceTable) {
      this.resourceTable?.fetchData();
    } else {
      this.getData({
        paginator: {
          pageIndex: 0,
          pageSize: 10
        }
      });
    }
  }

  /**
   * 当资源类型选择变化时检查防勒索/防篡改策略
   * @param {Array<string>} changes - 资源类型变化
   * @returns {Boolean}
   */
  private _checkPolicyOnResouceChange(value: string): boolean {
    const needDetect = this.formGroup.get('needDetect').value;
    const isNotInAntiResource = !includes(
      ANTI_RESOURCE_TYPES.map(item => item.value),
      value
    );
    if (needDetect && isNotInAntiResource) {
      this.messageService.error(
        this.i18n.get('explore_anti_worm_policy_error_label', [
          this._getResourceLabel(value)
        ])
      );
      return false;
    }
    return true;
  }

  /**
   * 当防勒索策略变化时检查选中资源
   * @param {Boolean} value - 防勒索策略开、关 状态
   * @returns {Boolean}
   */
  private _checkResourceOnPolicyChange(value: boolean): boolean {
    const resourceType = this.formGroup.get('resourceSubType').value;
    if (
      !!value &&
      !includes(
        ANTI_RESOURCE_TYPES.map(i => i.value),
        resourceType
      )
    ) {
      !isEmpty(resourceType) &&
        this.messageService.error(
          this.i18n.get('explore_anti_worm_policy_error_label', [
            this._getResourceLabel(resourceType)
          ])
        );
      return false;
    }
    return true;
  }

  private _getResourceLabel(value: string): string {
    return this.resourceTypes.find(i => i.value === value)?.label;
  }
  initOptions() {
    this.intervalUnit = this.dataMapService
      .toArray('Interval_Unit')
      .filter((v: OptionItem) => {
        return v.value !== 'y' && v.value !== 'p' && v.value !== 'MO';
      })
      .filter((v: OptionItem) => {
        return (v.isLeaf = true);
      });
    defer(() => {
      const needDetect = this?.data?.schedule?.needDetect;
      this.antiTamperingSettingOptions = [
        {
          key: '0',
          label: this.i18n.get('explore_anti_setting_01_label'),
          value: '0',
          isLeaf: true,
          disabled: isNil(needDetect) ? false : needDetect
        },
        {
          key: '1',
          label: this.i18n.get('explore_anti_setting_02_label'),
          value: '1',
          isLeaf: true,
          disabled: isNil(needDetect) ? true : !needDetect
        }
      ];
    });
  }

  initTable() {
    const tapeCols: TableCols[] = [
      {
        key: 'resourceId',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true
      },
      {
        key: 'resourceName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'resourceLocation',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'policyName',
        name: this.i18n.get('explore_belong_policy_name_label')
      },
      {
        key: 'sla_name',
        name: this.i18n.get('SLA'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.tableConfig = {
      table: {
        size: 'small',
        compareWith: 'resourceId',
        columns: tapeCols,
        virtualScroll: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '340px' },
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectedResources = selection;
          this.currentSelect = selection.length;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
    this.selectedTableConfig = {
      table: {
        async: false,
        size: 'small',
        compareWith: 'resourceId',
        columns: tapeCols,
        virtualScroll: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '340px' },
        selectionChange: selection => {
          this.selectedResources = selection;
          this.currentSelect = selection.length;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  selectIndexChange(event) {
    if (event === 'selected') {
      this.selectedResources.filter(item => {
        assign(item, {
          policyName: this.data?.policyName ? this.data?.policyName : ''
        });
      });
      this.selectedTableData = {
        data: this.selectedResources ? this.selectedResources : [],
        total: size(this.selectedResources) ? this.selectedResources.length : 0
      };
      this.seletedTable.setSelections(this.selectedResources);
    } else {
      this.resourceTable?.fetchData();
    }
  }

  getData(filters?: Filters) {
    if (
      this.formGroup.value.dataSourceType ===
      DataMap.Detecting_Data_Source.local.value
    ) {
      this.getLocalResource(filters);
    } else {
      this.getTargetResource(filters);
    }
  }

  getLocalResource(filters: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const conditions = this.genConditionsServie.getConditions(
      this.formGroup.value.resourceSubType
    );
    // 区分Kingbase、informix两种实例
    if (
      [
        DataMap.Resource_Type.KingBaseInstance.value,
        DataMap.Resource_Type.KingBaseClusterInstance.value,
        DataMap.Resource_Type.informixClusterInstance.value,
        DataMap.Resource_Type.informixInstance.value
      ].includes(this.formGroup.value.resourceSubType)
    ) {
      conditions.subType = [this.formGroup.value.resourceSubType];
    }

    if (!isEmpty(filters.conditions_v2)) {
      const filterConditions = JSON.parse(filters.conditions_v2);
      if (filterConditions.resourceName) {
        assign(conditions, {
          name: filterConditions.resourceName
        });
      }
      if (filterConditions.resourceLocation) {
        assign(conditions, {
          path: filterConditions.resourceLocation
        });
      }
      if (filterConditions.protectedObject) {
        assign(conditions, {
          protectedObject: filterConditions.protectedObject
        });
      }
    }
    assign(params, { conditions: JSON.stringify(conditions) });
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        const resourceParams = {
          resourceIds: map(res.records, 'uuid')
        };
        this.antiRansomwarePolicyApiService
          .ShowAntiRansomwarePolicies(resourceParams)
          .subscribe(data => {
            each(res.records, item => {
              extendSlaInfo(item);
            });
            this.tableData = {
              data: res.records
                .map(item => {
                  const antiResource = {
                    policyName: '',
                    resourceId: item.uuid,
                    resourceName: item.name,
                    resourceLocation: item.path,
                    resourceSubType: item.subType,
                    sla_name: item?.sla_name
                  };
                  each(data.records, policy => {
                    const resource = find(policy.selectedResources, {
                      resourceId: item.uuid
                    });
                    if (!isUndefined(resource)) {
                      assign(antiResource, { policyName: policy?.policyName });
                    }
                  });
                  return antiResource;
                })
                .filter(item =>
                  isEmpty(item?.policyName) ||
                  (!isEmpty(this.data) &&
                    this.data?.policyName === item?.policyName)
                    ? item
                    : assign(item, { disabled: true })
                ),
              total: res.totalCount
            };
            this.cdr.detectChanges();
            this.resourceTable?.setSelections(this.selectedResources);
            this.currentSelect =
              this.resourceTable?.getAllSelections().length || 0;
            this.formGroup.updateValueAndValidity();
            this.currentTotal = res.totalCount;
          });
      });
  }

  getTargetResource(filters: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const conditions = {
      resourceSubType: [this.formGroup.value.resourceSubType],
      generated_by_array: [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.reverseReplication.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ]
    };
    if (!isEmpty(filters.conditions)) {
      const filterConditions = JSON.parse(filters.conditions);
      if (filterConditions.resourceName) {
        assign(conditions, {
          resourceName: filterConditions.resourceName
        });
      }
      if (filterConditions.resourcePath) {
        assign(conditions, {
          resourceLocation: filterConditions.resourcePath
        });
      }
      if (filterConditions.sla_name) {
        assign(conditions, {
          protectedSlaName: filterConditions.sla_name
        });
      }
    }
    assign(params, { conditions: JSON.stringify(conditions) });
    this.copyControllerService
      .queryCopySummaryResourceV2(params)
      .subscribe(res => {
        each(res.records, item => {
          extendSummaryCopiesParams(item);
          assign(item, {
            sla_name: item.protectedSlaName
          });
        });
        const resourceParams = {
          resourceIds: map(res.records, 'resource_id')
        };
        this.antiRansomwarePolicyApiService
          .ShowAntiRansomwarePolicies(resourceParams)
          .subscribe(data => {
            this.tableData = {
              data: res.records
                .map(item => {
                  const antiResource = {
                    policyName: '',
                    resourceId: item.resource_id,
                    resourceName: item.resource_name,
                    resourceLocation: item.resource_location,
                    resourceSubType: item.resource_sub_type,
                    sla_name: item?.sla_name
                  };
                  each(data.records, policy => {
                    const resource = find(policy.selectedResources, {
                      resourceId: item.resource_id
                    });
                    if (!isUndefined(resource)) {
                      assign(antiResource, { policyName: policy.policyName });
                    }
                  });

                  return antiResource;
                })
                .filter(item =>
                  isEmpty(item?.policyName) ||
                  (!isEmpty(this.data) &&
                    this.data?.policyName === item?.policyName)
                    ? item
                    : assign(item, { disabled: true })
                ),
              total: res.totalCount
            };
            this.cdr.detectChanges();
            this.resourceTable?.setSelections(this.selectedResources);
            this.currentSelect =
              this.resourceTable?.getAllSelections().length || 0;
            this.formGroup.updateValueAndValidity();
            this.currentTotal = res.totalCount;
          });
      });
  }

  onOK(): Observable<any> {
    return this.data ? this.onModify() : this.onCreate();
  }

  onCreate(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      this.antiRansomwarePolicyApiService
        .CreateAntiRansomwarePolic({
          CreateAntiRansomwarePolicRequestBody: this.getParams()
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  onModify(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      this.warningMessageService.create({
        content: this.i18n.get('explore_modify_anti_policy_label', [
          this.data.policyName
        ]),
        onOK: () => {
          this.antiRansomwarePolicyApiService
            .UpdateAntiRansomwarePolicy({
              id: this.data.id,
              UpdateAntiRansomwarePolicyRequestBody: this.getParams()
            })
            .subscribe(
              res => {
                observer.next(res);
                observer.complete();
              },
              error => {
                observer.error(error);
                observer.complete();
              }
            );
        },
        onCancel: () => {
          observer.error('');
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }

  changeTimeUnits(value) {
    this.formGroup.get('interval').enable();
    if (value === DataMap.Interval_Unit.minute.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 59)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 59])
      });
    } else if (value === DataMap.Interval_Unit.hour.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 23)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 23])
      });
    } else if (value === DataMap.Interval_Unit.day.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 7)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 7])
      });
    } else if (value === DataMap.Interval_Unit.week.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 4)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 4])
      });
    } else if (value === DataMap.Interval_Unit.month.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 12)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 12])
      });
    } else if (value === DataMap.Interval_Unit.year.value) {
      this.formGroup
        .get('interval')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ]);
      this.intervalErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
      });
    } else if (value === DataMap.Interval_Unit.persistent.value) {
      this.formGroup.get('interval').disable();
      this.formGroup.get('interval').setValue('');
    }
    this.formGroup.get('interval').updateValueAndValidity();
  }

  getParams(): AntiRansomwarePolicyReq {
    const antiRansomwarePolicyReq = {
      dataSourceType: this.formGroup.value.dataSourceType,
      resourceSubType: this.formGroup.value.resourceSubType,
      description: this.formGroup.value.description,
      policyName: this.formGroup.get('policyName').value,
      selectedResources: this.selectedResources,
      schedule:
        this.formGroup.value.schedulePolicy ===
        DataMap.Scheduling_Plan.interval.value
          ? {
              schedulePolicy: this.getSchedulePolicy(),
              startDetectionTime: this.datePipe.transform(
                this.formGroup.value.startDetectionTime,
                'yyyy-MM-dd HH:mm:ss'
              ),
              interval: this.formGroup.value.interval,
              intervalUnit: this.formGroup.value.intervalUnit,
              detectionType: this.formGroup.value.detectionType,
              copyTime:
                this.formGroup.value.detectionType ===
                DataMap.Detecting_Range.specified.value
                  ? this.datePipe.transform(
                      this.formGroup.value.copyTime,
                      'yyyy-MM-dd HH:mm:ss'
                    )
                  : null,
              setWorm: this.formGroup.value.setWorm,
              needDetect: this.formGroup.value.needDetect
            }
          : {
              schedulePolicy: this.getSchedulePolicy(),
              setWorm: this.formGroup.value.setWorm,
              needDetect: this.formGroup.value.needDetect
            },
      clusterId:
        this.formGroup.value.dataSourceType ===
        DataMap.Detecting_Data_Source.local.value
          ? DataMap.Cluster_Type.local.value
          : null
    };
    return antiRansomwarePolicyReq;
  }

  private getSchedulePolicy(): any {
    if (!this.formGroup.get('needDetect').value) {
      return 'afterTaskCompleteSet';
    }
    return this.formGroup.value.schedulePolicy;
  }
}
