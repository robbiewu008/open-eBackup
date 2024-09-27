/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import { Component, OnInit, ViewChild } from '@angular/core';
import { DatatableComponent } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectResourceCategory,
  ResourceType,
  VirtualResourceService,
  VmwareService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  includes,
  isArray,
  isEmpty,
  reject
} from 'lodash';
import { forkJoin, Observable, Observer, Subject } from 'rxjs';
import { SelectedDisksComponent } from './selected-disks/selected-disks.component';

@Component({
  selector: 'aui-select-objects',
  templateUrl: './select-objects.component.html',
  styleUrls: ['./select-objects.component.less']
})
export class SelectObjectsComponent implements OnInit {
  selectDiskNum = 0;
  resourceData = [];
  isHyperv;
  isVirtualMachine;
  allDatas = [];
  selectedData = [];
  cachePage = {};
  hostClusterDiskData = [];
  activeIndex = 'selected';
  virtualType;
  isBatch = false;
  totalPageIndex = CommonConsts.PAGE_START;
  totalPageSize = CommonConsts.PAGE_SIZE_SMALL;
  selectedPageSize = CommonConsts.PAGE_SIZE_SMALL;
  selectedPageIndex = CommonConsts.PAGE_START;
  selectedTitle = this.i18n.get('common_virtual_machine_label');
  diskName = this.i18n.get('protection_disks_label');
  diskSelectTipKey = 'protection_protect_disk_select_label';

  total = CommonConsts.PAGE_TOTAL;
  valid$ = new Subject<boolean>();
  protectResourceCategory = ProtectResourceCategory;

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  constructor(
    private i18n: I18NService,
    private vMwareService: VmwareService,
    private drawModalService: DrawModalService,
    private virtualResourceService: VirtualResourceService
  ) {}

  initData(datas, type) {
    this.virtualType = type;
    if (!isArray(datas) || (isArray(datas) && datas.length === 1)) {
      this.isBatch = false;
    } else {
      this.isBatch = true;
    }
    this.resourceData = isArray(datas) ? datas : [datas];
    this.selectedData = cloneDeep(this.resourceData);
    this.isHyperv = this.resourceData[0]['resType'] === ResourceType.HYPERV;
    this.isVirtualMachine = [
      ProtectResourceCategory.vmware,
      ProtectResourceCategory.vmwares
    ].includes(this.virtualType);
  }

  ngOnInit() {
    this.selectedTitle = this.i18n.get(
      this.isVirtualMachine
        ? 'common_virtual_machine_label'
        : this.virtualType === ProtectResourceCategory.cluster
        ? 'protection_cluster_label'
        : 'common_host_label'
    );

    //  非Hyper-V类型下才查询硬盘
    if (this.isVirtualMachine) {
      !this.isBatch && this.getVmSelectedData();
    } else {
      this.diskName = this.i18n.get('protection_vm_node_label');
      this.diskSelectTipKey = 'protection_protect_vm_node_select_label';
      this.hostClusterDiskData = this.getHostClusterDiskData();
      this.getHostClusterSelectedData();
    }
    this.getTotalData(false);
  }

  getVmSelectedData() {
    const observerList = [];
    each(this.selectedData, (item, i) => {
      const loading = i < 10;
      observerList.push(this.getVmDiskData(item.uuid, loading));
    });

    forkJoin(observerList).subscribe({
      next: res => {
        res.forEach((disk: any, i) => {
          assign(this.selectedData[i], {
            allDisks: disk['error'] ? null : disk,
            allUsableDisks: disk['error']
              ? null
              : disk.filter(item => item.uuid),
            diskError: disk['error'] ? true : isEmpty(disk)
          });
        });
        this.valid$.next(
          this.selectedData.find(
            disk => disk.allUsableDisks && disk.allUsableDisks.length
          )
        );
      }
    });
  }

