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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { CommonConsts, ResourceType, VmRestoreOptionType } from 'app/shared';
import {
  EnvironmentsService,
  VirtualResourceService,
  VmwareService
} from 'app/shared/api/services';
import { DataMap, RestoreType } from 'app/shared/consts';
import { TextMapPipe } from 'app/shared/pipe';
import { I18NService } from 'app/shared/services';
import { assign, each, find } from 'lodash';

@Component({
  selector: 'aui-computer-location',
  templateUrl: './computer-location.component.html',
  styleUrls: ['./computer-location.component.less'],
  providers: [TextMapPipe]
})
export class ComputerLocationComponent implements OnInit {
  vCenterOptions = [];
  selectedVCenter;
  treeData = [];
  treeSelection;
  version;
  showErrorTip = false;
  errorTipLabel;
  searchValue: string;

  @Input() vmRestoreOptionType = VmRestoreOptionType.VM;
  @Input() copyData;
  @Input() type;

  @Output() changeLocation = new EventEmitter<any>();
  @Output() changeVcenter = new EventEmitter<any>();

  constructor(
    private environmentsService: EnvironmentsService,
    private virtualResourceService: VirtualResourceService,
    private i18n: I18NService,
    public textMapPipe: TextMapPipe,
    private vmwareService: VmwareService
  ) {}

  ngOnInit() {
    try {
      this.version =
        this.copyData &&
        JSON.parse(this.copyData.properties).vmware_metadata.runtime.host
          .version;
    } catch (ex) {
      this.version = '';
    }
    this.vCenterOptions = [];
    this.getVCenterOptions(CommonConsts.PAGE_START);
  }

