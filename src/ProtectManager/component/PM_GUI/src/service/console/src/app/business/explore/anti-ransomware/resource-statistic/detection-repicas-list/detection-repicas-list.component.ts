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
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import { CopyListComponent } from 'app/business/explore/copy-data/copy-resource-list/copy-list/copy-list.component';
import {
  AntiRansomwarePolicyApiService,
  CommonConsts,
  CommonShareRestoreApiService,
  CookieService,
  CopiesDetectReportService,
  CopiesService,
  CopyControllerService,
  DataMap,
  DataMapService,
  DetectionCopyAction,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RestoreType,
  RoleType,
  TapeCopyApiService,
  WarningMessageService
} from 'app/shared';
import { ModifyRetentionPolicyComponent } from 'app/shared/components';
import { CopyDataDetailComponent } from 'app/shared/components/copy-data-detail/copy-data-detail.component';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { RestoreService } from 'app/shared/services/restore.service';
import { TakeManualArchiveService } from 'app/shared/services/take-manual-archive.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  filter,
  find,
  first,
  get as __get,
  includes,
  intersection,
  isBoolean,
  isEmpty,
  isUndefined,
  map,
  reject,
  remove,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { DetectionReportComponent } from './detection-report/detection-report.component';
import { FeedbackWarningComponent } from './feedback-warning/feedback-warning.component';

