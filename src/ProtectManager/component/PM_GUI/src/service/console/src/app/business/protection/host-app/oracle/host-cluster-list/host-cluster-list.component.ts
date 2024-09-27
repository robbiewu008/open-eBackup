import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ClusterEnvironment,
  CommonConsts,
  DataMap,
  DataMapService,
  getLabelList,
  getPermissionMenuItem,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RoleOperationMap,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  isEmpty,
  isUndefined,
  size,
  some,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { RegisterComponent } from './register/register.component';
import { SummaryComponent } from './summary/summary.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-host-cluster-list',
  templateUrl: './host-cluster-list.component.html',
  styleUrls: ['./host-cluster-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class HostClusterListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData: any = [];
  name;

  groupCommon = GROUP_COMMON;

  registerTipShow = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('clusterTypeTpl', { static: true }) clusterTypeTpl: TemplateRef<
    any
  >;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    public warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400, 3);
    this.initTableConfig();
  }

  initTableConfig() {
    const opts: { [key: string]: ProButton } = {
      register: {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        popoverContent: this.i18n.get('protection_guide_cluster_tip_label'),
        popoverShow:
          USER_GUIDE_CACHE_DATA.active &&
          USER_GUIDE_CACHE_DATA.appType === DataMap.Resource_Type.oracle.value,
        onClick: () => this.register()
      },
      modify: {
        id: 'modify',
        permission: OperateItems.ModifyHost,
        label: this.i18n.get('common_modify_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.protectedResourceApiService
            .ShowResource({ resourceId: data[0]?.uuid })
            .subscribe(res => this.register(res));
        }
      },
      delete: {
        id: 'delete',
        permission: OperateItems.DeleteDatabase,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return isEmpty(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => this.delete(data)
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
    this.optsConfig = getPermissionMenuItem([
      opts.register,
      opts.delete,
      opts.addTag,
      opts.removeTag
    ]);
    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: [
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
                  this.getDetail(data);
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
              options: this.dataMapService.toArray(
                'resource_LinkStatus_Special'
              )
            },
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('resource_LinkStatus_Special')
            }
          },
          {
            key: 'clusterType',
            name: this.i18n.get('common_type_label'),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray('oracleClusterType')
            },
            cellRender: this.clusterTypeTpl
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
            name: this.i18n.get('common_operation_label'),
            hidden: 'ignoring',
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 1,
                items: getPermissionMenuItem([
                  opts.modify,
                  opts.delete,
                  opts.addTag,
                  opts.removeTag
                ])
              }
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        fetchData: (filters: Filters, args) => {
          this.getData(filters, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
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
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable?.fetchData();
      }
    });
  }

  getData(filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: [ClusterEnvironment.oralceClusterEnv]
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
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

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        // 获取标签数据
        const { showList, hoverList } = getLabelList(item);
        assign(item, {
          showLabelList: showList,
          hoverLabelList: hoverList
        });
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
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
    this.dataTable.fetchData();
  }

  register(rowData?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(rowData)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterComponent,
        lvOkDisabled: isEmpty(rowData),
        lvComponentParams: {
          rowData
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Observable((observer: Observer<void>) => {
            const content = modal.getContentComponent() as RegisterComponent;
            content.onOK().subscribe(
              () => {
                this.dataTable.fetchData();
                observer.next();
                observer.complete();
              },
              error => {
                observer.error(error);
                observer.complete();
              }
            );
          });
        }
      })
    );
  }

  delete(datas: any[]) {
    if (datas.length > 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedEnvironmentApiService.DeleteProtectedEnvironment(
                {
                  envId: item.uuid,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
            },
            cloneDeep(datas),
            () => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            }
          );
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_nas_share_delete_label'),
        onOK: () => {
          this.protectedEnvironmentApiService
            .DeleteProtectedEnvironment({
              envId: datas[0]?.uuid
            })
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      });
    }
  }

  getDetail(resource) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: resource.uuid })
      .subscribe(res => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'kubernetes-cluster-detail',
            lvWidth: MODAL_COMMON.normalWidth + 100,
            lvHeader: resource.name,
            lvContent: SummaryComponent,
            lvComponentParams: {
              rowItem: { ...res }
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ],
            lvAfterClose: () => {
              if (this.dataTable) {
                this.dataTable.setActiveItemEmpty();
              }
            }
          })
        );
      });
  }
}
