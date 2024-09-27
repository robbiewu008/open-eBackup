import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OpHcsServiceApiService,
  OperateItems,
  getPermissionMenuItem
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, filter, first, isArray, isEmpty, values } from 'lodash';
import { StorResourceNodeComponent } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node.component';

@Component({
  selector: 'aui-hcs-storage',
  templateUrl: './hcs-storage.component.html',
  styleUrls: ['./hcs-storage.component.less']
})
export class HcsStorageComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];

  scaleoutLabel = this.i18n.get('protection_database_type_block_storage_label');
  centralizedLabel = this.i18n.get('common_san_storage_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private opHcsServiceApiService: OpHcsServiceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(220);
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      add: {
        id: 'add',
        type: 'primary',
        permission: OperateItems.AddHcsStorage,
        label: this.i18n.get('common_add_label'),
        onClick: () => this.add()
      },
      delete: {
        id: 'modify',
        permission: OperateItems.AddHcsStorage,
        label: this.i18n.get('common_modify_label'),
        onClick: ([data]) => this.add(data)
      }
    };
    this.optsConfig = getPermissionMenuItem([opts.add]);
    this.tableConfig = {
      table: {
        compareWith: 'ipList',
        columns: [
          {
            key: 'storageType',
            name: this.i18n.get('common_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('hcsStorageType')
            }
          },
          {
            key: 'ipList',
            name: this.i18n.get('common_management_ip_label')
          },
          {
            key: 'port',
            name: this.i18n.get('common_port_label')
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
                items: getPermissionMenuItem(
                  filter(values(opts), item => item.id === 'modify')
                )
              }
            }
          }
        ],
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        trackByFn: (_, item) => {
          return item.ipList;
        }
      }
    };
  }

  getData(filters: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    this.opHcsServiceApiService
      .getAllStorageForHcs(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  add(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-hcs-storage',
        lvWidth: MODAL_COMMON.normalWidth + 150,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: StorResourceNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data: data
            ? {
                ip: data.ipList || data.ip,
                port: data.port,
                username: data.username,
                enableCert: data.enableCert,
                certName: data.certName,
                certSize: data.certSize,
                crlName: data.crlName,
                crlSize: data.crlSize,
                storageType:
                  data.storageType === '1'
                    ? this.scaleoutLabel
                    : this.centralizedLabel
              }
            : null,
          subType: DataMap.Resource_Type.HCS.value
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as StorResourceNodeComponent;
            content.onOK().subscribe(res => {
              const params = {
                storageType: `${res.storageType}`,
                ip: <string>first(res.ip),
                ipList: isArray(res.ip) ? res.ip.join(',') : res.ip,
                port: res.port,
                username: res.username,
                password: res.password,
                enableCert: res.enableCert,
                certification: res.certification,
                revocationList: res.revocationList,
                certName: res.certName,
                certSize: res.certSize,
                crlName: res.crlName,
                crlSize: res.crlSize
              };
              this.opHcsServiceApiService
                .initStorageResources({ storageList: [params] })
                .subscribe(
                  () => resolve(true),
                  () => resolve(false)
                );
            });
          });
        }
      })
    );
  }
}