@Component({
  selector: 'aui-detection-repicas-list',
  templateUrl: './detection-repicas-list.component.html',
  styleUrls: ['./detection-repicas-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class DetectionRepicasListComponent implements OnInit, AfterViewInit {
  action;
  rowData;
  copyType;
  resourceType;
  optsConfig;
  helpLabel = '';
  currentTotal = 0;
  currentSelect = 0;
  selectionData = [];
  activeIndex = 'total';
  _isEmpty = isEmpty;
  tableData: TableData;
  tableConfig: TableConfig;
  detectionCopyAction = DetectionCopyAction;
  copyListComponent: CopyListComponent;
  valid$ = new Subject<boolean>();
  deleteBtnDisable = true;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    public cookieService: CookieService,
    private messageBox: MessageboxService,
    private restoreService: RestoreService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private manualMountService: ManualMountService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private copyActionService: CopyActionService,
    private copyControllerService: CopyControllerService,
    private takeManualArchiveService: TakeManualArchiveService,
    private message: MessageService,
    private tapeCopyApiService: TapeCopyApiService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService,
    private copiesDetectReportService: CopiesDetectReportService,
    private cdr: ChangeDetectorRef,
    private globalService: GlobalService,
    private commonShareRestoreApiService: CommonShareRestoreApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngAfterViewInit() {
    if (
      !isUndefined(this.copyType) &&
      this.copyType !== DataMap.Detection_Copy_Status.all.value
    ) {
      this.dataTable.setFilterMap(
        assign(this.dataTable.filterMap, {
          filters: [
            {
              caseSensitive: false,
              key: 'anti_status',
              value: [this.copyType]
            }
          ]
        })
      );
      return;
    }
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    this.getComponents();
  }

  getComponents() {
    this.copyListComponent = new CopyListComponent(
      this.i18n,
      this.datePipe,
      this.detailService,
      this.dataMapService,
      this.drawModalService,
      this.copiesApiService,
      this.restoreService,
      this.warningMessageService,
      this.manualMountService,
      this.messageBox,
      this.message,
      this.tapeCopyApiService,
      this.takeManualArchiveService,
      this.cookieService,
      this.virtualScroll,
      this.cdr,
      this.copyActionService,
      this.copyControllerService,
      this.globalService,
      this.commonShareRestoreApiService,
      this.appUtilsService
    );
  }

  manualMount(item) {
    this.manualMountService.create({
      item,
      resType: this.resourceType,
      onOk: () => {
        this.dataTable.fetchData();
      }
    });
  }

  modifyRetentionPolicy(data) {
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
                this.dataTable.fetchData();
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }

  initConfig() {
    const getStatusOpts = () => {
      const opts = this.dataMapService
        .toArray('copydata_validStatus', [
          DataMap.copydata_validStatus.normal.value,
          DataMap.copydata_validStatus.invalid.value,
          DataMap.copydata_validStatus.deleting.value
        ])
        .filter(item => {
          return !includes(
            [
              DataMap.copydata_validStatus.sharing.value,
              DataMap.copydata_validStatus.downloading.value
            ],
            item.value
          );
        });

      if (
        !!intersection(this.resourceType, [
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.virtualMachine.value
        ])
      ) {
        return reject(opts, item =>
          includes([DataMap.copydata_validStatus.verifying.value], item.value)
        );
      }

      return opts;
    };
    this.optsConfig = [
      {
        id: 'maunalDetect',
        label: this.i18n.get('explore_maunal_detect_label'),
        disableCheck: data => {
          return !includes(
            [
              DataMap.Detection_Copy_Status.uninspected.value,
              DataMap.Detection_Copy_Status.infected.value,
              DataMap.Detection_Copy_Status.uninfected.value,
              DataMap.Detection_Copy_Status.exception.value
            ],
            data[0].anti_status
          );
        },
        onClick: data => {
          this.maunalDetect(data);
        }
      },
      {
        id: 'feedback',
        label: this.i18n.get('explore_error_feedbac_label'),
        disableCheck: data => {
          return !includes(
            [DataMap.Detection_Copy_Status.infected.value],
            data[0].anti_status
          );
        },
        onClick: data => {
          this.feedback(data);
        }
      },
      {
        id: 'detectionReport',
        label: this.i18n.get('explore_detection_report_label'),
        disableCheck: data => {
          return !includes(
            [
              DataMap.Detection_Copy_Status.infected.value,
              DataMap.Detection_Copy_Status.uninfected.value,
              DataMap.Detection_Copy_Status.exception.value
            ],
            data[0].anti_status
          );
        },
        onClick: data => {
          this.detectionReport(data);
        }
      },
      {
        id: 'restore',
        label:
          this.resourceType === DataMap.Resource_Type.HCSCloudHost.value
            ? this.i18n.get('common_restoring_disks_label')
            : this.i18n.get('common_restore_label'),
        disableCheck: data => {
          return data[0].status !== DataMap.copydata_validStatus.normal.value;
        },
        displayCheck: data => {
          return !(data[0].features
            ? data[0].features
                .toString(2)
                .split('')
                .reverse()[1] !== '1'
            : false);
        },
        permission: OperateItems.RestoreCopy,
        onClick: data => {
          let resourceName;
          if (data[0].resource_sub_type === 'LUN') {
            resourceName = this.i18n.get('protection_local_lun_label');
          } else {
            resourceName = this.dataMapService.getLabel(
              'Detecting_Resource_Type',
              data[0].resource_sub_type
            );
          }

          if (
            data[0].anti_status === DataMap.Detection_Copy_Status.infected.value
          ) {
            this.warningMessageService.create({
              content: this.i18n.get('explore_infected_operation_label', [
                resourceName,
                this.datePipe.transform(
                  data[0].display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                ),
                this.i18n.get('common_restore_label')
              ]),
              onOK: () => {
                this.restoreService.restore({
                  childResType: data[0].resource_sub_type,
                  copyData: data[0],
                  restoreType: RestoreType.CommonRestore,
                  onOk: () => {
                    this.dataTable.fetchData();
                  }
                });
              }
            });
            return;
          }
          this.restoreService.restore({
            childResType: data[0].resource_sub_type,
            copyData: data[0],
            restoreType: RestoreType.CommonRestore,
            onOk: () => {
              this.dataTable.fetchData();
            }
          });
        }
      },
      {
        id: 'fileLevelRestore',
        label: this.i18n.get('common_file_level_restore_label'),
        disableCheck: data => {
          return (
            (data[0].resource_sub_type ===
              DataMap.Resource_Type.fileset.value &&
              data[0].generated_by ===
                DataMap.CopyData_generatedType.replicate.value) ||
            (data[0].resource_sub_type ===
              DataMap.Resource_Type.HCSCloudHost.value &&
              !(
                data[0].status === DataMap.copydata_validStatus.normal.value &&
                includes(
                  [DataMap.CopyData_fileIndex.indexed.value],
                  data[0].indexed
                )
              ))
          );
        },
        displayCheck: data => {
          return (
            includes(
              [
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.NASFileSystem.value,
                DataMap.Resource_Type.fileset.value,
                DataMap.Resource_Type.HCSCloudHost.value
              ],
              data[0].resource_sub_type
            ) &&
            data[0].generated_by !==
              DataMap.CopyData_generatedType.tapeArchival.value
          );
        },
        permission: OperateItems.FileLevelRestore,
        onClick: data => {
          if (
            data[0].anti_status === DataMap.Detection_Copy_Status.infected.value
          ) {
            this.warningMessageService.create({
              content: this.i18n.get('explore_infected_operation_label', [
                this.dataMapService.getLabel(
                  'Detecting_Resource_Type',
                  data[0].resource_sub_type
                ),
                this.datePipe.transform(
                  data[0].display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                ),
                this.i18n.get('common_file_level_restore_label')
              ]),
              onOK: () => {
                this.restoreService.fileLevelRestore({
                  header: this.i18n.get('common_file_level_restore_label'),
                  childResType: data[0].resource_sub_type,
                  copyData: data[0],
                  restoreType: RestoreType.FileRestore,
                  onOk: () => {
                    this.dataTable.fetchData();
                  }
                });
              }
            });
            return;
          }
          this.restoreService.fileLevelRestore({
            header: this.i18n.get('common_file_level_restore_label'),
            childResType: data[0].resource_sub_type,
            copyData: data[0],
            restoreType: RestoreType.FileRestore,
            onOk: () => {
              this.dataTable.fetchData();
            }
          });
        }
      },
      {
        id: 'instantRestore',
        label: this.i18n.get('common_live_restore_job_label'),
        disableCheck: data => {
          return data[0].status !== DataMap.copydata_validStatus.normal.value;
        },
        displayCheck: data => {
          return (
            !(data[0].features
              ? data[0].features
                  .toString(2)
                  .split('')
                  .reverse()[2] !== '1'
              : false) &&
            data[0].generated_by !==
              DataMap.CopyData_generatedType.tapeArchival.value &&
            !(
              data[0].generated_by ===
                DataMap.CopyData_generatedType.replicate.value &&
              data[0].resource_sub_type === DataMap.Resource_Type.fileset.value
            )
          );
        },
        permission: OperateItems.InstanceRecovery,
        onClick: data => {
          if (
            data[0].anti_status === DataMap.Detection_Copy_Status.infected.value
          ) {
            this.warningMessageService.create({
              content: this.i18n.get('explore_infected_operation_label', [
                this.dataMapService.getLabel(
                  'Detecting_Resource_Type',
                  data[0].resource_sub_type
                ),
                this.datePipe.transform(
                  data[0].display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                ),
                this.i18n.get('common_live_restore_job_label')
              ]),
              onOK: () => {
                this.restoreService.restore({
                  childResType: data[0].resource_sub_type,
                  copyData: data[0],
                  restoreType: RestoreType.InstanceRestore,
                  onOk: () => {
                    this.dataTable.fetchData();
                  }
                });
              }
            });
            return;
          }
          this.restoreService.restore({
            childResType: data[0].resource_sub_type,
            copyData: data[0],
            restoreType: RestoreType.InstanceRestore,
            onOk: () => {
              this.dataTable.fetchData();
            }
          });
        }
      },
      {
        id: 'mount',
        label: this.i18n.get('common_live_mount_label'),
        disableCheck: data => {
          return (
            DataMap.copydata_validStatus.normal.value !== data[0].status ||
            (data[0].generated_by ===
            DataMap.CopyData_generatedType.liveMount.value
              ? data[0].generation > DataMap.CopyData_Generation.two.value
              : data[0].generation >= DataMap.CopyData_Generation.two.value) ||
            (data[0].resource_sub_type ===
              DataMap.Resource_Type.virtualMachine.value &&
              JSON.parse(data[0].resource_properties).ext_parameters &&
              isBoolean(
                JSON.parse(data[0].resource_properties).ext_parameters.all_disk
              ) &&
              !JSON.parse(data[0].resource_properties).ext_parameters.all_disk)
          );
        },
        displayCheck: data => {
          return (
            (!(data[0].features
              ? data[0].features
                  .toString(2)
                  .split('')
                  .reverse()[3] !== '1'
              : false) &&
              data[0].generated_by !==
                DataMap.CopyData_generatedType.tapeArchival.value &&
              !(
                data[0].generated_by ===
                  DataMap.CopyData_generatedType.replicate.value &&
                data[0].resource_type === DataMap.Resource_Type.fileset.value
              )) ||
            (data[0].generated_by ===
              DataMap.CopyData_generatedType.backup.value &&
              data[0].resource_sub_type === DataMap.Resource_Type.fileset.value)
          );
        },
        permission: OperateItems.MountingCopy,
        onClick: data => {
          if (
            data[0].anti_status === DataMap.Detection_Copy_Status.infected.value
          ) {
            this.warningMessageService.create({
              content: this.i18n.get('explore_infected_operation_label', [
                this.dataMapService.getLabel(
                  'Detecting_Resource_Type',
                  data[0].resource_sub_type
                ),
                this.datePipe.transform(
                  data[0].display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                ),
                this.i18n.get('common_live_mount_label')
              ]),
              onOK: () => {
                this.manualMount(data[0]);
              }
            });
            return;
          }
          this.manualMount(data[0]);
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        divide: includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.NASFileSystem.value
          ],
          this.resourceType
        ),
        displayCheck: data => {
          return (
            data[0].generated_by !==
            DataMap.CopyData_generatedType.tapeArchival.value
          );
        },
        disableCheck: data => {
          return !includes(
            [
              DataMap.copydata_validStatus.normal.value,
              DataMap.copydata_validStatus.invalid.value
            ],
            data[0].status
          );
        },
        onClick: data => {
          this.deleteCopy(data);
        }
      }
    ];
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('ID'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        hidden:
          this.i18n.get('deploy_type') !== DataMap.Deploy_Type.hyperdetect.value
      },
      {
        key: 'display_timestamp',
        name:
          this.i18n.get('deploy_type') !== DataMap.Deploy_Type.hyperdetect.value
            ? this.i18n.get('common_time_stamp_label')
            : this.i18n.get('common_hyperdetect_time_stamp_label'),
        sort: true,
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            click: data => {
              this.getCopyDetail(data);
            }
          }
        }
      },
      {
        key: 'location',
        name: this.i18n.get('common_location_label'),
        width: 96,
        hidden:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      },
      {
        key: 'model',
        name: this.i18n.get('explore_detection_model_version_label'),
        hidden: true
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        width: 96,
        hidden: !this.isDataBackup,
        cellRender: {
          type: 'status',
          config: getStatusOpts()
        }
      },
      {
        key: 'anti_status',
        name: this.i18n.get('operation_target_detection_status_label'),
        width: this.action === DetectionCopyAction.View ? 118 : null,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Detection_Copy_Status')
            .filter(item => {
              return item.value !== DataMap.Detection_Copy_Status.all.value;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detection_Copy_Status')
        }
      },
      {
        key: 'detection_time',
        name: this.i18n.get('explore_detect_time_label'),
        sort: true
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items:
              this.cookieService.role === RoleType.Auditor
                ? filter(this.optsConfig, { id: 'detectionReport' })
                : this.optsConfig
          }
        }
      }
    ];

    if (this.action !== DetectionCopyAction.View) {
      remove(cols, { key: 'detection_time' });
    }

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        showLoading: false,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode:
            this.action === DetectionCopyAction.DetectionSelect
              ? 'single'
              : 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: { y: '54vh' },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.currentSelect = selection.length;
          this.valid$.next(!!size(this.selectionData));
          this.deleteBtnDisable =
            !selection.length ||
            !isUndefined(
              find(
                selection,
                item =>
                  !includes(
                    [
                      DataMap.copydata_validStatus.normal.value,
                      DataMap.copydata_validStatus.invalid.value,
                      DataMap.copydata_validStatus.deleteFailed.value
                    ],
                    item.status
                  ) ||
                  item.generated_by ===
                    DataMap.CopyData_generatedType.tapeArchival.value ||
                  item.backup_type === DataMap.CopyData_Backup_Type.log.value
              )
            );
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE
      }
    };
    if (this.action === DetectionCopyAction.View && !this.isDataBackup) {
      delete this.tableConfig.table.rows;
      delete this.tableConfig.table.selectionChange;
    }
  }

  selectIndexChange(event) {
    this.valid$.next(!!size(this.selectionData));
    if (event === 'selected') {
      this.tableData = {
        data: this.selectionData,
        total: this.selectionData.length
      };
    } else {
      this.dataTable.fetchData();
    }
    this.dataTable.setSelections(this.selectionData);
  }

  getData(filters?: Filters, args?) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      resourceId: this.rowData.resource_id,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      assign(params, { conditions: filters.conditions });
    }

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.copiesDetectReportService
      .ShowDetectionDetails(params)
      .subscribe(res => {
        if (this.action === DetectionCopyAction.DetectionSelect) {
          res.items.filter(item =>
            includes(
              [
                DataMap.Detection_Copy_Status.uninspected.value,
                DataMap.Detection_Copy_Status.infected.value,
                DataMap.Detection_Copy_Status.uninfected.value,
                DataMap.Detection_Copy_Status.exception.value
              ],
              item.anti_status
            )
              ? item
              : assign(item, { disabled: true })
          );
        }
        if (this.action === DetectionCopyAction.FeedbackSelect) {
          res.items.filter(item =>
            includes(
              [DataMap.Detection_Copy_Status.infected.value],
              item.anti_status
            )
              ? item
              : assign(item, { disabled: true })
          );
        }
        this.tableData = {
          data: res.items.filter(item =>
            assign(item, {
              display_timestamp: this.datePipe.transform(
                item.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              )
            })
          ),
          total: res.total
        };
        this.currentTotal = res.total;
        this.cdr.detectChanges();
      });
  }

  deleteCopyData() {
    // 批量删除
    const timeArr = map(this.selectionData, item => {
      return this.datePipe.transform(
        item.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
    });
    let uninfectedTimeArr;
    const callConfirm = isLastUninfectedCopy => {
      this.warningMessageService.create({
        content: isLastUninfectedCopy
          ? this.i18n.get('common_copy_batch_delete_uninfected_label', [
              timeArr.join(','),
              uninfectedTimeArr
            ])
          : this.i18n.get('common_copy_delete_label', [timeArr.join(',')]),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.copiesApiService.deleteCopyV1CopiesCopyIdDelete({
                copyId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            map(cloneDeep(this.selectionData), item => {
              return assign(item, {
                name: this.datePipe.transform(
                  item.display_timestamp,
                  'yyyy-MM-dd HH:mm:ss'
                ),
                isAsyn: true
              });
            }),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
              this.deleteBtnDisable = true;
            }
          );
        }
      });
    };
    let tmpData = filter(this.selectionData, item => item.anti_status === 2);
    if (!!tmpData.length) {
      this.copiesDetectReportService
        .ShowDetectionDetails({
          pageNo: 0,
          pageSize: 20,
          resourceId: this.rowData.resource_id,
          conditions: `{"anti_status":[${DataMap.Detection_Copy_Status.uninfected.value}]}`
        })
        .subscribe(res => {
          if (res.total === tmpData.length) {
            tmpData.sort((a, b) => b.display_timestamp - a.display_timestamp);
            uninfectedTimeArr = this.datePipe.transform(
              tmpData[0].display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            );
            callConfirm(true);
          } else {
            callConfirm(false);
          }
        });
    } else {
      callConfirm(false);
    }
  }

  getCopyDetail(item) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'explore-copy-list',
        lvWidth: MODAL_COMMON.largeWidth,
        lvContent: CopyDataDetailComponent,
        lvComponentParams: {
          data: assign(item, {
            optItems: this.copyListComponent.getOptItems(item),
            name: this.datePipe.transform(
              item.display_timestamp,
              'yyyy-MM-dd HH:mm:ss'
            )
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

  batchMaunalDetect(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.antiRansomwarePolicyApiService
        .CreateCopyDetection({
          CreateCopyDetectionRequestBody: {
            copyId: this.selectionData[0].uuid
          }
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  batchFeedback(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      ) {
        this.antiBatchFeedback(observer);
      } else {
        this.normalBatchFeedback(observer);
      }
    });
  }

  antiBatchFeedback(observer: Observer<void>) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'feedbackWarningMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get('common_danger_label'),
        lvContent: FeedbackWarningComponent,
        lvComponentParams: {
          feedbackTplLabel: this.i18n.get('explore_detection_feedback_label', [
            map(this.selectionData, 'display_timestamp')
          ])
        },
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as FeedbackWarningComponent;
          const modalIns = modal.getInstance();
          component.isFeedbackChecked$.subscribe(e => {
            modalIns.lvOkDisabled = !e;
          });
        },
        lvCancel: modal => {
          modal.close();
        },
        lvOk: modal => {
          const component = modal.getContentComponent() as FeedbackWarningComponent;
          this.batchOperateService.selfGetResults(
            item => {
              return this.copiesDetectReportService.UpdateCopyDetectionStatus({
                copyId: item.uuid,
                extParameters: {
                  is_security_snap: component.isSecuritySnap
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            map(cloneDeep(this.selectionData), item => {
              return assign(item, {
                name: item.display_timestamp,
                isAsyn: false
              });
            }),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
              this.valid$.next(!!size(this.selectionData));
            }
          );
          modal.close();
        },
        lvAfterClose: () => {
          observer.error('');
          observer.complete();
        }
      }
    });
  }

  normalBatchFeedback(observer: Observer<void>) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_detection_feedback_label', [
        map(this.selectionData, 'display_timestamp')
      ]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.copiesDetectReportService.UpdateCopyDetectionStatus({
              copyId: item.uuid,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(this.selectionData), item => {
            return assign(item, {
              name: item.display_timestamp,
              isAsyn: false
            });
          }),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
            this.valid$.next(!!size(this.selectionData));
          }
        );
      },
      lvAfterClose: () => {
        observer.error('');
        observer.complete();
      }
    });
  }

  feedback(data) {
    if (
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
    ) {
      this.antiFeedback(data);
    } else {
      this.normalFeedBack(data);
    }
  }

  normalFeedBack(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_detection_feedback_label', [
        data[0].display_timestamp
      ]),
      onOK: () => {
        this.copiesDetectReportService
          .UpdateCopyDetectionStatus({
            copyId: data[0].uuid
          })
          .subscribe(res => {
            this.dataTable.fetchData();
          });
      },
      lvAfterClose: () => {
        this.dataTable.fetchData();
      }
    });
  }

  antiFeedback(data: any) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'feedbackWarningMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get('common_danger_label'),
        lvContent: FeedbackWarningComponent,
        lvComponentParams: {
          feedbackTplLabel: this.i18n.get(
            'explore_hyper_detection_feedback_label',
            [data[0].display_timestamp]
          )
        },
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as FeedbackWarningComponent;
          const modalIns = modal.getInstance();
          component.isFeedbackChecked$.subscribe(e => {
            modalIns.lvOkDisabled = !e;
          });
        },
        lvCancel: modal => {
          modal.close();
        },
        lvOk: modal => {
          const component = modal.getContentComponent() as FeedbackWarningComponent;
          const params = {
            copyId: data[0].uuid,
            extParameters: {
              is_security_snap: component.isSecuritySnap
            }
          };
          this.copiesDetectReportService
            .UpdateCopyDetectionStatus(params)
            .subscribe(res => {
              this.dataTable.fetchData();
            });
          modal.close();
        }
      }
    });
  }

  maunalDetect(data) {
    this.antiRansomwarePolicyApiService
      .CreateCopyDetection({
        CreateCopyDetectionRequestBody: { copyId: data[0].uuid }
      })
      .subscribe(res => {
        this.dataTable.fetchData();
      });
  }

  detectionReport(data) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'detection-copy-report',
        lvHeader: this.i18n.get('explore_detection_report_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: DetectionReportComponent,
        lvComponentParams: {
          copyId: data[0].uuid,
          resourceType: this.resourceType
        }, //type
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  deleteCopy(data) {
    let i18nKey = 'common_copy_delete_label';
    const callConfirm = () => {
      this.warningMessageService.create({
        content: this.i18n.get(i18nKey, [
          this.datePipe.transform(
            data[0].display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          )
        ]),
        onOK: () => {
          this.copiesApiService
            .deleteCopyV1CopiesCopyIdDelete({
              copyId: data[0].uuid
            })
            .subscribe(() => this.dataTable.fetchData());
        }
      });
    };
    if (__get(first(data), 'anti_status') === 2) {
      this.copiesDetectReportService
        .ShowDetectionDetails({
          pageNo: 0,
          pageSize: 20,
          resourceId: this.rowData.resource_id,
          conditions: `{"anti_status":[${DataMap.Detection_Copy_Status.uninfected.value}]}`
        })
        .subscribe(res => {
          if (
            res.total === 1 &&
            res.items[0].uuid === __get(first(data), 'uuid')
          ) {
            i18nKey = 'common_copy_uninfected_delete_label';
            callConfirm();
          } else {
            callConfirm();
          }
        });
    } else {
      callConfirm();
    }
  }
}
