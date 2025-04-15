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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AntiRansomwarePolicyApiService,
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  CopiesService,
  CopyControllerService,
  DataMap,
  DataMapService,
  DetectionCopyAction,
  extendSummaryCopiesParams,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  RoleType,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  find,
  first,
  includes,
  isEmpty,
  reject,
  size,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { ResourceListComponent } from '../../copy-data/copy-resource-list/resource-list/resource-list.component';
import { AntiPolicyDetailComponent } from '../../policy/anti-policy-setting/anti-policy/anti-policy-detail/anti-policy-detail.component';
import { DetectionRepicasListComponent } from './detection-repicas-list/detection-repicas-list.component';

@Component({
  selector: 'aui-resource-statistic',
  templateUrl: './resource-statistic.component.html',
  styleUrls: ['./resource-statistic.component.less']
})
export class ResourceStatisticComponent
  implements OnInit, AfterViewInit, OnDestroy {
  name;
  dataMap = DataMap;
  header;
  resourceType = this.dataMapService
    .toArray('Detecting_Resource_Type')
    .map(item => item.value);
  tableData: TableData;
  timeSub$: Subscription;
  destroy$ = new Subject();
  tableConfig: TableConfig;
  resourceListComponent: ResourceListComponent;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  appTypeOptions = this.dataMapService
    .toArray('Detecting_Resource_Type')
    .map(item => {
      return assign(item, {
        isLeaf: true
      });
    });
  appType = [];
  detectType = [];
  detectionTypeOptions = this.dataMapService.toArray('Detection_Copy_Type');

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('sourceNamePopover', { static: false }) sourceNamePopover;
  @ViewChild('repicasTpl', { static: true }) repicasTpl: TemplateRef<any>;
  @ViewChild('infectedRepicasTpl', { static: true })
  infectedRepicasTpl: TemplateRef<any>;
  @ViewChild('uninfectedRepicasTpl', { static: true })
  uninfectedRepicasTpl: TemplateRef<any>;
  @ViewChild('prepareRepicasTpl', { static: true })
  prepareRepicasTpl: TemplateRef<any>;
  @ViewChild('detectingRepicasTpl', { static: true })
  detectingRepicasTpl: TemplateRef<any>;
  @ViewChild('uninspectedRepicasTpl', { static: true })
  uninspectedRepicasTpl: TemplateRef<any>;
  @ViewChild('abnormalRepicasTpl', { static: true })
  abnormalRepicasTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private copyControllerService: CopyControllerService,
    private virtualResourceService: VirtualResourceService,
    private copiesDetectReportService: CopiesDetectReportService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initData();
    this.initConfig();
    this.getComponents();
  }

  initData() {
    if (this.isHcsUser) {
      this.appTypeOptions = reject(this.appTypeOptions, item =>
        includes(
          [DataMap.Detecting_Resource_Type.openstackServer.value],
          item.value
        )
      );
      this.resourceType = reject(this.resourceType, item =>
        includes(
          [DataMap.Detecting_Resource_Type.openstackServer.value],
          item.value
        )
      );
    }
    const globalDetectType = this.appUtilsService.getCacheValue(
      'jump-to-anti-ransomware'
    );
    this.detectType = isEmpty(globalDetectType)
      ? []
      : [find(this.detectionTypeOptions, { value: globalDetectType })];
  }

  onChange() {
    this.tableData = {
      data: [],
      total: 0
    };
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  getComponents() {
    this.resourceListComponent = new ResourceListComponent(
      this.i18n,
      this.detailService,
      this.copiesApiService,
      this.dataMapService,
      this.warningMessageService,
      this.virtualScroll,
      this.cdr,
      this.copyControllerService
    );
  }

  getResourceDetail(item) {
    // vmware单独处理
    if (item.resource_sub_type === DataMap.Resource_Type.virtualMachine.value) {
      this.virtualResourceService
        .queryResourcesV1VirtualResourceGet({
          pageSize: CommonConsts.PAGE_SIZE,
          pageNo: CommonConsts.PAGE_START,
          conditions: JSON.stringify({
            uuid: item.resource_id
          })
        })
        .subscribe(vmRes =>
          this.detailService.openDetailModal(item.resource_sub_type, {
            data: first(vmRes.items)
          })
        );
      return;
    }
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({ resource_id: item.resource_id })
    };
    this.copyControllerService
      .queryCopySummaryResourceV2(params)
      .subscribe(res => {
        each(res.records, item => extendSummaryCopiesParams(item));
        this.resourceListComponent.getResourceDetail({
          ...first(res.records),
          name: item.name,
          resource_name: item.name
        });
      });
  }

  getRepicasDetail(rowData, copyType, count) {
    if (!count) {
      return;
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_copies_label'),
      lvModalKey: 'anti-copies' + copyType,
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvComponentParams: {
        action: DetectionCopyAction.View,
        resourceType: this.resourceType,
        rowData,
        copyType
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  maunalDetect(rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_maunal_detect_label'),
      lvModalKey: 'anti-copy-maunalDetect',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as DetectionRepicasListComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        action: DetectionCopyAction.DetectionSelect,
        resourceType: this.resourceType,
        rowData
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as DetectionRepicasListComponent;
          content.batchMaunalDetect().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  feedback(rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_error_feedbac_label'),
      lvModalKey: 'anti-copy-feedback',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetectionRepicasListComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as DetectionRepicasListComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        action: DetectionCopyAction.FeedbackSelect,
        resourceType: this.resourceType,
        rowData
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as DetectionRepicasListComponent;
          content.batchFeedback().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  delete(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_delete_kubernetes_rule_label'),
      onOK: () => {
        this.dataTable.fetchData();
      }
    });
  }

  getPolicyDetail(data) {
    const params = {
      startPage: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      id: data.policy_id
    };
    this.antiRansomwarePolicyApiService
      .ShowAntiRansomwarePolicies(params)
      .subscribe(res => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'anti-policy-detail',
            lvWidth: MODAL_COMMON.normalWidth,
            lvHeader: data.policy_name,
            lvContent: AntiPolicyDetailComponent,
            lvComponentParams: {
              data: res.records[0]
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ]
          })
        );
      });
  }

  search(value) {
    this.dataTable.filterChange({
      caseSensitive: false,
      filterMode: 'contains',
      key: 'name',
      value: trim(value)
    });
  }

  changeAppType(e) {
    this.dataTable.filterChange({
      caseSensitive: false,
      filterMode: 'in',
      key: 'resourceSubType',
      value: this.appType.map(item => item.value)
    });
  }

  changeDetectType(e) {
    this.dataTable.fetchData();
  }

  getData(filters: Filters) {
    const params: any = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      resourceSubType: this.resourceType
    };

    const defaultConditions = {};
    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.resourceSubType) {
        params.resourceSubType = conditionsTemp.resourceSubType;
        delete conditionsTemp.resourceSubType;
      }

      if (!!conditionsTemp) {
        assign(defaultConditions, conditionsTemp);
      }
    }

    if (!!this.detectType.length) {
      assign(defaultConditions, {
        detection_status: this.detectType.map(item => item.value)
      });
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.copiesDetectReportService.ShowDetectionStatistics({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        each(res.items, (item: any) => {
          assign(item, {
            resourceSubType: item?.resource_sub_type
          });
        });
        this.tableData = {
          data: res.items,
          total: res.total
        };
        this.cdr.detectChanges();
      });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'maunalDetect',
        label: this.i18n.get('explore_maunal_detect_label'),
        onClick: data => {
          this.maunalDetect(data[0]);
        }
      },
      {
        id: 'feedback',
        label: this.i18n.get('explore_error_feedbac_label'),
        onClick: data => {
          this.feedback(data[0]);
        }
      }
    ];

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_resource_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getResourceDetail(data);
            }
          }
        }
      },
      {
        key: 'location',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'resourceSubType',
        name: this.i18n.get('common_resource_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Detecting_Resource_Type')
            .filter(item => {
              if (this.isHcsUser) {
                return !includes(
                  [DataMap.Detecting_Resource_Type.openstackServer.value],
                  item.value
                );
              }
              return true;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detecting_Resource_Type')
        }
      },
      {
        key: 'policy_name',
        name: this.i18n.get('explore_associated_policy_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            click: data => {
              this.getPolicyDetail(data);
            }
          }
        }
      },
      {
        key: 'total_copy_num',
        name: this.i18n.get('common_number_of_copy_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.repicasTpl
      },
      {
        key: 'infected_copy_num',
        name: this.i18n.get('explore_infected_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.infectedRepicasTpl
      },
      {
        key: 'uninfected_copy_num',
        name: this.i18n.get('explore_uninfected_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.uninfectedRepicasTpl
      },
      {
        key: 'abnormal_copy_num',
        name: this.i18n.get('explore_detecte_fail_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.abnormalRepicasTpl
      },
      {
        key: 'detecting_copy_num',
        name: this.i18n.get('explore_detecting_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.detectingRepicasTpl
      },
      {
        key: 'prepare_copy_num',
        name: this.i18n.get('common_ready_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.prepareRepicasTpl
      },
      {
        key: 'uninspected_copy_num',
        name: this.i18n.get('explore_uninspected_label'),
        thAlign: 'right',
        sort: true,
        cellRender: this.uninspectedRepicasTpl
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: opts
          }
        }
      }
    ];

    if (this.cookieService.role === RoleType.Auditor) {
      cols.splice(10, 1);
    }

    this.tableConfig = {
      table: {
        compareWith: 'resource_id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters) => {
          this.getData(filters);
        },
        trackByFn: (index, item) => {
          return item.resource_id;
        }
      }
    };
  }
}
