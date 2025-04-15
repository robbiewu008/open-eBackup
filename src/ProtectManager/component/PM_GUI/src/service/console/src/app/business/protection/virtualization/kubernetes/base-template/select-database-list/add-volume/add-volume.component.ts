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
import { ModalRef } from '@iux/live';
import { CommonConsts, DataMap } from 'app/shared';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  get,
  includes,
  isEmpty,
  map,
  reject,
  size,
  uniqueId
} from 'lodash';

@Component({
  selector: 'aui-add-volume',
  templateUrl: './add-volume.component.html',
  styleUrls: ['./add-volume.component.less']
})
export class AddVolumeComponent implements OnInit {
  data;
  selection;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  totalTableData = [];
  selectionData = [];

  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('pageS', { static: false }) pageS;
  dataMap = DataMap;

  constructor(private modal: ModalRef) {}

  ngOnInit() {
    this.initData();
  }

  stateChange(event) {}
  initData() {
    const sts = JSON.parse(get(this.data, ['extendInfo', 'sts']));
    setTimeout(() => {
      const volumes = map(sts.volumeNames, item => {
        return { name: item, id: uniqueId() };
      });
      this.totalTableData = cloneDeep(volumes);
      if (this.data.volumes) {
        each(this.totalTableData, v => {
          assign(v, {
            checked: includes(this.data.volumes, v.name)
          });
        });
      }
      this.selectionData = !isEmpty(this.data.volumes)
        ? filter(volumes, item => {
            return includes(this.data.volumes, item.name);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(volumes, item => {
            return includes(
              this.data.protectedObject?.extParameters?.volume_names,
              item.name
            );
          })
        : [];
      this.selection = this.selectionData;
    });
  }

  selectionChange(selection) {
    this.selectionData = [...selection];
    this.disableOkBtn();
  }

  clearSelected() {
    this.selectionData = [];
    each(this.totalTableData, item => {
      item.checked = false;
    });
    this.disableOkBtn();
  }

  removeSingle(item) {
    this.selectionData = reject(this.selectionData, value => {
      return item.name === value.name;
    });
    this.selection = this.selectionData;
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData);
  }

  onOK() {
    return cloneDeep(this.selectionData);
  }
}
