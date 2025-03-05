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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import { FileRestoreComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/file-restore/file-restore.component';
import { FileSystemResponseList } from 'app/shared/api/models';
import {
  AppService,
  CopyControllerService,
  ProtectedResourceApiService,
  RestoreFilesControllerService,
  RestoreManagerService
} from 'app/shared/api/services';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  NodeType,
  ResourceType,
  RestoreLocationType,
  RestoreType,
  VmFileReplaceStrategy
} from 'app/shared/consts';
import {
  BaseUtilService,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { extendNodeParams } from 'app/shared/utils';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  includes,
  isArray,
  isEmpty,
  isString,
  isUndefined,
  last,
  map,
  reduce,
  reject,
  size,
  trim,
  unionBy
} from 'lodash';
import { Subject } from 'rxjs';
import { finalize, switchMap, takeUntil } from 'rxjs/operators';
import { FileTreeComponent } from '../file-tree/file-tree.component';

@Component({
  selector: 'aui-vm-file-level-restore',
  templateUrl: './vm-file-level-restore.component.html',
  styleUrls: ['./vm-file-level-restore.component.less']
})
export class VmFileLevelRestoreComponent implements OnInit, OnDestroy {
  rowCopy;
  childResType;
  restoreType;
  restoreLevel;

  formGroup: FormGroup;
  resourceProperties;
  disableOriginLocation = false;
  originalFileData = [];
  originalSelection;
  selectionAssociate = true;
  restoreLocationType = RestoreLocationType;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  vmFileReplaceStrategy = VmFileReplaceStrategy;
  nodeType = NodeType;
  isEmpty = isEmpty;

  osTypeOptions = this.dataMapService
    .toArray('OS_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    })
    .filter(item =>
      includes(
        [DataMap.OS_Type.Windows.value, DataMap.OS_Type.Linux.value],
        item.value
      )
    );

  environmentOptions = [];
  vmTreeData = [];
  vmIpOptions = [];
  cacheOriginVmIpOptions = [];
  cacheNewVmIpOptions = [];
  vmIpNoData = false;

  targetCloudPlatformOptions = [];
  cloudPlatformTenantOptions = [];
  regionsOptions = [];
  projectsOptions = [];
  cloudHostOptions = [];
  serverTreeData = [];

  restorePath;
  restoreParamsVaild;
  testLoading = false;
  okLoading = false;
  isTest = false;
  disabled = true;

  // 文件检索关键字
  queryKey;
  cacheTreeData;
  treeTotal = 0;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = 50;
  searchByKeyFlag = false;

  // 虚拟机IP类型
  vmIpTypeOptions = {
    exist: '1',
    custom: '2'
  };

  // 搜索之前保持原来的数据
  needSearchResource = false;
  originalTreeData = [];
  // 搜索关键字
  searchVmKey: string;
  currentSearchKey: string;
  searchTrigger$ = new Subject<string>();
  destroy$ = new Subject();
  // 应用对应需要搜索的子类型
  searchSubTypeMap = {
    [DataMap.Resource_Type.cNwareVm.value]: [
      DataMap.Resource_Type.cNwareHostPool.value,
      DataMap.Resource_Type.cNwareCluster.value,
      DataMap.Resource_Type.cNwareHost.value,
      DataMap.Resource_Type.cNwareVm.value
    ]
  };

  requiredErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  pathErrorTip = {
    invalidPath: this.i18n.get('common_path_error_label')
  };

  @ViewChild(FileRestoreComponent, { static: false })
  FileRestoreComponent: FileRestoreComponent;
  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;
  @ViewChild('keyPopover', { static: false }) keyPopover;
  @ViewChild('fileTree', { static: false })
  fileTreeComponent: FileTreeComponent;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private appService: AppService,
    private messageService: MessageService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreService: RestoreManagerService,
    private restoreFilesControllerService: RestoreFilesControllerService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private warningMessageService: WarningMessageService,
    private dataMapService: DataMapService,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.initFooter();
    this.initFormGroup();
    this.getOriginalFileData();
    this.getResource();
    this.getEnvironment();
    this.setSearchState();
  }

