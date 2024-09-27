import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AntiRansomwareNetworkApiService,
  CommonConsts,
  I18NService,
  MODAL_COMMON,
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
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, cloneDeep, each, isNumber, isUndefined, size } from 'lodash';
import { BindRelationComponent } from '../bind-relation/bind-relation.component';

@Component({
  selector: 'aui-detail',
  templateUrl: './detail.component.html',
  styleUrls: ['./detail.component.less']
})
export class DetailComponent implements OnInit {
  data;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE / 2;
  tenantTableConfig: TableConfig;
  tenantTableData: TableData;
  selectionTenant = [];
  deleteDisableBtn = true;

  columns = [
    {
      key: 'iface',
      label: this.i18n.get('common_name_label'),
      content: ''
    },
    {
      key: 'type',
      label: this.i18n.get('common_type_label'),
      content: ''
    },
    {
      key: 'ip',
      label: this.i18n.get('common_ip_address_label'),
      content: ''
    },
    {
      key: 'netmask',
      label: this.i18n.get('common_mask_ip_label'),
      content: ''
    },
    {
      key: 'macAddress',
      label: this.i18n.get('common_mac_address_label'),
      content: ''
    }
  ];
  options: ProButton[] = [
    {
      id: 'modify',
      label: this.i18n.get('system_add_nic_relation_label'),
      onClick: data => {
        this.modify();
      }
    }
  ];

  @ViewChild('tenantTable', { static: false }) tenantTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private modal: ModalRef,
    public drawModalService: DrawModalService,
    private antiRansomwareNetworkApiService: AntiRansomwareNetworkApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.initTable();
  }

  ngAfterViewInit(): void {
    this.tenantTable.fetchData();
  }

  initConfig() {
    for (let item of this.columns) {
      if (item.key === this.columns[1].key) {
        if (this.data.parentName) {
          item.content = this.i18n.get('VLAN');
        } else {
          item.content = this.i18n.get('system_service_network_label');
        }
      } else {
        if (this.data[item.key]) {
          item.content = this.data[item.key];
        } else {
          item.content = '--';
        }
      }
    }
  }

  initTable() {
    this.tenantTableConfig = {
      table: {
        columns: [
          {
            key: 'deviceName',
            name: this.i18n.get('protection_equipment_name_label')
          },
          {
            key: 'esn',
            name: 'ESN',
            hidden: true
          },
          {
            key: 'vstoreName',
            name: this.i18n.get('common_tenant_label')
          },
          {
            key: 'vstoreId',
            name: this.i18n.get('system_vstore_id_label'),
            hidden: true
          },
          {
            key: 'gateway',
            name: this.i18n.get('common_gateway_label')
          },
          {
            key: 'operation',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 1,
                items: [
                  {
                    id: 'delete',
                    label: this.i18n.get('system_delete_nic_relation_label'),
                    onClick: data => {
                      this.warningMessageService.create({
                        content: this.i18n.get(
                          'system_delete_nic_relation_tip_label',
                          [data[0].vstoreName]
                        ),
                        width: 700,
                        onOK: () => {
                          this.antiRansomwareNetworkApiService
                            .BatchDeleteNetWorkRelation({
                              relationIds: [data[0].id]
                            })
                            .subscribe(res => {
                              this.selectionTenant = [];
                              this.tenantTable.setSelections([]);
                              this.deleteDisableBtn = true;
                              this.cdr.detectChanges();
                              this.tenantTable.fetchData();
                            });
                        }
                      });
                    }
                  }
                ]
              }
            }
          }
        ],
        async: true,
        size: 'small',
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        virtualScroll: true,
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '420px' },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true,
          keepRadioLogic: true
        },
        fetchData: (filter: Filters, args) => {
          this.getDetail(filter, args);
        },
        selectionChange: data => {
          this.selectionTenant = data;
          this.deleteDisableBtn = !data.length;
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      },
      pagination: {
        mode: 'simple',
        pageSizeOptions: [10, 20, 50],
        showPageSizeOptions: true,
        winTablePagination: true,
        pageSize: this.pageSize
      }
    };
  }

  getDetail(filters: Filters, args) {
    const params = {
      pageNo: `${filters.paginator.pageIndex}`,
      pageSize: `${filters.paginator.pageSize}`,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true,
      iFace: this.data.iface
    };

    this.antiRansomwareNetworkApiService
      .QueryIFaceDetail(params)
      .subscribe(res => {
        this.tenantTableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  modify() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'bindRelation',
        lvHeader: this.i18n.get('system_add_nic_relation_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: BindRelationComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data: this.data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BindRelationComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as BindRelationComponent;
            content.modify().subscribe(
              () => {
                resolve(true);
                this.selectionTenant = [];
                this.tenantTable.setSelections([]);
                this.deleteDisableBtn = true;
                this.cdr.detectChanges();
                this.tenantTable.fetchData();
              },
              () => {
                resolve(false);
              }
            );
          });
        }
      })
    );
  }

  delete() {
    const params = [];
    const nameTip = [];
    const promises = [];
    each(this.selectionTenant, item => {
      params.push(item.id);
      nameTip.push(item.vstoreName);
    });
    this.warningMessageService.create({
      content: this.i18n.get('system_delete_nic_relation_tip_label', [nameTip]),
      width: 700,
      onOK: () => {
        promises.push(
          new Promise((resolve, reject) => {
            this.antiRansomwareNetworkApiService
              .BatchDeleteNetWorkRelation({
                relationIds: params
              })
              .subscribe(
                res => {
                  resolve(res);
                  this.deleteDisableBtn = true;
                },
                err => {
                  reject(err);
                }
              );
          })
        );
        Promise.all(promises).then(() => {
          this.tenantTable.fetchData();
        });
      }
    });
  }

  refresh() {
    this.tenantTable.fetchData();
  }
}
