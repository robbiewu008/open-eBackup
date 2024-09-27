import {
  Component,
  OnInit,
  ChangeDetectionStrategy,
  AfterViewInit,
  ViewChild,
  ChangeDetectorRef,
  TemplateRef
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  Filters,
  TableCols
} from 'app/shared/components/pro-table';
import {
  I18NService,
  DataMapService,
  WarningMessageService,
  MODAL_COMMON,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  getPermissionMenuItem,
  OperateItems,
  GROUP_COMMON,
  GlobalService,
  RoleOperationMap,
  hasResourcePermission,
  getLabelList
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  first,
  toString,
  isUndefined,
  isEmpty,
  assign,
  size,
  cloneDeep,
  reject,
  trim,
  each,
  map as _map,
  includes,
  some
} from 'lodash';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { DetailComponent } from './detail/detail.component';
import { map } from 'rxjs/operators';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { RegisterComponent } from './register/register.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-cluster',
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClusterComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  optItems = [];
  selectionData = [];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        onClick: () => this.register(),
        popoverContent: this.i18n.get('protection_guide_cluster_tip_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyProtection,
        onClick: data => {
          this.register(first(data));
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteResource,
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        }
      },
      {
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
      {
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
    ];

    const optsItem = getPermissionMenuItem(opts);
    this.optsConfig = reject(optsItem, item => {
      return includes(
        ['resourceAuth', 'resourceReclaiming', 'modify'],
        item.id
      );
    });
    this.optItems = cloneDeep(reject(optsItem, { id: 'register' }));

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => this.getDetail(data)
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
        key: 'clusterType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Mysql_Cluster_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Mysql_Cluster_Type')
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
        compareWith: 'uuid',
        columns: cols,
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters, args) => {
          this.getData(filters, args);
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

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    const defaultConditions = {
      subType: DataMap.Resource_Type.MySQLCluster.value
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

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              clusterType: item.extendInfo?.clusterType,
              showLabelList: showList,
              hoverLabelList: hoverList
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
        this.cdr.detectChanges();
      });
  }

  getDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'mysql_cluster_detail',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: DetailComponent,
      lvComponentParams: {
        data
      },
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ],
      lvAfterClose: () => {
        // 关闭详情框，发布全局消息
        this.globalService.emitStore({
          action: 'detailModalClose',
          state: true
        });
      }
    });
  }

  register(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_register_label'),
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: RegisterComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as RegisterComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(status => {
          modalIns.lvOkDisabled = status === 'INVALID';
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as RegisterComponent;
          content.onOK().subscribe({
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

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_label', [
        toString(_map(datas, 'name'))
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: datas[0].uuid
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid
              });
            },
            datas,
            () => {
              this.dataTable.fetchData();
            }
          );
        }
      }
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
}