  setSearchState() {
    this.needSearchResource = includes(
      [DataMap.Resource_Type.cNwareVm.value],
      this.childResType
    );
    if (!this.needSearchResource) {
      return;
    }
    this.searchTrigger$
      .pipe(
        switchMap(searchKey => {
          this.searchVmKey = searchKey;
          return this.protectedResourceApiService.ListResources({
            akLoading: false,
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE_MAX,
            conditions: JSON.stringify({
              rootUuid: this.formGroup.value.environment,
              subType: this.searchSubTypeMap[this.childResType] || [
                this.childResType
              ],
              name: [['~~'], searchKey]
            })
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        // 响应结果回来时，搜索值已清空，则不更新树
        if (!this.currentSearchKey) {
          return;
        }
        each(res.records, item => {
          assign(item, {
            label: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            children: item.subType === this.childResType ? null : [],
            disabled: item.subType !== this.childResType,
            isLeaf: item.subType === this.childResType,
            expanded: false
          });
        });
        this.vmTreeData = res.records;
      });
  }

  vmFilterChange(value) {
    this.currentSearchKey = value;
    if (
      value === this.searchVmKey ||
      !this.needSearchResource ||
      isEmpty(this.formGroup?.value.environment)
    ) {
      return;
    }
    if (!trim(value)) {
      this.vmTreeData = cloneDeep(this.originalTreeData);
      return;
    }
    this.searchTrigger$.next(trim(value));
  }

  initFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  getResourceIcon(node) {
    if (
      includes(
        [
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.childResType
      )
    ) {
      switch (node.subType) {
        case ResourceType.CNWARE:
        case ResourceType.NUTANIX:
          return node.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
            ? 'aui-icon-vCenter'
            : 'aui-icon-vCenter-offine';
        case DataMap.Resource_Type.cNwareHostPool.value:
          return 'aui-icon-host-pool';
        case DataMap.Resource_Type.cNwareCluster.value:
        case DataMap.Resource_Type.nutanixCluster.value:
          return 'aui-icon-cluster';
        case DataMap.Resource_Type.cNwareHost.value:
        case DataMap.Resource_Type.nutanixHost.value:
          return 'aui-icon-host';
        default:
          return 'aui-sla-vm';
      }
    } else if (this.childResType === DataMap.Resource_Type.hyperVVm.value) {
      switch (node.subType) {
        // 目前hyperv只有主机和虚拟机，后续接入集群，这里需要修改
        default:
          return 'aui-sla-vm';
      }
    }
    const nodeResource = find(
      DataMap.Resource_Type,
      item => item.value === node.type
    );
    return nodeResource['icon'] + '';
  }

  isCloudResource(resourceType) {
    return includes(
      [
        DataMap.Resource_Type.HCSCloudHost.value,
        DataMap.Resource_Type.openStackCloudServer.value,
        DataMap.Resource_Type.APSCloudServer.value
      ],
      resourceType
    );
  }

  getIpHelp(): string {
    if (
      includes(
        [
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value
        ],
        this.childResType
      )
    ) {
      return this.i18n.get('common_vmtools_help_label');
    }
    // 云场景
    if (this.isCloudResource(this.childResType)) {
      return this.i18n.get('common_cloud_ip_help_label');
    }
    // 其它虚拟机场景
    return this.i18n.get('common_vm_ip_help_label');
  }

  getAgentId(vm, isNew = true) {
    if (isEmpty(vm)) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        akDoException: false,
        conditions: JSON.stringify({
          uuid: vm.rootUuid || vm.root_uuid
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
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getResourcesDetails(agentsId, vm, isNew);
        }
      });
  }

  getResourcesDetails(agentId, vm, isNew) {
    this.appService
      .ListResourcesDetails({
        agentId,
        envId: vm.rootUuid || vm.root_uuid,
        resourceIds: [vm.uuid],
        pageNo: CommonConsts.PAGE_START_EXTRA,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        akDoException: false,
        conditions: JSON.stringify({
          targetType: 'Adapter',
          uuid: vm.uuid,
          FQDN: vm.extendInfo?.FQDN,
          ScanRequestId: vm.uuid
        })
      })
      .subscribe(res => {
        const tempData: any = first(res.records);
        const ips = tempData?.extendInfo?.ipAddress;
        this.vmIpNoData = isEmpty(ips);
        this.vmIpOptions = !this.vmIpNoData
          ? map(ips.split(','), item => {
              return {
                value: item,
                label: item,
                isLeaf: true
              };
            })
          : [];
        if (isNew) {
          this.cacheNewVmIpOptions = cloneDeep(this.vmIpOptions);
        } else {
          this.cacheOriginVmIpOptions = cloneDeep(this.vmIpOptions);
        }
      });
  }

  getResource() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.rowCopy?.resource_id,
        akDoException: false
      })
      .pipe(
        finalize(() => {
          if (this.disableOriginLocation) {
            this.vmIpNoData = isEmpty(this.vmIpOptions);
            return;
          }
          this.formGroup
            .get('location')
            .setValue(this.resourceProperties?.path);
          const fcIp = this.resourceProperties?.extendInfo?.vm_ip;
          if (!isEmpty(fcIp) && isString(fcIp)) {
            this.vmIpOptions = map(fcIp.split(','), item => {
              return {
                value: item,
                label: item,
                isLeaf: true
              };
            });
          }
          if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
            const hcsIp = JSON.parse(
              this.resourceProperties?.extendInfo?.host || '{}'
            )?.vm_ip;
            this.vmIpOptions = !isEmpty(hcsIp)
              ? map(hcsIp, item => {
                  return {
                    value: item,
                    label: item,
                    isLeaf: true
                  };
                })
              : [];
          }
          if (
            includes(
              [
                DataMap.Resource_Type.cNwareVm.value,
                DataMap.Resource_Type.nutanixVm.value
              ],
              this.childResType
            )
          ) {
            const cnwareIp =
              JSON.parse(this.resourceProperties?.extendInfo?.details || '{}')
                .ip || '';
            this.vmIpOptions = !isEmpty(cnwareIp)
              ? map(cnwareIp.split(','), item => {
                  return {
                    value: item,
                    label: item,
                    isLeaf: true
                  };
                })
              : [];
          }

          if (this.childResType === DataMap.Resource_Type.hyperVVm.value) {
            this.getAgentId(this.resourceProperties, false);
          }
          this.cacheOriginVmIpOptions = cloneDeep(this.vmIpOptions);
          this.vmIpNoData = isEmpty(this.vmIpOptions);
        })
      )
      .subscribe({
        next: res => {
          this.resourceProperties = res;
          // HCS云服务软删除不能作为恢复目标
          if (
            this.childResType === DataMap.Resource_Type.HCSCloudHost.value &&
            res.extendInfo?.status ===
              DataMap.HCS_Host_LinkStatus.softDelete.value
          ) {
            this.disableOriginLocation = true;
            this.formGroup
              .get('restoreLocation')
              .setValue(RestoreLocationType.NEW);
          }
        },
        error: () => {
          this.resourceProperties = JSON.parse(
            this.rowCopy.resource_properties
          );
        }
      });
  }

  getEnvironment() {
    if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
      const extParams = {
        conditions: JSON.stringify({
          subType: ResourceType.HCS_CONTAINER
        })
      };
      this.appUtilsService.getResourceByRecursion(
        extParams,
        params => this.protectedResourceApiService.ListResources(params),
        resource => {
          this.targetCloudPlatformOptions = map(resource, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            };
          });
        }
      );
    } else if (
      includes(
        [
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value
        ],
        this.childResType
      )
    ) {
      const extParams = {
        conditions: JSON.stringify({
          subType: [this.childResType],
          type: ResourceType.PLATFORM
        })
      };
      this.appUtilsService.getResourceByRecursion(
        extParams,
        params => this.protectedResourceApiService.ListResources(params),
        resource => {
          resource = resource.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          this.environmentOptions = map(resource, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            };
          });
        }
      );
    } else if (
      includes(
        [
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.childResType
      )
    ) {
      const extParams = {
        conditions: JSON.stringify({
          subType: this.rowCopy?.resource_type,
          type: this.rowCopy?.resource_type
        })
      };
      this.appUtilsService.getResourceByRecursion(
        extParams,
        params => this.protectedResourceApiService.ListResources(params),
        resource => {
          resource = resource.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          this.environmentOptions = map(resource, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            };
          });
        }
      );
    } else if (this.childResType === DataMap.Resource_Type.hyperVVm.value) {
      const extParams = {
        conditions: JSON.stringify({
          subType: [
            DataMap.Resource_Type.hyperVHost.value,
            DataMap.Resource_Type.hyperVScvmm.value,
            DataMap.Resource_Type.hyperVCluster.value
          ],
          type: ResourceType.Virtualization
        })
      };
      this.appUtilsService.getResourceByRecursion(
        extParams,
        params => this.protectedResourceApiService.ListResources(params),
        resource => {
          this.environmentOptions = map(resource, item => {
            return {
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            };
          });
        }
      );
    }
  }

  getTreeData(environmentId) {
    if (!environmentId) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        rootUuid: environmentId,
        type: [ResourceType.CLUSTER]
      })
    };
    if (
      includes(
        [
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.childResType
      )
    ) {
      assign(extParams, {
        conditions: JSON.stringify({
          parentUuid: environmentId
        })
      });
    }
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.vmTreeData = map(resource, item => {
          return assign(item, {
            label: item.name,
            disabled: this.disableTreeData(item),
            contentToggleIcon: this.getResourceIcon(item),
            children: [],
            isLeaf: false,
            expanded: false
          });
        });

        if (this.childResType === DataMap.Resource_Type.nutanixVm.value) {
          this.vmTreeData = this.vmTreeData.filter(
            item =>
              !includes([DataMap.Resource_Type.nutanixVm.value], item.subType)
          );
        }

        this.originalTreeData = cloneDeep(this.vmTreeData);
      }
    );
  }

  disableTreeData(item) {
    switch (item.subType) {
      case DataMap.Resource_Type.hyperVVm.value:
        return false;
      case DataMap.Resource_Type.nutanixVm.value:
        return false;
      default:
        return item.type !== ResourceType.VM;
    }
  }

  getCloudPlatform() {
    if (!this.formGroup.value.targetCloudPlatform) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        visible: '1',
        parentUuid: this.formGroup.value.targetCloudPlatform
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.cloudPlatformTenantOptions = map(resource, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
      }
    );
  }

  getRegion() {
    if (!this.formGroup.value.cloudPlatformTenant) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: this.formGroup.value.cloudPlatformTenant
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.regionsOptions = map(resource, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
      }
    );
  }

  getProject() {
    if (!this.formGroup.value.regions) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: this.formGroup.value.regions
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.projectsOptions = map(resource, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
      }
    );
  }

  getCloudHost() {
    if (!this.formGroup.value.projects) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: this.formGroup.value.projects
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        // 软删除不能恢复
        resource = reject(
          resource,
          item =>
            item.extendInfo?.status ===
            DataMap.HCS_Host_LinkStatus.softDelete.value
        );
        this.cloudHostOptions = map(resource, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
      }
    );
  }

  getOptions(subType, params, node?) {
    const extParams = {
      conditions: JSON.stringify(params)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === ResourceType.OPENSTACK_CONTAINER) {
          this.serverTreeData = map(resource, item => {
            return assign(item, {
              label: item.name,
              disabled: true,
              children: [],
              isLeaf: false,
              expanded: false
            });
          });
        } else {
          each(resource, item => {
            const isOpenStackCloudServer =
              item.subType === DataMap.Resource_Type.openStackCloudServer.value;
            node.children.push(
              assign(item, {
                label: item.name,
                disabled: !isOpenStackCloudServer,
                children: isOpenStackCloudServer ? null : [],
                isLeaf: isOpenStackCloudServer,
                expanded: false
              })
            );
          });
          this.serverTreeData = [...this.serverTreeData];
        }
      }
    );
  }

  openstackExpandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    if (node.subType === ResourceType.OPENSTACK_CONTAINER) {
      this.getOptions(
        ResourceType.OpenStackDomain,
        {
          parentUuid: node.uuid,
          visible: ['1']
        },
        node
      );
    } else if (node.subType === ResourceType.OpenStackDomain) {
      this.getOptions(
        ResourceType.OpenStackProject,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else {
      this.getOptions(
        ResourceType.OpenStackCloudServer,
        {
          path: [['=~'], `${node.path}/`],
          subType: [DataMap.Resource_Type.openStackCloudServer.value]
        },
        node
      );
    }
  }

  expandedChange(event) {
    if (!event.expanded || event.children?.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(event);
  }

  getExpandedChangeData(event) {
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: event.uuid
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (this.childResType === DataMap.Resource_Type.cNwareVm.value) {
          each(resource, item => {
            event.children.push(
              assign(item, {
                label: item.name,
                disabled: item.subType !== DataMap.Resource_Type.cNwareVm.value,
                contentToggleIcon: this.getResourceIcon(item),
                children:
                  item.subType === DataMap.Resource_Type.cNwareVm.value
                    ? null
                    : [],
                isLeaf: item.subType === DataMap.Resource_Type.cNwareVm.value,
                expanded: false
              })
            );
          });
        } else if (this.childResType === DataMap.Resource_Type.hyperVVm.value) {
          each(resource, item => {
            event.children.push(
              assign(item, {
                label: item.name,
                disabled: item.subType !== DataMap.Resource_Type.hyperVVm.value,
                contentToggleIcon: this.getResourceIcon(item),
                children:
                  item.subType === DataMap.Resource_Type.hyperVVm.value
                    ? null
                    : [],
                isLeaf: item.subType === DataMap.Resource_Type.hyperVVm.value,
                expanded: false
              })
            );
          });
        } else if (
          this.childResType === DataMap.Resource_Type.nutanixVm.value
        ) {
          each(resource, item => {
            event.children.push(
              assign(item, {
                label: item.name,
                disabled:
                  item.subType !== DataMap.Resource_Type.nutanixVm.value,
                contentToggleIcon: this.getResourceIcon(item),
                children:
                  item.subType === DataMap.Resource_Type.nutanixVm.value
                    ? null
                    : [],
                isLeaf: item.subType === DataMap.Resource_Type.nutanixVm.value,
                expanded: false
              })
            );
          });
        } else {
          resource.sort((a, b) => {
            if (a.type === ResourceType.HOST) {
              return -1;
            } else if (a.type === b.type) {
              return 0;
            } else {
              return 1;
            }
          });
          each(resource, item => {
            event.children.push(
              assign(item, {
                label:
                  item.type !== ResourceType.VM
                    ? item.name
                    : `${item.name}(${last(
                        item.extendInfo?.moReference?.split('/')
                      )})`,
                disabled: item.type !== ResourceType.VM,
                contentToggleIcon: this.getResourceIcon(item),
                children: item.type === ResourceType.VM ? null : [],
                isLeaf: item.type === ResourceType.VM,
                expanded: false
              })
            );
          });
        }
        this.vmTreeData = [...this.vmTreeData];
        if (!this.currentSearchKey) {
          this.originalTreeData = cloneDeep(this.vmTreeData);
        }
      }
    );
  }

  setPortValid() {
    if (this.showPort()) {
      this.formGroup
        .get('port')
        ?.addValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]);
    } else {
      this.formGroup.get('port')?.clearValidators();
    }
    this.formGroup.get('port')?.updateValueAndValidity();
  }

  validTargetPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (
        !CommonConsts.REGEX.windowsPath.test(control.value) &&
        !CommonConsts.REGEX.linuxPath.test(control.value)
      ) {
        return { invalidPath: { value: control.value } };
      }
      return null;
    };
  }

  addOsTypeControl() {
    if (this.manualOstype()) {
      if (isEmpty(this.formGroup.get('osType'))) {
        this.formGroup.addControl(
          'osType',
          new FormControl('', {
            validators: [this.baseUtilService.VALID.required()]
          })
        );
      }
    } else {
      this.formGroup.get('osType')?.clearValidators();
    }
    this.formGroup.get('osType')?.updateValueAndValidity();
  }

  initFormGroup() {
    if (this.rowCopy.isSearchRestore) {
      this.restorePath = [this.rowCopy.searchRestorePath];
    }

    this.disableOriginLocation =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy?.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    this.resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.formGroup = this.fb.group({
      restoreLocation: [RestoreLocationType.ORIGIN],
      location: new FormControl(''),
      environment: new FormControl(''),
      vm: new FormControl([]),
      targetPath: new FormControl('', {
        validators: [this.validTargetPath()]
      }),
      vmIpType: new FormControl(this.vmIpTypeOptions.exist),
      vmIp: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      customIp: new FormControl(''),
      port: new FormControl('22'),
      userName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      originalType: [VmFileReplaceStrategy.Overwriting]
    });

    if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
      this.formGroup.addControl('targetCloudPlatform', new FormControl(''));
      this.formGroup.addControl('cloudPlatformTenant', new FormControl(''));
      this.formGroup.addControl('regions', new FormControl(''));
      this.formGroup.addControl('projects', new FormControl(''));
      this.formGroup.addControl('cloudHost', new FormControl(''));
      this.formGroup.removeControl('environment');
      this.formGroup.removeControl('vm');
    } else if (
      this.childResType === DataMap.Resource_Type.openStackCloudServer.value
    ) {
      this.formGroup.addControl('targetServer', new FormControl([]));
      this.formGroup.removeControl('environment');
      this.formGroup.removeControl('vm');
    }

    // 操作系统选择
    this.addOsTypeControl();

    // 端口校验
    this.formGroup
      .get('osType')
      ?.valueChanges.subscribe(() => defer(() => this.setPortValid()));

    this.formGroup
      .get('vmIpType')
      .valueChanges.subscribe(res => this.listenVmIpType(res));

    this.listenForm();

    if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
      this.listenHcsForm();
    }

    if (this.childResType === DataMap.Resource_Type.APSCloudServer.value) {
      this.formGroup.addControl('targetServer', new FormControl(''));
      this.listenAPSForm();
    }
    defer(() => {
      if (this.disableOriginLocation) {
        this.formGroup.get('restoreLocation').setValue(RestoreLocationType.NEW);
      } else {
        this.setPortValid();
      }
    });
  }

  resetVmIp() {
    this.vmIpNoData = true;
    this.vmIpOptions = [];
    this.cacheNewVmIpOptions = [];
    this.formGroup.get('vmIp').setValue('', { emitEvent: false });
  }

  listenVmIpType(res) {
    if (res === this.vmIpTypeOptions.exist) {
      this.formGroup
        .get('vmIp')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup.get('customIp').clearValidators();
    } else {
      this.formGroup
        .get('customIp')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]);
      this.formGroup.get('vmIp').clearValidators();
    }
    this.formGroup.get('customIp').updateValueAndValidity();
    this.formGroup.get('vmIp').updateValueAndValidity();
  }

  listenForm() {
    this.formGroup.valueChanges.subscribe(res => {
      this.isTest = false;
    });
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreLocationType.ORIGIN) {
        this.vmIpOptions = this.cacheOriginVmIpOptions;
        if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
          this.formGroup.get('targetCloudPlatform').clearValidators();
          this.formGroup.get('cloudPlatformTenant').clearValidators();
          this.formGroup.get('regions').clearValidators();
          this.formGroup.get('projects').clearValidators();
          this.formGroup.get('cloudHost').clearValidators();
        } else if (
          this.childResType === DataMap.Resource_Type.openStackCloudServer.value
        ) {
          this.formGroup.get('targetServer').clearValidators();
        } else if (
          this.childResType === DataMap.Resource_Type.APSCloudServer.value
        ) {
          this.formGroup.get('targetServer').clearValidators();
        } else {
          this.formGroup.get('environment').clearValidators();
          this.formGroup.get('vm').clearValidators();
        }
      } else {
        this.vmIpOptions = this.cacheNewVmIpOptions;
        if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
          this.formGroup
            .get('targetCloudPlatform')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('cloudPlatformTenant')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('regions')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('projects')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('cloudHost')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else if (
          this.childResType === DataMap.Resource_Type.openStackCloudServer.value
        ) {
          if (isEmpty(this.serverTreeData)) {
            this.getOptions(ResourceType.OPENSTACK_CONTAINER, {
              type: ResourceType.OpenStack,
              subType: ResourceType.OPENSTACK_CONTAINER
            });
          }
          this.formGroup
            .get('targetServer')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else if (
          this.childResType === DataMap.Resource_Type.APSCloudServer.value
        ) {
          this.formGroup
            .get('targetServer')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup
            .get('environment')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('vm')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
      }
      this.vmIpNoData = isEmpty(this.vmIpOptions);
      this.formGroup.get('vmIp').setValue('');
      if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
        this.formGroup
          .get('targetCloudPlatform')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('cloudPlatformTenant')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('regions')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('projects')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('cloudHost')
          .updateValueAndValidity({ emitEvent: false });
      } else if (
        this.childResType === DataMap.Resource_Type.openStackCloudServer.value
      ) {
        this.formGroup
          .get('targetServer')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup
          .get('osType')
          .updateValueAndValidity({ emitEvent: false });
      } else if (
        this.childResType === DataMap.Resource_Type.APSCloudServer.value
      ) {
        this.formGroup
          .get('targetServer')
          .updateValueAndValidity({ emitEvent: false });
      } else {
        this.formGroup
          .get('environment')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup.get('vm').updateValueAndValidity({ emitEvent: false });
      }
      this.setPortValid();
    });

    this.vmValueChanged('targetServer');

    this.formGroup.get('environment')?.valueChanges.subscribe(res => {
      if (this.formGroup.value.restoreLocation === RestoreLocationType.NEW) {
        this.getTreeData(res);
        this.resetVmIp();
      }
    });
    this.vmValueChanged('vm');
    this.formGroup.statusChanges.subscribe(valid => {
      this.restoreParamsVaild = valid === 'VALID';
      this.getOkDisabled();
    });
  }

  vmValueChanged(formControlName) {
    this.formGroup.get(formControlName)?.valueChanges.subscribe(res => {
      let ips = '';
      if (
        includes(
          [
            DataMap.Resource_Type.cNwareVm.value,
            DataMap.Resource_Type.nutanixVm.value
          ],
          this.childResType
        )
      ) {
        ips = JSON.parse(res[0]?.extendInfo?.details || '{}')?.ip || '';
      } else if (this.childResType === DataMap.Resource_Type.hyperVVm.value) {
        this.getAgentId(res[0]);
      } else {
        ips = res[0]?.extendInfo?.vm_ip;
      }
      this.vmIpNoData = isEmpty(ips);
      this.vmIpOptions = !this.vmIpNoData
        ? map(ips.split(','), item => {
            return {
              value: item,
              label: item,
              isLeaf: true
            };
          })
        : [];
      this.cacheNewVmIpOptions = cloneDeep(this.vmIpOptions);
      this.formGroup.get('vmIp').setValue('', { emitEvent: false });
      // 操作系统选择
      this.addOsTypeControl();
      this.setPortValid();
    });
  }

  listenHcsForm() {
    this.formGroup.get('targetCloudPlatform').valueChanges.subscribe(res => {
      this.formGroup
        .get('cloudPlatformTenant')
        .setValue('', { emitEvent: false });
      this.formGroup.get('regions').setValue('', { emitEvent: false });
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.cloudPlatformTenantOptions = [];
      this.regionsOptions = [];
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getCloudPlatform());
      this.resetVmIp();
    });

    this.formGroup.get('cloudPlatformTenant').valueChanges.subscribe(res => {
      this.formGroup.get('regions').setValue('', { emitEvent: false });
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.regionsOptions = [];
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getRegion());
      this.resetVmIp();
    });

    this.formGroup.get('regions').valueChanges.subscribe(res => {
      this.formGroup.get('projects').setValue('', { emitEvent: false });
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.projectsOptions = [];
      this.cloudHostOptions = [];
      defer(() => this.getProject());
      this.resetVmIp();
    });

    this.formGroup.get('projects').valueChanges.subscribe(res => {
      this.formGroup.get('cloudHost').setValue('', { emitEvent: false });
      this.cloudHostOptions = [];
      defer(() => this.getCloudHost());
      this.resetVmIp();
    });

    this.formGroup.get('cloudHost').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const cloudHost = find(this.cloudHostOptions, { value: res });
      const ips = JSON.parse(cloudHost?.extendInfo?.host || '{}').vm_ip;
      this.vmIpNoData = isEmpty(ips);
      this.vmIpOptions = !this.vmIpNoData
        ? map(ips, item => {
            return {
              value: item,
              label: item,
              isLeaf: true
            };
          })
        : [];
      this.cacheNewVmIpOptions = cloneDeep(this.vmIpOptions);
      this.formGroup.get('vmIp').setValue('', { emitEvent: false });
      this.setPortValid();
    });
  }

  listenAPSForm() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreLocationType.NEW) {
        this.getApsTreeData();
        this.formGroup
          .get('targetServer')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('targetServer').updateValueAndValidity();
    });

    this.formGroup.get('targetServer').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }

      const ips = res[0]?.extendInfo.vm_ip;
      this.vmIpNoData = isEmpty(ips);
      this.vmIpOptions = !this.vmIpNoData
        ? map(ips.split(','), item => {
            return {
              value: item,
              label: item,
              isLeaf: true
            };
          })
        : [];
      this.cacheNewVmIpOptions = cloneDeep(this.vmIpOptions);
      this.formGroup.get('vmIp').setValue('', { emitEvent: false });
    });
  }

  getApsOptions(subType, params, node?) {
    const extParams = {
      conditions: JSON.stringify(params)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === DataMap.Resource_Type.ApsaraStack.value) {
          this.serverTreeData = map(resource, item => {
            return assign(item, {
              label: item.name,
              disabled: true,
              children: [],
              isLeaf: false,
              expanded: false
            });
          });
        } else {
          if (subType === DataMap.Resource_Type.APSZone.value) {
            resource = filter(resource, val => {
              return val.subType !== DataMap.Resource_Type.APSResourceSet.value;
            });
          }
          each(resource, item => {
            const isCloudServer =
              item.subType === DataMap.Resource_Type.APSCloudServer.value;
            node.children.push(
              assign(item, {
                label: item.name,
                disabled: !isCloudServer,
                children: isCloudServer ? null : [],
                isLeaf: isCloudServer,
                expanded: false
              })
            );
          });
        }
        this.serverTreeData = [...this.serverTreeData];
      }
    );
  }

  getApsTreeData() {
    if (!isEmpty(this.serverTreeData)) {
      return;
    }
    this.getApsOptions(DataMap.Resource_Type.ApsaraStack.value, {
      type: DataMap.Resource_Type.ApsaraStack.value,
      subType: DataMap.Resource_Type.ApsaraStack.value
    });
  }

  apsExpandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    if (node.subType === DataMap.Resource_Type.ApsaraStack.value) {
      this.getApsOptions(
        DataMap.Resource_Type.APSRegion.value,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else if (node.subType === DataMap.Resource_Type.APSRegion.value) {
      this.getApsOptions(
        DataMap.Resource_Type.APSZone.value,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else {
      this.getApsOptions(
        DataMap.Resource_Type.APSCloudServer.value,
        {
          path: [['=~'], `${node.path}/`],
          subType: [DataMap.Resource_Type.APSCloudServer.value]
        },
        node
      );
    }
  }

  getFilePath(paths) {
    if (isEmpty(paths)) {
      return [];
    }
    if (!paths[0].nodeName) {
      return ['/'];
    }
    let filterPaths = [];
    let childPaths = [];
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'path');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(find(childPaths, { path: item.path })) || item.isMoreBtn;
    });

    return reduce(
      filterPaths,
      (arr, item) => {
        arr.push(
          item.path === '/' ? `/${item.name}` : `${item.path}/${item.name}`
        );
        return arr;
      },
      []
    );
  }

  tableSelectionChange(selection) {
    if (
      this.fileTreeComponent?.pathMode ===
      this.fileTreeComponent?.modeMap?.fromTag
    ) {
      this.restorePath = [...selection];
    } else {
      this.restorePath = this.getFilePath(selection);
    }

    this.getOkDisabled();
  }

  getOriginalFileData() {
    this.originalFileData = [
      {
        name: '',
        nodeName: '',
        children: [],
        path: '/',
        label: this.rowCopy?.resource_name,
        absolutePath: '/',
        contentToggleIcon: 'aui-icon-directory'
      }
    ];
  }

  getCopySourceTree(node) {
    const moreBtn = find(node.children, { isMoreBtn: true });
    if (!!size(node.children) && !moreBtn) {
      return;
    }
    this.getCopySourceNode(node, moreBtn?.startPage);
  }

  getCopySourceNode(node, startPage?) {
    const params = {
      copyId: this.rowCopy.uuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      parentPath:
        node.path === '/'
          ? `${node.path}${node.nodeName}`
          : `${node.path}/${node.nodeName}`,
      akOperationTips: false,
      akLoading: false
    };
    if (!isEmpty(this.rowCopy.device_esn)) {
      assign(params, { memberEsn: this.rowCopy.device_esn });
    }
    // 已索引副本请求下一页需要上一页最后一条数据的sort值
    const sortValue = node.children[node.children.length - 2]?.sort;
    if (params.pageNo > 0 && !isEmpty(sortValue)) {
      assign(params, {
        searchAfter: sortValue
      });
    }
    node.isLoading = true;
    // 解决点击更多的时候节点没有加载状态
    if (params.pageNo > 0) {
      this.originalFileData = [...this.originalFileData];
    }
    this.copyControllerService
      .ListCopyCatalogs(params)
      .pipe(
        finalize(() => {
          node.isLoading = false;
        })
      )
      .subscribe(res => {
        this.updataChildren(res, node, true);
        this.originalFileData = [...this.originalFileData];
      });
  }

  updataChildren(res: FileSystemResponseList, node, isCopySource?) {
    each(res.records, (item: any) => {
      item = extendNodeParams(node, item);
    });
    if (isArray(node.children) && !isEmpty(node.children)) {
      node.children = [
        ...reject(node.children, n => {
          return n.isMoreBtn;
        }),
        ...res.records
      ];
    } else {
      node.children.push(...res.records);
    }
    if (res.totalCount > size(node.children)) {
      const moreClickNode = {
        label: `${this.i18n.get('common_more_label')}...`,
        contentToggleIcon: '',
        isMoreBtn: true,
        hasChildren: false,
        isLeaf: true,
        children: null,
        parent: node,
        startPage: Math.floor(size(node.children) / CommonConsts.MAX_PAGE_SIZE)
      };
      node.children = [...node.children, moreClickNode];
    }
    if (find(this.originalSelection, node) && isCopySource) {
      this.originalSelection = [...this.originalSelection, ...res.records];
    }
  }

  restoreParamsChange(valid) {
    this.restoreParamsVaild = valid === 'VALID';
    this.getOkDisabled();
  }

  getOkDisabled() {
    this.disabled = isEmpty(this.restorePath) || !this.restoreParamsVaild;
  }

  getPath() {
    if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
      return find(this.cloudHostOptions, {
        value: this.formGroup.value.cloudHost
      })?.path;
    } else if (
      this.childResType === DataMap.Resource_Type.openStackCloudServer.value
    ) {
      return this.formGroup.value.targetServer[0]?.path;
    } else if (
      this.childResType === DataMap.Resource_Type.APSCloudServer.value
    ) {
      return this.formGroup.value.targetServer[0].path;
    } else {
      return this.formGroup.value.vm[0]?.path;
    }
  }

  getTargetPath() {
    return {
      tips: this.i18n.get('common_files_label'),
      targetPath:
        this.formGroup.value.restoreLocation === this.restoreLocationType.NEW
          ? this.getPath()
          : this.resourceProperties.path
    };
  }

  getVmIp(): string {
    return this.formGroup.get('vmIpType')?.value === this.vmIpTypeOptions.exist
      ? this.formGroup.get('vmIp')?.value
      : this.formGroup.get('customIp')?.value;
  }

  showPort(): boolean {
    if (this.manualOstype()) {
      return this.formGroup.value.osType === DataMap.OS_Type.Linux.value;
    }
    if (this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN) {
      return includes(
        [DataMap.Os_Type.linux.value, DataMap.vmwareOsType.linux.value],
        this.resourceProperties?.extendInfo?.os_type
      );
    }
    const vm: any = this.getVm();
    return includes(
      [DataMap.Os_Type.linux.value, DataMap.vmwareOsType.linux.value],
      vm?.extendInfo?.os_type
    );
  }

  // 是否需要手动选择操作系统，从1.3.0升级到1.6.0时操作系统可能获取不到，界面做容错处理
  manualOstype() {
    if (
      includes(
        [
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.childResType
      )
    ) {
      return true;
    }
    if (this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN) {
      return isUndefined(this.resourceProperties?.extendInfo?.os_type);
    }
    const vm = this.getVm();
    if (isEmpty(vm)) {
      return false;
    }
    return isUndefined(vm?.extendInfo?.os_type);
  }

  getParams() {
    let params: any = {};
    if (
      includes(
        [
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.APSCloudServer.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.nutanixVm.value
        ],
        this.childResType
      )
    ) {
      const vm: any = this.getVm() || {};
      params = {
        copy_id: this.rowCopy.uuid,
        object_type: this.rowCopy.resource_sub_type,
        restore_location: this.formGroup.value.restoreLocation,
        filters: [],
        restore_objects: this.restorePath,
        restore_type: RestoreType.FileRestore,
        target: {
          details: [],
          env_id:
            this.formGroup.value.restoreLocation ===
            this.restoreLocationType.NEW
              ? vm.uuid
              : this.rowCopy.resource_id,
          env_type: this.childResType,
          restore_target:
            this.formGroup.value.restoreLocation ===
            this.restoreLocationType.NEW
              ? vm?.path
              : ''
        },
        source: {
          source_location: this.rowCopy.resource_location,
          source_name: this.rowCopy.resource_name
        },
        ext_parameters: {
          vm_name:
            this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
              ? this.resourceProperties.name
              : vm.name,
          USER_NAME: this.formGroup.value.userName,
          PASSWORD: this.formGroup.value.password,
          VM_IP: this.getVmIp(),
          FILE_REPLACE_STRATEGY: this.formGroup.value.originalType,
          TARGET_PATH: this.formGroup.value.targetPath || ''
        }
      };
    } else {
      params = this.FileRestoreComponent.getParams();
    }
    if (this.showPort()) {
      assign(params.ext_parameters, {
        PORT: this.formGroup.get('port')?.value
      });
    }
    return params;
  }

  onOK() {
    let tips = this.i18n.get('protection_filelevel_restore_tip_label', [
      this.getTargetPath().tips
    ]);
    const targetPath = this.getTargetPath().targetPath;
    const targetLabel = this.i18n.get('protection_restore_target_label');
    this.warningMessageService.create({
      header: this.i18n.get('common_restore_tips_label'),
      content: `${tips}
      <br><br>
      <div *ngIf="${targetPath}">
      <span class='warning-class'>${targetLabel}</span>
      <br>
      <span>${targetPath}</span>
    </div>`,
      width: MODAL_COMMON.smallWidth + 50,
      onOK: () => {
        const params = this.getParams();
        this.restoreService
          .createRestoreV1RestoresPost({ body: params })
          .subscribe(res => {
            this.modal.close();
          });
      }
    });
  }

  getVm() {
    if (this.childResType === DataMap.Resource_Type.HCSCloudHost.value) {
      return find(this.cloudHostOptions, {
        value: this.formGroup.value.cloudHost
      });
    }
    if (
      this.childResType === DataMap.Resource_Type.openStackCloudServer.value
    ) {
      return first(this.formGroup.value.targetServer);
    }
    if (this.childResType === DataMap.Resource_Type.APSCloudServer.value) {
      return this.formGroup.value.targetServer[0];
    }
    return first(this.formGroup.value.vm);
  }

  testConnection() {
    const vm: any = this.getVm();
    const params = {
      username: this.formGroup.value.userName,
      password: this.formGroup.value.password,
      osType:
        this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
          ? this.resourceProperties?.extendInfo?.os_type
          : vm?.extendInfo?.os_type || '',
      vmIp: this.getVmIp()
    };
    if (includes([DataMap.Resource_Type.cNwareVm.value], this.childResType)) {
      const osType =
        this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
          ? JSON.parse(this.resourceProperties?.extendInfo?.details || '{}')
              .osType
          : JSON.parse(vm?.extendInfo?.details || '{}').osType;
      assign(params, {
        osType:
          osType === 2
            ? DataMap.vmwareOsType.windows.value
            : DataMap.vmwareOsType.linux.value
      });
    }
    // 使用选择的操作系统
    if (this.manualOstype()) {
      assign(params, {
        osType: this.formGroup.get('osType').value
      });
    }
    if (this.showPort()) {
      assign(params, {
        port: this.formGroup.get('port')?.value
      });
    }
    this.restoreFilesControllerService
      .checkDestConnection({
        checkDestConnectionRequest: params,
        akOperationTips: false
      })
      .subscribe({
        next: () => {
          this.messageService.success(
            this.i18n.get('common_test_link_success_label'),
            {
              lvMessageKey: 'successKey',
              lvShowCloseButton: true
            }
          );
          this.isTest = true;
        },
        error: () => {
          this.isTest = false;
        }
      });
  }
}
