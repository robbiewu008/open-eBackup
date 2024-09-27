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
import { Component, OnInit } from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AppService,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  assign,
  each,
  every,
  filter,
  first,
  includes,
  isEmpty,
  isNumber,
  map,
  omit,
  pick,
  toLower
} from 'lodash';

@Component({
  selector: 'aui-select-disk',
  templateUrl: './select-disk.component.html',
  styleUrls: ['./select-disk.component.less']
})
export class SelectDiskComponent implements OnInit {
  data;
  diskType = 'VIRTIO';
  disksInfo = [];
  unitconst = CAPACITY_UNIT;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  columns = [
    {
      key: 'showName',
      hidden: false,
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'slot',
      hidden: true,
      label: this.i18n.get('common_slot_label')
    },
    {
      key: 'size',
      hidden: false,
      label: this.i18n.get('common_capacity_label')
    }
  ];

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private appService: AppService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initDiskInfo();
    this.getResourceDetail();
  }

  initDiskInfo() {
    each(['VIRTIO', 'IDE', 'SCSI', 'SATA', 'USB'], type => {
      this.disksInfo.push({
        type,
        activeIndex: 'total',
        allDatas: [],
        selection: []
      });
    });
  }

  getResourceDetail() {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: this.data.rootUuid || this.data.root_uuid
        })
      })
      .subscribe((res: any) => {
        if (first(res.records)) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            this.diskOkBtn();
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getDisk(agentsId);
        }
      });
  }

  getDisk(agentsId, recordsTemp?: any[], startPage?: number) {
    const params = {
      agentId: agentsId,
      envId: this.data.rootUuid || this.data.root_uuid,
      resourceIds: [this.data.uuid || this.data.root_uuid],
      pageNo: startPage || CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        resourceType: DataMap.Resource_Type.cNwareDisk.value,
        uuid: this.data.uuid
      })
    };

    this.appService.ListResourcesDetails(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START_EXTRA;
      }
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) ||
        res.totalCount === CommonConsts.PAGE_START
      ) {
        each(recordsTemp, item => {
          assign(item, omit(JSON.parse(item.extendInfo?.details), ['name']), {
            showName: JSON.parse(item.extendInfo?.details)?.name
          });
        });
        each(this.disksInfo, item => {
          let disks = filter(
            recordsTemp,
            v => toLower(v.bus) === toLower(item.type)
          );
          if (!isEmpty(disks)) {
            assign(item, {
              allDatas: [...disks],
              selection: !isEmpty(this.data?.diskInfo)
                ? filter(disks, item =>
                    includes(map(this.data?.diskInfo, 'uuid'), item.uuid)
                  )
                : [...disks]
            });
          }
        });
        return;
      }
      startPage++;
      this.getDisk(agentsId, recordsTemp, startPage);
    });
  }

  diskOkBtn() {
    this.modal.getInstance().lvOkDisabled = every(this.disksInfo, item =>
      isEmpty(item.selection)
    );
  }

  selectionChange() {
    this.diskOkBtn();
  }

  onOK() {
    const disks = [];
    each(this.disksInfo, item => {
      each(item?.selection, v => {
        disks.push(pick(v, ['uuid', 'name']));
      });
    });
    return disks;
  }
}
