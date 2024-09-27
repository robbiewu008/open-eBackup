import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  OnDestroy,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  GlobalService,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  RoleOperationMap,
  WarningMessageService,
  getLabelList,
  getPermissionMenuItem
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  first,
  includes,
  isEmpty,
  isUndefined,
  mapValues,
  reject,
  size,
  some,
  trim,
  values
} from 'lodash';
import { map, takeUntil } from 'rxjs/operators';
import { RegisterObjectStorageComponent } from './register-object-storage/register-object-storage.component';
import { SummaryComponent } from './summary/summary.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-object-storage',
  templateUrl: './object-storage.component.html',
  styleUrls: ['./object-storage.component.less']
})
export class ObjectStorageComponent implements OnInit, OnDestroy {
  tableConfig: TableConfig;
  tableData: TableData;
  optItems;
  selectionData = [];
  name;
  optsConfig;
  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;

  destroy$ = new Subject();
  registerTipShow = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private globalService: GlobalService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.getUserGuideState();
    this.showRegisterTip();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showRegisterTip();
      });
  }

  showRegisterTip() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.showTips) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      connectivityTest: {
        id: 'connectivityTest',
        label: this.i18n.get('protection_connectivity_test_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.check(data);
        }
      },
      modify: {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.register(data);
        }
      },
      delete: {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.delete(data);
        }
      },
      addTag: {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      removeTag: {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data);
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    };
    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));
    this.optsConfig = getPermissionMenuItem([
      opts.delete,
      opts.addTag,
      opts.removeTag
    ]);

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
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
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'storageType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('objectStorageType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('objectStorageType')
        }
      },
      {
        key: 'endpoint',
        name: 'Endpoint',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'protocol',
        name: this.i18n.get('common_protocol_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('protocolType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('protocolType')
        }
      },
      {
        key: 'AK',
        name: 'AK',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.resourceTagTpl
      },
      {
        key: 'operation',
        width: 130,
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

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.ObjectStorage.value
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (!!conditionsTemp.AK) {
        assign(conditionsTemp, {
          ak: conditionsTemp.AK
        });
        delete conditionsTemp.AK;
      }
      if (!!conditionsTemp.protocol) {
        assign(conditionsTemp, {
          useHttps: conditionsTemp.protocol
        });
        delete conditionsTemp.protocol;
      }
      if (conditionsTemp.labelList) {
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
          }
        });
        delete conditionsTemp.labelList;
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, tmp => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(tmp);
            assign(tmp, {
              AK: tmp.extendInfo?.ak,
              SK: tmp.extendInfo?.sk,
              storageType: Number(tmp.extendInfo.storageType),
              protocol: tmp.extendInfo.useHttps,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  register(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'register-object-storage',
        lvWidth: MODAL_COMMON.smallModal,
        lvHeader: data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_register_label'),
        lvContent: RegisterObjectStorageComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterObjectStorageComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                if (
                  !isEmpty(data) &&
                  includes(
                    mapValues(this.drawModalService.modals, 'key'),
                    'detail-modal'
                  )
                ) {
                  this.getResourceDetail(content.rowData);
                } else {
                  this.dataTable.fetchData();
                }
              },
              error: () => resolve(false)
            });
          });
        },
        lvCancel: modal => {
          const content = modal.getContentComponent() as RegisterObjectStorageComponent;
          if (
            !isEmpty(data) &&
            includes(
              mapValues(this.drawModalService.modals, 'key'),
              'detail-modal'
            )
          ) {
            this.getResourceDetail(content.rowData);
          }
        }
      })
    );
  }

  search() {
    this.dataTable.filterChange({
      caseSensitive: false,
      filterMode: 'contains',
      key: 'name',
      value: trim(this.name)
    });
  }

  getResourceDetail(data?) {
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'object_storage_detail',
      lvWidth: 800,
      lvContent: SummaryComponent,
      lvComponentParams: {
        data: data
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
    });
  }

  check(data) {
    this.protectedResourceApiService
      .CheckProtectedResource({ resourceId: data[0].uuid })
      .subscribe(res => {
        this.messageService.success(this.i18n.get('job_status_success_label'), {
          lvMessageKey: 'successKey',
          lvShowCloseButton: true
        });
        this.dataTable.fetchData();
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          ) &&
          size(data) === 1
        ) {
          this.getResourceDetail(first(data));
        }
      });
  }

  delete(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: data[0].uuid
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
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                )
              ) {
                this.drawModalService.destroyModal('detail-modal');
              }
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid,
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
}
