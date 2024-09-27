import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProductStoragesApiService,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, isEmpty, trim, isFunction } from 'lodash';
import { AddStorageComponent } from './add-storage/add-storage.component';

@Component({
  selector: 'aui-external-storage',
  templateUrl: './external-storage.component.html',
  styleUrls: ['./external-storage.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ExternalStorageComponent implements OnInit {
  ip;
  type;
  wwn;
  esn;
  deviceName;
  tableData = [];
  filterParams = {};
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns = [
    {
      key: 'deviceName',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'status',
      label: this.i18n.get('common_status_label'),
      filter: true,
      filterMap: this.dataMapService.toArray('External_Storage_Status')
    },
    {
      key: 'ip',
      label: this.i18n.get('common_ip_address_label')
    },
    {
      key: 'type',
      label: this.i18n.get('common_type_label')
    },
    {
      key: 'wwn',
      label: 'WWN'
    },
    {
      key: 'esn',
      label: this.i18n.get('common_serial_number_label')
    }
  ];

  @ViewChild('deviceNamePopover', { static: false }) deviceNamePopover;
  @ViewChild('ipPopover', { static: false }) ipPopover;
  @ViewChild('typePopover', { static: false }) typePopover;
  @ViewChild('wwnPopover', { static: false }) wwnPopover;
  @ViewChild('esnPopover', { static: false }) esnPopover;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private productStoragesApiService: ProductStoragesApiService,
    private cookieService: CookieService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.getStorages();
    this.virtualScroll.getScrollParam(220);
  }

  onChange() {
    this.ngOnInit();
  }

  getStorages() {
    const params = {
      page: this.startPage,
      size: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }

    this.productStoragesApiService
      .queryProductStoragesUsingGET(params)
      .subscribe(res => {
        this.total = res.total;
        this.tableData = res.items;
        this.cdr.detectChanges();
      });
  }

  optsCallback = item => {
    const menus = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyingExternalStorage,
        onClick: (d: any) => {
          this.productStoragesApiService
            .getProductStorageUsingGET({ storageId: item.id })
            .subscribe(res => this.addStorage(res));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingExternalStorage,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('common_delete_device_label', [
              item.deviceName
            ]),
            onOK: () => {
              this.productStoragesApiService
                .deleteStorageUsingDELETE({ storageId: item.id })
                .subscribe(res => {
                  this.getStorages();
                });
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  addStorage(item = {} as any, callback?: () => void) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-storage-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_add_external_storage_label')
          : this.i18n.get('common_modify_colon_label', [item.deviceName]),
        lvContent: AddStorageComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddStorageComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddStorageComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                if (isFunction(callback)) {
                  callback();
                } else {
                  this.getStorages();
                }
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  searchByDeviceName() {
    if (this.deviceNamePopover) {
      this.deviceNamePopover.hide();
    }
    assign(this.filterParams, {
      device_name: trim(this.deviceName)
    });
    this.getStorages();
  }

  searchByType() {
    if (this.typePopover) {
      this.typePopover.hide();
    }
    assign(this.filterParams, {
      type: trim(this.type)
    });
    this.getStorages();
  }

  searchByIp() {
    if (this.ipPopover) {
      this.ipPopover.hide();
    }
    assign(this.filterParams, {
      ip: trim(this.ip)
    });
    this.getStorages();
  }

  searchByWwn() {
    if (this.wwnPopover) {
      this.wwnPopover.hide();
    }
    assign(this.filterParams, {
      wwn: trim(this.wwn)
    });
    this.getStorages();
  }

  searchByEsn() {
    if (this.esnPopover) {
      this.esnPopover.hide();
    }
    assign(this.filterParams, {
      esn: trim(this.esn)
    });
    this.getStorages();
  }

  storagePageChange(page) {
    this.pageSize = page.pageSize;
    this.startPage = page.pageIndex;
    this.getStorages();
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getStorages();
  }
}
