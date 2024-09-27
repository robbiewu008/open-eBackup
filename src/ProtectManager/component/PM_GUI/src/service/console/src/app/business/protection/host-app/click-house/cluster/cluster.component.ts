import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { MessageService } from '@iux/live';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { RegisterComponent } from './register/register.component';
import { map } from 'rxjs/operators';
import {
  Component,
  OnInit,
  AfterViewInit,
  ChangeDetectorRef,
  ViewChild,
  TemplateRef
} from '@angular/core';
import {
  ProTableComponent,
  TableCols,
  TableData,
  TableConfig,
  Filters
} from 'app/shared/components/pro-table';
import {
  OperateItems,
  I18NService,
  DataMap,
  DataMapService,
  GlobalService,
  MODAL_COMMON,
  ResourceType,
  getTableOptsItems,
  CommonConsts,
  extendSlaInfo,
  getPermissionMenuItem,
  WarningMessageService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  GROUP_COMMON,
  RoleOperationMap,
  hasResourcePermission,
  getLabelList
} from 'app/shared';
import {
  filter,
  each,
  size,
  first,
  isEmpty,
  cloneDeep,
  values,
  reject,
  isNil,
  assign,
  trim,
  eq,
  includes,
  mapValues,
  omit,
  get,
  some
} from 'lodash';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-cluster',
  templateUrl: './cluster.component.html',
  styles: ['']
})
export class ClusterComponent implements OnInit, AfterViewInit {
  selectionData;
  opts: ProButton[];
  optsConfig: ProButton[];
  optItems: ProButton[];
  tableConfig: TableConfig;
  tableData: TableData;
  searchKey: string;

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable') dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private detailService: ResourceDetailService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnInit() {
    this.initConfig();

    this.globalService.getState('registerClickHouseCluster').subscribe(() => {
      this.dataTable.fetchData();
    });
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }
  initConfig() {
    const optItems: { [key: string]: ProButton } = {
      register: {
        id: 'register',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        type: 'primary',
        onClick: () => this.register()
      },
      rescan: {
        id: 'rescan',
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_rescan_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data[0].rootUuid
            })
            .subscribe(() => {
              this.dataTable.fetchData();
            });
        }
      },
      connectivityTest: {
        id: 'connectivityTest',
        divide: true,
        permission: OperateItems.ManualBackup,
        label: this.i18n.get('protection_connectivity_test_label'),
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => this.connectTest(data)
      },
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterNasShare,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.register(first(data));
        },
        disableCheck: data => {
          return (
            size(
              filter(data, item => {
                return (
                  item['rootUuid'] === first(data)['rootUuid'] &&
                  item['shareMode'] === first(data)['shareMode'] &&
                  hasResourcePermission(item)
                );
              })
            ) !== size(data) || !size(data)
          );
        }
      },
      deleteResource: {
        id: 'deleteResource',
        permission: OperateItems.DeleteResource,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  isEmpty(val.sla_id) &&
                  val.protection_status !==
                    DataMap.Protection_Status.creating.value &&
                  hasResourcePermission(val)
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.deleteRes(data)
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

    this.opts = cloneDeep(
      getPermissionMenuItem(values(reject(optItems, { id: 'register' })))
    );

    each(this.opts, item => {
      if (item.disabledTips) {
        item.disabledTips = '';
      }
    });

    const cols: TableCols[] = [
      {
        key: 'name',
        sort: true,
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: (data: any) => {
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
          options: this.dataMapService.toArray('clickHouse_cluster_status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('clickHouse_cluster_status')
        }
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
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
            items: this.opts
          }
        }
      }
    ];

    this.optsConfig = getPermissionMenuItem([
      optItems.register,
      optItems.deleteResource,
      optItems.addTag,
      optItems.removeTag
    ]);

    this.optItems = cloneDeep(getPermissionMenuItem(values(this.opts)));

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

  register(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'reigster-click-house-cluster',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvHeader: isEmpty(data)
        ? this.i18n.get('common_register_label')
        : this.i18n.get('common_modify_label'),
      lvContent: RegisterComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        item: data
      }
    });
  }

  deleteRes(data: object) {
    if (eq(size(data), 1)) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resource_delete_label', [
          data[0].name
        ]),
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
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('protection_resources_delete_label'),
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

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.searchKey)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  getResourceDetail(res) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [DataMap.Application_Type.ClickHouse.value],
          uuid: [['~~'], res.uuid]
        })
      })
      .pipe(
        map(result => {
          return first(result.records) || {};
        })
      )
      .subscribe(item => {
        if (!item || isEmpty(item)) {
          this.messageService.error(
            this.i18n.get('common_resource_not_exist_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'resNotExistMesageKey'
            }
          );
          return;
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        extendSlaInfo(item);
        this.detailService.openDetailModal(
          DataMap.Resource_Type.ClickHouseCluster.value,
          {
            data: assign(
              omit(cloneDeep(res), ['sla_id']),
              item,
              {
                optItems: getTableOptsItems(
                  cloneDeep(this.optItems),
                  assign(omit(cloneDeep(res), ['sla_id']), item),
                  this
                )
              },
              {
                optItemsFn: v => {
                  return getTableOptsItems(cloneDeep(this.optItems), v, this);
                }
              }
            )
          }
        );
      });
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading: !isNil(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.ClickHouse.value,
      type: ResourceType.CLUSTER
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.equipment) {
        assign(conditionsTemp, {
          environment: {
            name: conditionsTemp.equipment
          }
        });
        delete conditionsTemp.equipment;
      }
      if (conditionsTemp.equipmentType) {
        if (isEmpty(conditionsTemp.environment)) {
          assign(conditionsTemp, {
            environment: {
              subType: conditionsTemp.equipmentType
            }
          });
        } else {
          assign(conditionsTemp.environment, {
            subType: conditionsTemp.equipmentType
          });
        }
        delete conditionsTemp.equipmentType;
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
              sub_type: item.subType,
              parentName: item.environment?.name,
              auth_status: DataMap.Verify_Status.true.value,
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            extendSlaInfo(item);
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

  connectTest(data) {
    this.protectedResourceApiService
      .CheckProtectedResource({
        resourceId: first(data)['uuid']
      })
      .subscribe(res => {
        let returnRes;
        try {
          returnRes = JSON.parse(res);
        } catch (error) {
          returnRes = [];
        }
        const idx = returnRes.findIndex(item => item.code !== 0);
        if (idx !== -1) {
          this.messageService.error(this.i18n.get(returnRes[idx].code), {
            lvMessageKey: 'errorKey',
            lvShowCloseButton: true
          });
        } else {
          this.messageService.success(
            this.i18n.get('common_operate_success_label'),
            {
              lvMessageKey: 'successKey',
              lvShowCloseButton: true
            }
          );
        }
      });
  }
}