  getTotalData(loading) {
    // 批量操作无总计
    if (!this.isBatch) {
      return;
    }
    const conditions = this.resourceData[0].conditions;
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageNo: this.totalPageIndex,
        pageSize: this.totalPageSize,
        conditions: JSON.stringify(conditions)
      })
      .subscribe(res => {
        this.allDatas = res.items;
        this.total = res.total;

        res.items.forEach(item => {
          if (this.isVirtualMachine) {
            if (this.isBatch) return;
            this.getVmDiskData(item.uuid, loading).subscribe({
              next: (diskRes: any) => {
                item['allDisks'] = diskRes['error'] ? null : diskRes;
                item['allUsableDisks'] = diskRes['error']
                  ? null
                  : diskRes.filter(item => item.uuid);
                item['diskError'] = diskRes['error'] ? true : isEmpty(diskRes);
              }
            });
          } else {
            item['allDisks'] = this.hostClusterDiskData;
            item['allUsableDisks'] = this.hostClusterDiskData;
          }
        });
      });
  }

  getHostClusterSelectedData() {
    each(this.selectedData, (render, i) => {
      render['allDisks'] = this.hostClusterDiskData;
      render['allUsableDisks'] = this.hostClusterDiskData;
      this.selectedData[i] = render;
    });
  }

  pageChange(source) {
    this.totalPageIndex = source.pageIndex;
    this.totalPageSize = source.pageSize;
    this.getTotalData(true);
  }

  selectionChange() {
    if (this.isVirtualMachine) {
      this.valid$.next(this.selectedData.length > 0);
      return;
    }
    let validValue = false;
    if (this.selectedData.length) {
      const invalidItem = this.selectedData.find(
        item => !this.countSelectedDisks(item) && !item.diskError
      );
      validValue = isEmpty(invalidItem);
      if (!invalidItem) {
        const validItem = this.selectedData.find(item =>
          this.countSelectedDisks(item)
        );
        validValue = !isEmpty(validItem);
      }
    }
    this.valid$.next(validValue);
  }

  delete(item) {
    this.selectedData = reject(this.selectedData, value => {
      return value.uuid === item.uuid;
    });
    this.selectionChange();
  }

  getRowDisabled(rowItem) {
    let disabled = false;
    const selected = find(
      this.selectedData,
      item => rowItem.uuid === item.uuid
    );
    if ((!selected && rowItem.diskError) || rowItem.sla_id) {
      disabled = true;
    }
    return disabled;
  }

  trackById(index: number, item: any) {
    return item.uuid;
  }

  getTableHeadDiabled(renderData) {
    return !(renderData || []).find(item =>
      item.sub_type === DataMap.Resource_Type.msVirtualMachine.value ||
      item.sub_type === DataMap.Resource_Type.msHostSystem.value
        ? !item.sla_id
        : !item.sla_id && item.allUsableDisks && item.allUsableDisks.length
    );
  }

  getVmDiskData(vmUuid: string, loading): Observable<void> {
    return new Observable((observer: Observer<any>) => {
      this.vMwareService
        .listVmDiskV1VirtualMachinesVmUuidDisksGet({
          vmUuid,
          akLoading: loading,
          akDoException: false
        })
        .subscribe({
          next: res => {
            observer.next(res);
            observer.complete();
          },
          error: ex => {
            observer.next(ex);
            observer.complete();
          }
        });
    });
  }

  countSelectedDisks(item) {
    if (item.disksInfo && !isEmpty(item.disksInfo)) {
      let count = 0;
      each(item.disksInfo, d => {
        count += d.selection.length;
      });
      this.selectDiskNum = count;
      return count;
    }
    const resourceData = this.resourceData[0];
    if (
      resourceData.ext_parameters &&
      !isEmpty(resourceData.ext_parameters.disk_info)
    ) {
      let count = 0;
      each(item.allUsableDisks, d => {
        if (
          includes(resourceData.ext_parameters.disk_info, d.uuid) ||
          resourceData.ext_parameters.disk_info[0] === '*'
        ) {
          count++;
        }
      });
      this.selectDiskNum = count;
      return count;
    }
    this.selectDiskNum = item.allUsableDisks ? item.allUsableDisks.length : 0;
    return item.allUsableDisks ? item.allUsableDisks.length : 0;
  }

  getHostClusterDiskData() {
    const allDisks = [];
    //  IDE 4个槽位号 0:0 0:1 1:0 1:1
    //  SCSI 64个槽位号 0:0-0:15 1:0-1:15 2:0-2:15 3:0-3:15
    //  SATA 120个槽位号 0:0-0:29 1:0-1:29 2:0-2:29 3:0-3:29
    //  NVME 60个槽位号 0:0-0:14 1:0-1:14 2:0-2:14 3:0-3:14
    const diskList = [
      { label: 'IDE', x: 2, y: 2 },
      { label: 'SATA', x: 4, y: 30 },
      { label: 'SCSI', x: 4, y: 16 },
      { label: 'NVME', x: 4, y: 15 }
    ];
    each(diskList, item => {
      for (let i = 0; i < item.x; i++) {
        for (let j = 0; j < item.y; j++) {
          allDisks.push({
            slot: `${item.label}(${i}:${j})`,
            uuid: `${item.label}(${i}:${j})`
          });
        }
      }
    });

    return allDisks;
  }

  selectDisks(item) {
    if (item.diskError) {
      return;
    }

    if (this.isVirtualMachine) {
      this.getVmDiskData(item.uuid, true).subscribe({
        next: (disk: any) => {
          assign(item, {
            allDisks: disk['error'] ? null : disk,
            allUsableDisks: disk['error'] ? null : disk.filter(d => d.uuid),
            diskError: disk['error'] ? true : isEmpty(disk)
          });

          this.selectDisk(item);
        }
      });
    } else {
      this.selectDisk(item);
    }
  }

  selectDisk(data) {
    const cacheData = cloneDeep(data);
    if (data.diskError) {
      return;
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.normalWidth,
      lvHeader: this.i18n.get(
        this.isVirtualMachine
          ? 'protection_select_disks_label'
          : 'protection_select_vm_node_label'
      ),
      lvContent: SelectedDisksComponent,
      lvComponentParams: { data: cacheData, virtualType: this.virtualType },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as SelectedDisksComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvOk: modal => {
        this.selectedData.forEach((item, i) => {
          if (item.uuid === cacheData.uuid) {
            this.selectedData[i] = cacheData;
          }
        });
        this.selectedData = [...this.selectedData];
        this.selectionChange();
      }
    });
  }

  onOK() {
    return { selectedList: this.selectedData };
  }
}
