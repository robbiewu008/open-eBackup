import {
  Component,
  OnInit,
  Output,
  EventEmitter,
  ViewChild,
  AfterViewInit,
  ChangeDetectorRef
} from '@angular/core';
import {
  CapacityCalculateLabel,
  CommonConsts,
  CAPACITY_UNIT,
  DataMapService,
  I18NService,
  WarningMessageService,
  LocalStorageApiService,
  LANGUAGE,
  BaseUtilService,
  MODAL_COMMON,
  StoragesApiService,
  CookieService,
  DataMap
} from 'app/shared';
import {
  BackupClustersApiService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { map, toUpper } from 'lodash';
import { AppUtilsService } from '../../../../../shared/services/app-utils.service';

@Component({
  selector: 'aui-backup-node-detail-distributed',
  templateUrl: './backup-node-detail-distributed.component.html',
  styleUrls: ['./backup-node-detail-distributed.component.less'],
  providers: [CapacityCalculateLabel]
})
export class BackupNodeDetailDistributedComponent
  implements OnInit, AfterViewInit {
  activeIndex;
  drawData;
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  ipLabel = this.i18n.get('system_management_ip_label');
  roleLabel = this.i18n.get('common_role_label');
  tableConfig: TableConfig;
  tableDataBackup: TableData;
  tableDataArchived: TableData;
  tableDataReplication: TableData;
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('dataTableBackup', { static: false })
  dataTableBackup: ProTableComponent;
  @ViewChild('dataTableArchived', { static: false })
  dataTableArchived: ProTableComponent;
  @ViewChild('dataTableReplication', { static: false })
  dataTableReplication: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    public clusterApiService: ClustersApiService,
    public baseUtilService: BaseUtilService,
    public virtualScroll: VirtualScrollService,
    public appUtilsService: AppUtilsService,
    public cdr?: ChangeDetectorRef
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.virtualScroll.getScrollParam(200);
  }

  ngAfterViewInit() {
    this.dataTableBackup.fetchData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'ifaceName',
        name: this.isDecouple
          ? this.i18n.get('common_optional_port_label')
          : this.i18n.get('common_port_label')
      },
      {
        key: 'ipAddress',
        name: this.i18n.get('common_ip_address_mask_label')
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'ipAddress',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        fetchData: () => {
          this.getData();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData() {
    this.clusterApiService
      .getPacificNodeDetail({ name: this.drawData.name })
      .subscribe(res => {
        this.tableDataBackup = {
          data: res.usedNetworkInfo.backupIpInfoList,
          total: res.usedNetworkInfo.backupIpInfoList.length
        };
        this.tableDataArchived = {
          data: res.usedNetworkInfo.archiveIpInfoList,
          total: res.usedNetworkInfo.archiveIpInfoList.length
        };
        if (this.isDecouple) {
          this.tableDataReplication = {
            data: res.usedNetworkInfo.replicationIpInfoList,
            total: res.usedNetworkInfo.replicationIpInfoList.length
          };
        }
        this.cdr.detectChanges();
      });
  }

  clusterRole(item) {
    if (this.isDecouple) {
      return map(item.role, value =>
        this.dataMapService.getLabel('nodeRole', toUpper(value))
      );
    }
    return map(item.role, value =>
      this.dataMapService.getLabel('DistributedClusterRole', value)
    );
  }
}
