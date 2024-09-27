import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  AntiRansomwareNetworkApiService,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  SystemApiService,
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
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { assign, each, isEmpty, isUndefined, size, trim } from 'lodash';
import { BindRelationComponent } from './bind-relation/bind-relation.component';
import { DetailComponent } from './detail/detail.component';

@Component({
  selector: 'aui-network-config',
  templateUrl: './network-config.component.html',
  styleUrls: ['./network-config.component.less']
})
export class NetworkConfigComponent implements OnInit {
  FormGroup: FormGroup;
  networkTableConfig: TableConfig;
  networkData: TableData;
  displayData = [];
  oringinalData = [];
  selectionData = [];

  deleteBtnDisabled = true;

  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  searchPlaceHolder = this.i18n.get('common_search_type_label', ['']);

  options: ProButton[] = [
    {
      id: 'modify',
      label: this.i18n.get('system_add_nic_relation_label'),
      onClick: data => {
        this.modify(data);
      }
    }
  ];

  @ViewChild('typeTpl', { static: true }) typeTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public drawModalService: DrawModalService,
    public i18n: I18NService,
    public cookieService: CookieService,
    public baseUtilService: BaseUtilService,
    private cdr: ChangeDetectorRef,
    private fb: FormBuilder,
    private systemApiService: SystemApiService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private dataMapService: DataMapService,
    private antiRansomwareNetworkApiService: AntiRansomwareNetworkApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  getNetwork(filters: Filters, args?: { isAutoPolling: any }) {
    const params = {
      CreateDemoInfoRequestBody: {}
    };

    if (args && args.isAutoPolling) {
      assign(params, {
        akLoading: false
      });
    }

    this.antiRansomwareNetworkApiService
      .QueryNetworkInfo(params)
      .subscribe(res => {
        this.networkData = {
          data: res.records,
          total: size(res.records)
        };

        this.total = size(res.records);
      });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'iface',
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
            click: data => this.getDetail(data)
          }
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        cellRender: this.typeTpl
      },
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label')
      },
      {
        key: 'netmask',
        name: this.i18n.get('common_mask_ip_label')
      },
      {
        key: 'parentName',
        name: this.i18n.get('system_belong_label')
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.options
          }
        }
      }
    ];

    this.networkTableConfig = {
      table: {
        async: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false,
        autoPolling: 3 * CommonConsts.TIME_INTERVAL,
        compareWith: 'iface',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: false
        },
        fetchData: (filter: Filters, args) => {
          this.getNetwork(filter, args);
        },
        selectionChange: data => {
          this.selectionData = data;
          if (data) {
            this.deleteBtnDisabled = false;
          }
        },
        trackByFn: (index, item) => {
          return item.iface;
        }
      }
    };
  }

  modify(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'bindRelation',
        lvHeader: this.i18n.get('system_add_nic_relation_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: BindRelationComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data: data[0]
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
                this.cdr.detectChanges();
                this.dataTable.fetchData();
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

  getDetail(data) {
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.iface,
      lvModalKey: 'openGauss_cluster_detail',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 200
        : MODAL_COMMON.normalWidth + 100,
      lvContent: DetailComponent,
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

  search(value) {
    this.dataTable.filterChange({
      filterMode: 'contains',
      caseSensitive: false,
      key: 'iface',
      value: [value]
    });
  }

  refreshNetworkInfo() {
    this.dataTable.fetchData();
  }
}
