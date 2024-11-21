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
import {
  Component,
  OnInit,
  Output,
  EventEmitter,
  ViewChild,
  ChangeDetectorRef,
  AfterViewInit,
  TemplateRef,
  ViewChildren,
  ElementRef,
  QueryList
} from '@angular/core';
import {
  CommonConsts,
  DataMapService,
  I18NService,
  LANGUAGE,
  BaseUtilService,
  DataMap,
  ClustersApiService
} from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  every,
  filter,
  find,
  isArray,
  isEmpty,
  map,
  size,
  some
} from 'lodash';
import { forkJoin, Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-node-network-edit-distributed',
  templateUrl: './node-network-edit-distributed.component.html',
  styleUrls: ['./node-network-edit-distributed.component.less']
})
export class NodeNetworkEditDistributedComponent
  implements OnInit, AfterViewInit {
  activeIndex = 'backup';
  tableDataBackup;
  tableDataArchived;
  selectionData = {
    backupNetWorkInfoList: [],
    archiveNetWorkInfoList: []
  };
  selectionInvalid = new EventEmitter<any>();

  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public virtualScroll: VirtualScrollService,
    public clustersApiService: ClustersApiService,
    public cdr?: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.virtualScroll.getScrollParam(200);
  }

  ngAfterViewInit() {
    this.getData();
  }

  checkChange(item) {
    item.port = item.selection.length;
    this.validSelection();
  }

  validSelection() {
    const archiveZeroCount = filter(
      this.tableDataArchived,
      item => item.port === 0
    ).length;
    this.selectionInvalid.emit(
      some(this.tableDataBackup, item => item.port === 0) ||
        (archiveZeroCount > 0 &&
          archiveZeroCount < this.tableDataArchived.length)
    );
  }

  getData() {
    this.clustersApiService.getPacificNetworkInfo({}).subscribe(res => {
      const arrBackup = [];
      const arrArchive = [];
      each(res, item => {
        arrBackup.push({
          manageIp: item.pacificNodeInfoVo.manageIp,
          port: item.usedNetworkInfo.backupIpInfoList.length,
          expand: false,
          selection: item.usedNetworkInfo.backupIpInfoList,
          ipPoolDtoList: item.allNetworkInfo.backupIpInfoList
        });
        arrArchive.push({
          manageIp: item.pacificNodeInfoVo.manageIp,
          port: item.usedNetworkInfo.archiveIpInfoList.length,
          expand: false,
          selection: item.usedNetworkInfo.archiveIpInfoList,
          ipPoolDtoList: item.allNetworkInfo.archiveIpInfoList
        });
      });
      this.tableDataBackup = arrBackup;
      this.tableDataArchived = arrArchive;
      this.validSelection();
    });
  }

  getParams() {
    const request = {
      backupNetWorkInfoList: map(this.tableDataBackup, item => {
        return {
          manageIp: item.manageIp,
          ipInfoList: item.selection
        };
      })
    };
    if (!isEmpty(this.tableDataArchived[0].selection)) {
      assign(request, {
        archiveNetWorkInfoList: map(this.tableDataArchived, item => {
          return {
            manageIp: item.manageIp,
            ipInfoList: item.selection
          };
        })
      });
    }
    return { request: request };
  }

  onOK() {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      this.clustersApiService.updatePacificNetworkInfo(params).subscribe(
        res => {
          observer.next(res);
          observer.complete();
        },
        error => {
          observer.error(error);
          observer.complete();
        }
      );
    });
  }
}