  getVCenterOptions(startPage) {
    this.environmentsService
      .queryResourcesV1EnvironmentsGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          type: ResourceType.VSPHERE
        })
      })
      .subscribe(res => {
        res.items.forEach(item => {
          this.vCenterOptions.push({
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] > 0) {
          this.getVCenterOptions(startPage);
          return;
        }
        this.vCenterOptions = [...this.vCenterOptions];
      });
  }

  vCenterChange(event) {
    const vcenter = this.vCenterOptions.find(
      item => item.key === this.selectedVCenter
    );
    this.treeData = [];
    this.showErrorTip = false;
    this.changeVcenter.emit(vcenter ? vcenter : '');
    this.getVcenterChangeData(CommonConsts.PAGE_START, event);
  }

  getVcenterChangeData(startPage, event) {
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          parent_uuid: event
        })
      })
      .subscribe(res => {
        res.items.forEach(item => {
          const node = {
            label: `${item.name}(${this.textMapPipe.transform(
              item.sub_type,
              'Resource_Type'
            )})`,
            name: item.name,
            vmIp: item.vm_ip,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            uuid: item.uuid,
            path: item.path,
            subType: item.sub_type,
            unselected: true,
            children: [],
            isLeaf: false
          };
          this.treeData.push(node);
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] > 0) {
          this.getVcenterChangeData(startPage, event);
          return;
        }
        this.treeData = [...this.treeData];
      });
  }

  expandedChange(event) {
    if (!event.expanded || event.children.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(CommonConsts.PAGE_START, event);
  }

  getExpandedChangeData(startPage, event) {
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          parent_uuid: event.uuid
        })
      })
      .subscribe(res => {
        each(res.items, (item: any) => {
          if (
            this.vmRestoreOptionType === VmRestoreOptionType.VM &&
            item.type === ResourceType.VM
          ) {
            return;
          }

          if (
            this.vmRestoreOptionType === VmRestoreOptionType.VM &&
            event.subType === DataMap.Resource_Type.hostSystem.value &&
            event.version < this.version
          ) {
            return;
          }

          let nodeUnselected = true;

          if (
            (this.vmRestoreOptionType === VmRestoreOptionType.VM &&
              item.alias_type &&
              ((this.version <= item.version &&
                item.sub_type === DataMap.Resource_Type.hostSystem.value) ||
                item.sub_type !== DataMap.Resource_Type.hostSystem.value)) ||
            (this.vmRestoreOptionType === VmRestoreOptionType.DISK &&
              item.sub_type === DataMap.Resource_Type.virtualMachine.value)
          ) {
            nodeUnselected = false;
          }

          // 即时恢复目标不能是esxi
          if (
            this.vmRestoreOptionType === VmRestoreOptionType.VM &&
            this.type === RestoreType.InstanceRestore &&
            item.environment_sub_type !==
              DataMap.Resource_Type.vmwareVcenterServer.value
          ) {
            nodeUnselected = true;
          }
          const node = {
            label: `${item.name}(${this.textMapPipe.transform(
              item.sub_type,
              'Resource_Type'
            )})`,
            name: item.name,
            vmIp: item.vm_ip,
            contentToggleIcon: this.getResourceIcon(item),
            uuid: item.uuid,
            path: item.path,
            subType: item.sub_type,
            version: item.version,
            firmware: item.firmware,
            environmentSubType: item.environment_sub_type,
            children: [],
            unselected: nodeUnselected,
            isLeaf: item.sub_type === DataMap.Resource_Type.virtualMachine.value
          };
          if (
            this.type === RestoreType.FileRestore &&
            item.sub_type === DataMap.Resource_Type.virtualMachine.value
          ) {
            assign(node, { os_type: item.os_type });
          }
          event.children.push(node);
          if (
            this.vmRestoreOptionType === VmRestoreOptionType.VM &&
            node.subType === DataMap.Resource_Type.clusterComputeResource.value
          ) {
            this.getExpandedChangeData(CommonConsts.PAGE_START, node);
          }
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] > 0) {
          this.getExpandedChangeData(startPage, event);
          return;
        }
        this.treeData = [...this.treeData];
      });
  }

  getResourceIcon(node) {
    const nodeResource = find(
      DataMap.Resource_Type,
      item => item.value === node.sub_type
    );
    return nodeResource['icon'] + '';
  }

  nodeCheck(e) {
    if (
      this.vmRestoreOptionType === VmRestoreOptionType.VM &&
      e.node &&
      e.node.subType === DataMap.Resource_Type.clusterComputeResource.value
    ) {
      this.vmwareService
        .getClusterConfigV1ComputeResourcesClustersClusterUuidConfigGet({
          clusterUuid: e.node.uuid
        })
        .subscribe(res => {
          if (res && !res.drs_enabled) {
            this.showErrorTip = !res.drs_enabled;
            this.errorTipLabel = this.i18n.get(
              'protection_vm_restore_vsphere_drs_desc_label'
            );
            e.node.unselected = true;
            this.nodeClick(e, true);
            return;
          }
          this.nodeClick(e);
        });
    } else {
      this.nodeClick(e);
    }
  }

  nodeClick(e, drsDisabled?) {
    const verisonErrorChildren = [];
    if (
      this.vmRestoreOptionType === VmRestoreOptionType.VM &&
      e.node &&
      e.node.subType === DataMap.Resource_Type.clusterComputeResource.value
    ) {
      e.node.children.forEach(child => {
        if (
          this.version > child.version &&
          child.subType === DataMap.Resource_Type.hostSystem.value
        ) {
          verisonErrorChildren.push(child.name);
        }
      });
      verisonErrorChildren.length && (e.node.unselected = true);
    }

    this.showErrorTip = e.node.unselected;
    if (!e.node.unselected) {
      this.changeLocation.emit(e.node);
    } else {
      this.changeLocation.emit(e.node);
      this.errorTipLabel =
        this.vmRestoreOptionType === VmRestoreOptionType.DISK
          ? this.i18n.get('protection_computer_location_disk_tip_label')
          : e.node.subType ===
            DataMap.Resource_Type.clusterComputeResource.value
          ? this.i18n.get(
              this.type === 'mount'
                ? 'protection_computer_location_vm_cluster_version_mount_tip_label'
                : 'protection_computer_location_vm_cluster_version_tip_label',
              [
                verisonErrorChildren.join(
                  this.i18n.language === this.i18n.defaultLanguage ? '，' : ', '
                )
              ]
            )
          : e.node.subType === DataMap.Resource_Type.hostSystem.value
          ? this.i18n.get(
              this.type === 'mount'
                ? 'protection_computer_location_vm_version_mount_tip_label'
                : 'protection_computer_location_vm_version_tip_label'
            )
          : this.i18n.get('protection_computer_location_vm_tip_label');
      if (
        this.vmRestoreOptionType === VmRestoreOptionType.VM &&
        this.type === RestoreType.InstanceRestore &&
        e.node.environmentSubType !==
          DataMap.Resource_Type.vmwareVcenterServer.value &&
        e.node.subType !== DataMap.Resource_Type.dataCenter.value
      ) {
        this.errorTipLabel = this.i18n.get(
          'protection_computer_location_vm_esx_tip_label'
        );
      }
      if (drsDisabled) {
        this.errorTipLabel = this.i18n.get(
          'protection_vm_restore_vsphere_drs_desc_label'
        );
      }
    }
  }
}
