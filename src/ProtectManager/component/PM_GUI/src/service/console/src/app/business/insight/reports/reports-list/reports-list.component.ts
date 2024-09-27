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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { AbstractControl, FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  hasReportPermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ReportService,
  ResourceSetApiService,
  ResourceSetType,
  RoleOperationMap,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter as _filter,
  get,
  includes,
  isEmpty,
  isUndefined,
  last,
  map as _map,
  reject,
  replace,
  set,
  size,
  some,
  split,
  trim,
  uniq,
  values
} from 'lodash';
import { map } from 'rxjs/operators';
import { CreateReportComponent } from './create-report/create-report.component';

@Component({
  selector: 'aui-reports-list',
  templateUrl: './reports-list.component.html',
  styleUrls: ['./reports-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ReportsListComponent implements OnInit, AfterViewInit {
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Input() isResourceSet = false;
  @Output() allSelectChange = new EventEmitter<any>();

  name;
  formGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];
  dataMap = DataMap;
  disableDownloadTips = 'X';
  emailErrorTips = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('insight_report_email_max_size_tips_label'),
    invalidName: this.i18n.get('system_error_email_label'),
    sameEmailError: this.i18n.get('system_same_email_error_label')
  };
  isAllSelect = false; // 用来标记是否全选
  allSelectDisabled = true;
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');

  groupCommon = GROUP_COMMON;

  @Input() activeIndex;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('statusTpl', { static: true })
  statusTpl: TemplateRef<any>;
  @ViewChild('typeTpl', { static: true })
  typeTpl: TemplateRef<any>;
  @ViewChild('fileFormatTpl', { static: true })
  fileFormatTpl: TemplateRef<any>;
  @ViewChild('timeUnitTpl', { static: true })
  timeUnitTpl: TemplateRef<any>;
  @ViewChild('scopeTpl', { static: true })
  scopeTpl: TemplateRef<any>;
  @ViewChild('createTimeTpl', { static: true })
  createTimeTpl: TemplateRef<any>;
  @ViewChild('postEmailTpl', { static: true })
  postEmailTpl: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private reportService: ReportService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    public virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private resourceSetService: ResourceSetApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(220);
    if (
      this.isResourceSet &&
      !!this.allSelectionMap[ResourceSetType.Report]?.isAllSelected
    ) {
      this.isAllSelect = true;
    }
    this.initConfig();
    this.initForm();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: RoleOperationMap.report,
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      download: {
        id: 'download',
        permission: OperateItems.Protection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(data) > 20 ||
            size(
              _filter(data, val => {
                return (
                  val.status === DataMap.Report_Status.success.value &&
                  hasReportPermission(val)
                );
              })
            ) !== size(data)
          );
        },
        disabledTips: this.i18n.get('insight_report_download_tips_label'),
        label: this.i18n.get('common_download_label'),
        onClick: data => this.download(data)
      },
      postEmail: {
        id: 'postEmail',
        permission: OperateItems.ModifyProtection,
        disableCheck: data => {
          return (
            !size(data) ||
            size(data) > 20 ||
            size(
              _filter(data, val => {
                return (
                  val.status === DataMap.Report_Status.success.value &&
                  hasReportPermission(val)
                );
              })
            ) !== size(data)
          );
        },
        disabledTips: this.i18n.get('insight_report_send_tips_label'),
        label: this.i18n.get('common_send_email_label'),
        onClick: data => this.postEmail(data)
      },
      delete: {
        id: 'delete',
        permission: OperateItems.DeleteResource,
        disableCheck: data => {
          return (
            !size(data) ||
            size(data) > 100 ||
            some(data, item => !hasReportPermission(item))
          );
        },
        disabledTips: this.i18n.get('insight_report_delete_tips_label'),
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteRes(data)
      }
    };
    this.optItems = cloneDeep(
      getPermissionMenuItem(
        values(reject(opts, item => includes(['create'], item.id)))
      )
    );
    each(this.optItems, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('ID'),
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('insight_report_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Report_Status')
        },
        cellRender: this.statusTpl
      },
      {
        key: 'type',
        name: this.i18n.get('insight_report_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getReportsOptions()
        },
        cellRender: this.typeTpl
      },
      {
        key: 'dataSource',
        name: this.i18n.get('common_data_source_label'),
        width: 400,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'scope',
        name: this.i18n.get('insight_report_scope_label'),
        cellRender: this.scopeTpl
      },
      {
        key: 'timeUnit',
        hidden: true,
        name: this.i18n.get('insight_report_frequency_label'),
        cellRender: this.timeUnitTpl
      },
      {
        key: 'fileFormat',
        hidden: true,
        name: this.i18n.get('insight_report_format_label'),
        cellRender: this.fileFormatTpl
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_create_time_label'),
        sort: true,
        cellRender: this.createTimeTpl
      },
      {
        key: 'operation',
        width: 144,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];

    if (this.isResourceSet) {
      cols.pop();
    }

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns: cols,
        rows: this.isDetail
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          if (this.isResourceSet) {
            set(this.allSelectionMap, ResourceSetType.Report, {
              data: this.selectionData
            });
            this.allSelectChange.emit();
          }
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };

    if (this.isResourceSet) {
      delete this.tableConfig.table.autoPolling;
    }

    this.optsConfig = getPermissionMenuItem([
      opts.create,
      opts.download,
      opts.postEmail,
      opts.delete
    ]);
  }

  allSelect(turnPage?) {
    // 用于资源集全选资源
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.Report, { isAllSelected });
    this.selectionData = isAllSelected ? [...this.tableData.data] : [];
    each(this.tableData.data, item => {
      item.disabled = isAllSelected;
    });
    this.dataTable.setSelections(cloneDeep(this.selectionData));
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  private getReportsOptions() {
    let options = this.dataMapService.toArray('Report_Type').filter(item => {
      return includes(
        [
          DataMap.Report_Type.storageSpace.value,
          DataMap.Report_Type.backupJob.value,
          DataMap.Report_Type.recoveryJob.value,
          DataMap.Report_Type.recoveryDrillJob.value,
          DataMap.Report_Type.tapeUsed.value,
          DataMap.Report_Type.resourceUsed.value,
          DataMap.Report_Type.agentResourceUsed.value
        ],
        item.value
      );
    });
    if (this.appUtilsService.isDecouple) {
      options = reject(
        options,
        item => item.value === DataMap.Report_Type.storageSpace.value
      );
    }
    return options;
  }

  initForm() {
    this.formGroup = this.fb.group({
      emailAddress: new FormControl('')
    });
  }

  vaildEmail() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const emailArr = split(control.value, ',');
      let invalidInfo;

      if (emailArr.length !== uniq(emailArr).length) {
        invalidInfo = { sameEmailError: { value: control.value } };
      }

      if (size(emailArr) > 5) {
        invalidInfo = { invalidMaxLength: { value: control.value } };
      }

      each(emailArr, item => {
        if (!CommonConsts.REGEX.email.test(item)) {
          invalidInfo = { invalidName: { value: control.value } };
        }
      });

      return invalidInfo ? invalidInfo : null;
    };
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });

    set(
      this.dataTable.filterMap,
      'paginator.pageIndex',
      CommonConsts.PAGE_START
    );
    this.dataTable.fetchData();
  }

  create() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'create-report',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateReportComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateReportComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(formGroupStatus => {
            modalIns.lvOkDisabled = formGroupStatus === 'INVALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateReportComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);

                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        },
        lvCancel: modal => {}
      })
    );
  }

  download(data) {
    each(data, item => {
      this.reportService
        .downloadReportUsingGETResponse({
          reportId: item.uuid,
          memberEsn: item.esn
        })
        .subscribe({
          next: res => {
            const bf = new Blob([res.body], {
              type: 'application/octet-stream'
            });
            const fileName = replace(
              last(split(res.headers.get('Content-Disposition'), 'filename=')),
              /\"/g,
              ''
            );
            this.appUtilsService.downloadFile(decodeURI(fileName), bf);
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          }
        });
    });
  }

  postEmail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'report-post-email-modal',
      lvType: 'drawer',
      lvHeader: this.i18n.get('common_send_email_label'),
      lvWidth: MODAL_COMMON.normalWidth + 50,
      lvContent: this.postEmailTpl,
      lvComponentParams: {},
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const modalIns = modal.getInstance();
        this.formGroup
          .get('emailAddress')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildEmail()
          ]);
        this.formGroup.statusChanges.subscribe(formGroupStatus => {
          modalIns.lvOkDisabled = formGroupStatus === 'INVALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent();
          const emailList = split(this.formGroup.value.emailAddress, ',');
          let datas = [];

          each(emailList, email => {
            datas = [
              ...datas,
              ..._map(data, item => {
                return {
                  ...item,
                  email,
                  report: { reportId: item.uuid, receiveEmail: email },
                  memberEsn: item?.esn || '',
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                };
              })
            ];
          });

          this.batchOperateService.selfGetResults(
            item => {
              return this.reportService.sendReportUsingPOST(item);
            },
            cloneDeep(datas),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
              resolve(true);
              modal.close();
            },
            this.i18n.get('common_name_label'),
            true,
            NaN,
            [
              {
                label: this.i18n.get('system_recipient_email_label'),
                key: 'email'
              }
            ]
          );
        });
      },
      lvAfterClose: modal => {
        this.formGroup.get('emailAddress').clearValidators();
        this.formGroup.get('emailAddress').setValue('');
      }
    });
  }

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('insight_report_delete_label'),
        onOK: () => {
          this.reportService
            .deleteReportUsingDELETE({
              reportId: data[0].uuid,
              memberEsn: data[0].esn || ''
            })
            .subscribe(() => {
              this.selectionData = reject(
                this.dataTable.getAllSelections(),
                item => {
                  return item.uuid === data[0].uuid;
                }
              );
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('insight_report_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.reportService.deleteReportUsingDELETE({
                reportId: item.uuid,
                memberEsn: item.esn || '',
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(this.selectionData),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          );
        }
      });
    }
  }

  getData(filters: Filters, args) {
    const params = {
      startPage: filters.paginator.pageIndex + 1 || CommonConsts.PAGE_START + 1,
      pageSize: filters.paginator.pageSize || CommonConsts.PAGE_SIZE,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (this.isDetail) {
      assign(params, {
        resourceSetId: this.data[0].uuid
      });
    }

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.name) {
        assign(params, {
          name: conditionsTemp.name[1]
        });
      }

      if (conditionsTemp.status) {
        conditionsTemp.status.shift();
        assign(params, {
          statusList: [...conditionsTemp.status]
        });
      }

      if (conditionsTemp.type) {
        conditionsTemp.type.shift();
        assign(params, {
          typeList: [...conditionsTemp.type]
        });
      }

      if (conditionsTemp.dataSource) {
        assign(params, {
          externalClusterName: conditionsTemp.dataSource[1]
        });
      }
    }

    if (!!size(filters.sort)) {
      assign(params, {
        orderBy: filters.sort.key,
        orderType: filters.sort.direction
      });
    }

    this.reportService
      .queryReportsUsingGET(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              scope: [item.startTime, item.endTime],
              dataSource: get(item, 'externalClusterName'),
              subType: 'report'
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };

        if (this.isResourceSet) {
          this.allSelectDisabled = res.totalCount === 0;
          if (!!this.allSelectionMap[ResourceSetType.Report]?.isAllSelected) {
            this.allSelect(false);
          }
          this.dataCallBack();
        }

        this.cdr.detectChanges();
      });
  }

  dataCallBack() {
    // 用于资源集各类情况下的资源回显
    if (!isEmpty(this.allSelectionMap[ResourceSetType.Report]?.data)) {
      // 重新进入时回显选中的数据
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.Report].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }

    if (
      !!this.data &&
      isEmpty(this.allSelectionMap[ResourceSetType.Report]?.data) &&
      !this.isDetail
    ) {
      this.getSelectedData();
    }
  }

  getSelectedData() {
    // 用于修改时回显
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.Report,
      type: ResourceSetType.Report
    };
    this.resourceSetService.QueryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.Report, {
        data: _map(res, item => {
          return { uuid: item };
        })
      });
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.Report].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
      this.allSelectChange.emit();
    });
  }
}
