import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  TapeLibraryApiService,
  getAccessibleViewList
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
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { includes } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { StorageDeviceDetailComponent } from './storage-device-detail/storage-device-detail.component';

@Component({
  selector: 'aui-storage-device-list',
  templateUrl: './storage-device-list.component.html',
  styleUrls: ['./storage-device-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class StorageDeviceListComponent
  implements OnInit, AfterViewInit, OnDestroy {
  optsConfig;
  tableData: TableData;
  tableConfig: TableConfig;
  scanBtnDisable = false;
  timeSub$: Subscription;
  destroy$ = new Subject();
  selectionData = [];
  node;
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

  @ViewChild('deviceTable', { static: false }) deviceTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private tapeLibraryApiService: TapeLibraryApiService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    if (!this.isDataBackup) {
      this.deviceTable.fetchData();
    }
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'scan',
        type: 'primary',
        label: this.i18n.get('system_scan_tape_label'),
        onClick: () => {
          this.scanTape();
        },
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.ScanTapeLibrary
          ];
        }
      }
    ];
    this.optsConfig = opts;

    const cols: TableCols[] = [
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
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService.toArray('Media_Tape_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Tape_Status')
        }
      },
      {
        key: 'serialNo',
        name: this.i18n.get('common_serial_number_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'vendor',
        name: this.i18n.get('system_archive_vender_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'productId',
        name: this.i18n.get('system_archive_product_id_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'slotNumber',
        name: this.i18n.get('system_archive_slot_num_label')
      },
      {
        key: 'driveNumber',
        name: this.i18n.get('system_archive_drive_num_label')
      },
      {
        key: 'controllerName',
        name: this.i18n.get('common_home_node_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'serialNo',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  getData(filters?: Filters) {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.tapeLibraryApiService.getTapeLibrariesUsingGET({
            akLoading: !index,
            memberEsn: this.node?.remoteEsn
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  scanTape() {
    this.tapeLibraryApiService
      .scanTapeLibrariesUsingPUT({
        akOperationTips: false,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(
        res => {
          this.deviceTable.fetchData();
        },
        err => {
          this.deviceTable.fetchData();
        }
      );
  }

  getDetail(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'archive-device-detail',
      lvHeader: item.name,
      lvContent: StorageDeviceDetailComponent,
      lvWidth: MODAL_COMMON.xLargeWidth,
      lvComponentParams: {
        item,
        node: this.node
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }
}
