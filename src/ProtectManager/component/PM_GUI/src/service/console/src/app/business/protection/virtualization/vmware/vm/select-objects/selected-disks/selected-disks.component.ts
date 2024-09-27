import { Component, OnInit, ViewChild } from '@angular/core';
import { DatatableComponent, PaginatorComponent } from '@iux/live';
import {
  CAPACITY_UNIT,
  CommonConsts,
  I18NService,
  ProtectResourceCategory
} from 'app/shared';
import {
  assign,
  each,
  filter,
  find,
  includes,
  isEmpty,
  size,
  trim
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-selected-disks',
  templateUrl: './selected-disks.component.html',
  styleUrls: ['./selected-disks.component.less']
})
export class SelectedDisksComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  data;
  virtualType;
  columns;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  totalPageSize = CommonConsts.PAGE_SIZE;
  selectedPageSize = CommonConsts.PAGE_SIZE;
  totalPageIndex = CommonConsts.PAGE_START;
  selectedPageIndex = CommonConsts.PAGE_START;
  queryDeviceId;
  diskType = 'IDE';
  notAllDiskShow = false;
  valid$ = new Subject<boolean>();
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild(PaginatorComponent, { static: false }) lvPage: PaginatorComponent;
  @ViewChild('deviceIdPopover', { static: false }) deviceIdPopover;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.initDisks();
    this.initColumns();
    this.checkNotAllDisk();
  }

  initColumns() {
    this.columns = [
      {
        key: 'name',
        hidden: ![
          ProtectResourceCategory.vmware,
          ProtectResourceCategory.vmwares
        ].includes(this.virtualType),
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'slot',
        label: this.i18n.get('common_slot_label')
      },
      {
        key: 'capacity',
        align: 'right',
        hidden: ![
          ProtectResourceCategory.vmware,
          ProtectResourceCategory.vmwares
        ].includes(this.virtualType),
        label: this.i18n.get('common_capacity_label')
      }
    ];
  }

  searchByDeviceId() {
    if (this.deviceIdPopover) {
      this.deviceIdPopover.hide();
    }
    this.lvTable.filter({
      key: 'slot',
      value: trim(this.queryDeviceId),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
    this.cacheCurrentParam();
  }

  totalPageChange(event) {
    this.totalPageIndex = event.pageIndex;
    this.totalPageSize = event.pageSize;
    this.cacheCurrentParam();
  }

  selectedPageChange(event) {
    this.selectedPageIndex = event.pageIndex;
    this.selectedPageSize = event.pageSize;
    this.cacheCurrentParam();
  }

  cacheCurrentParam() {
    this.data.disksInfo.forEach(disk => {
      if (disk.type === this.diskType) {
        if (disk.activeIndex === 'total') {
          disk.totalParam = {
            queryDeviceId: this.queryDeviceId,
            pageSize: this.totalPageSize,
            pageIndex: this.totalPageIndex
          };
        } else {
          disk.selectedParam = {
            queryDeviceId: this.queryDeviceId,
            pageSize: this.selectedPageSize,
            pageIndex: this.selectedPageIndex
          };
        }
      }
    });
  }

  diskTypeTabChange(event) {
    this.updateParam(this.data.disksInfo.find(disk => disk.type === event));
  }

  diskInfoTabChange(event, currentDisk) {
    this.updateParam(currentDisk);
  }

  updateParam(currentDisk) {
    if (this.deviceIdPopover) {
      this.deviceIdPopover.hide();
    }
    this.queryDeviceId = '';
    if (currentDisk.activeIndex === 'total') {
      this.queryDeviceId = currentDisk.totalParam.queryDeviceId;
      this.totalPageIndex = currentDisk.totalParam.pageIndex;
      this.totalPageSize = currentDisk.totalParam.pageSize;
    } else {
      this.queryDeviceId = currentDisk.selectedParam.queryDeviceId;
      this.selectedPageIndex = currentDisk.selectedParam.pageIndex;
      this.selectedPageSize = currentDisk.selectedParam.pageSize;
    }
    if (trim(this.queryDeviceId)) {
      this.lvTable.filter({
        key: 'slot',
        value: trim(this.queryDeviceId),
        filterMode: 'contains'
      });
    }
  }

  initDisks() {
    if (this.data.allDisks && !this.data.disksInfo) {
      assign(this.data, {
        disksInfo: []
      });
      each(['IDE', 'SATA', 'SCSI', 'NVME'], item => {
        const datas = filter(this.data.allDisks, disk => {
          return includes(disk.slot, item);
        });
        let selection = [];
        if (
          this.data.ext_parameters &&
          this.data.ext_parameters.disk_info &&
          !this.data.ext_parameters.all_disk
        ) {
          each(datas, d => {
            if (
              includes(this.data.ext_parameters.disk_info, d.uuid) ||
              this.data.ext_parameters.disk_info[0] === '*'
            ) {
              selection.push(d);
            }
          });
        } else {
          selection = [...datas];
        }
        this.data.disksInfo.push({
          activeIndex: 'total',
          type: item,
          totalParam: {},
          selectedParam: {},
          allDatas: datas,
          selectDatas: datas.filter(item => item.uuid),
          selection: selection.filter(item => item.uuid)
        });
      });
    } else {
      this.data.disksInfo.forEach(disk => {
        disk.totalParam = {};
        disk.activeIndex = 'total';
        disk.selectedParam = {};
      });
    }
  }

  getTableHeadDiabled(renderData) {
    return !(renderData || []).find(item => item.uuid);
  }

  selectionChange(e, diskType) {
    diskType.selectDatas = filter(diskType.selection, item => {
      return find(diskType.selectDatas, { uuid: item.uuid });
    });
    this.valid$.next(!!this.data.disksInfo.find(item => item.selection.length));
    this.checkNotAllDisk();
  }

  checkNotAllDisk() {
    this.notAllDiskShow = !isEmpty(
      find(this.data.disksInfo, item => {
        return (
          !!size(item.allDatas) &&
          !!size(item.selection) &&
          size(item.allDatas) !== size(item.selection)
        );
      })
    );
  }
}
