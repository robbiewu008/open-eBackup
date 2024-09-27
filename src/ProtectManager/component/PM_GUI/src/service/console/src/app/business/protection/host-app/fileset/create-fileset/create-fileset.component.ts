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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import {
  FormBuilder,
  FormControl,
  FormGroup,
  Validators
} from '@angular/forms';
import { MessageService, TreeComponent } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMapService,
  I18NService,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import {
  EnvironmentsService,
  HostService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared/api/services';
import {
  DataMap,
  FilterBy,
  FilterMode,
  RestoreFileType
} from 'app/shared/consts';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import {
  assign,
  cloneDeep,
  each,
  endsWith,
  filter,
  find,
  includes,
  isArray,
  isEmpty,
  isNumber,
  map,
  omit,
  reject,
  size,
  startsWith,
  union,
  unionBy,
  trim,
  set
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-create-fileset',
  templateUrl: './create-fileset.component.html',
  styleUrls: ['./create-fileset.component.less']
})
export class CreateFilesetComponent implements OnInit {
  formGroup: FormGroup;
  fileValid$ = new Subject<boolean>();
  lvShowCheckbox = false;
  @Input() rowItem;
  hostOptions = [];
  filesData = [];
  fileSelection = [];
  filterParams = [] as any;
  filesetNameErrorTip: any = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_label')
  };
  osType;
  @Input() sub_type;
  copyRowItemPaths;
  selectionPath = [];
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  pageSizeOptions = [CommonConsts.PAGE_SIZE_SMALL];
  dataMap = DataMap;
  selectHost;
  treeLabel;
  @ViewChild('page', { static: false }) page;

  @ViewChild('resourceFilter', { static: false }) filterComponent;
  @ViewChild(TreeComponent, { static: false }) lvTree: TreeComponent;
  @ViewChild('namePopover', { static: false }) namePopover;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    private messageService: MessageService,
    public baseUtilService: BaseUtilService,
    public hostApiService: HostService,
    public dataMapService: DataMapService,
    public environmentsApiService: EnvironmentsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      selectedHost: new FormControl(
        this.rowItem
          ? this.sub_type === DataMap.Resource_Type.HDFS.value
            ? this.rowItem.environment_uuid
            : this.rowItem.environment?.uuid
          : '',
        Validators.required
      ),
      name: new FormControl(this.rowItem ? this.rowItem.name : '')
    });
    if (this.sub_type === DataMap.Resource_Type.fileset.value) {
      this.formGroup
        .get('name')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.baseUtilService.VALID.name(
            /^[\u4e00-\u9fa5a-zA-Z_][\u4e00-\u9fa5a-zA-Z_0-9-]*$/
          )
        ]);
      this.filesetNameErrorTip = {
        ...this.baseUtilService.requiredErrorTip,
        invalidName: this.i18n.get('protection_fileset_name_vaild_label'),
        invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
      };
    } else {
      this.formGroup
        .get('name')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
        ]);
      this.filesetNameErrorTip = {
        ...this.baseUtilService.requiredErrorTip,
        invalidName: this.i18n.get('common_valid_name_label')
      };
    }
    if (this.rowItem) {
      this.copyRowItemPaths = cloneDeep(
        JSON.parse(this.rowItem.extendInfo.paths)
      );
      this.selectionPath = map(this.copyRowItemPaths, value => {
        return {
          path: value
        };
      });
    }
  }

  selectionChange() {
    if (this.rowItem) {
      this.setNodeIndeterminate(this.filesData);
      this.filesData = [...this.filesData];
    }
    this.validPath();
    this.getSelectionPath();
  }

  validPath() {
    if (this.rowItem) {
      const paths = this.getPath(this.fileSelection);
      this.fileValid$.next(
        size(union(paths, this.copyRowItemPaths)) > 0 &&
          size(union(paths, this.copyRowItemPaths)) <=
            (this.sub_type === DataMap.Resource_Type.HDFS.value ? 256 : 64)
      );
    } else {
      this.fileValid$.next(
        size(this.fileSelection) > 0 &&
          size(this.getPath(this.fileSelection)) <=
            (this.sub_type === DataMap.Resource_Type.HDFS.value ? 256 : 64)
      );
    }
  }

  getSelectionPath() {
    let paths = this.getPath(this.fileSelection);
    if (this.rowItem) {
      paths = union(paths, this.copyRowItemPaths);
    }
    this.selectionPath = map(paths, value => {
      return {
        path: value
      };
    });
  }

  removePathFormTable(item) {
    this.selectionPath = reject(this.selectionPath, v => {
      return item.path === v.path;
    });
    // 当前移除的节点
    const currentNode = find(this.fileSelection, v => {
      return item.path === v.extendInfo?.path;
    });
    this.fileSelection = reject(this.fileSelection, v => {
      return item.path === v.extendInfo?.path;
    });
    // 移除子节点
    if (currentNode && !isEmpty(currentNode.children)) {
      this.fileSelection = reject(this.fileSelection, v => {
        return includes(
          map(currentNode.children, 'extendInfo.path'),
          v.extendInfo?.path
        );
      });
    }
    if (this.rowItem) {
      this.copyRowItemPaths = reject(this.copyRowItemPaths, v => {
        return item.path === v;
      });
    }
    if (this.rowItem) {
      this.setNodeIndeterminate(this.filesData);
      this.filesData = [...this.filesData];
    }
    this.validPath();
    if (
      size(this.getPath(this.fileSelection)) > 64 &&
      this.sub_type === DataMap.Resource_Type.fileset.value
    ) {
      this.messageService.error(this.i18n.get('protection_max_fileset_label'), {
        lvShowCloseButton: true,
        lvMessageKey: 'filesetSLAMessageKey'
      });
    }
  }

  setNodeIndeterminate(sourceData) {
    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      this.setHDFSNodeIndeterminate(sourceData);
    } else {
      const paths = this.getPath(this.fileSelection);
      const selectedPaths = union(paths, this.copyRowItemPaths);
      sourceData.forEach(element => {
        if (
          (!isEmpty(this.hostOptions) &&
            find(this.hostOptions, {
              key: this.formGroup.value.selectedHost
            }).os_type === DataMap.Os_Type.windows.value) ||
          (!isEmpty(this.rowItem) &&
            this.rowItem.environment_os_type === DataMap.Os_Type.windows.value)
        ) {
          if (
            find(selectedPaths, item => {
              return (
                includes(
                  item,
                  endsWith(element.extendInfo?.path, '\\')
                    ? element.extendInfo?.path
                    : element.extendInfo?.path + '\\'
                ) || isEmpty(element.extendInfo?.path)
              );
            })
          ) {
            element.indeterminate = true;
          } else {
            element.indeterminate = false;
          }
        } else {
          if (
            find(selectedPaths, item => {
              return (
                includes(
                  item,
                  endsWith(element.extendInfo?.path, '/')
                    ? element.extendInfo?.path
                    : element.extendInfo?.path + '/'
                ) || isEmpty(element.extendInfo?.path)
              );
            })
          ) {
            element.indeterminate = true;
          } else {
            element.indeterminate = false;
          }
        }
        if (element.children) {
          this.setNodeIndeterminate(element.children);
        }
      });
    }
  }

  setHDFSNodeIndeterminate(sourceData) {
    const paths = this.getPath(this.fileSelection);
    const selectedPaths = union(paths, this.copyRowItemPaths);
    sourceData.forEach(element => {
      if (
        find(selectedPaths, item => {
          return (
            includes(
              item,
              endsWith(element?.extendInfo?.path, '/')
                ? element?.extendInfo?.path
                : element?.extendInfo?.path + '/'
            ) || isEmpty(element?.extendInfo?.path)
          );
        })
      ) {
        element.indeterminate = true;
      } else {
        element.indeterminate = false;
      }
      if (element.children) {
        this.setHDFSNodeIndeterminate(element.children);
      }
    });
  }
  addData(array: any[], item) {
    array.push({
      key: item.uuid,
      value: item.uuid,
      label: !isEmpty(item.environment?.endpoint)
        ? `${item.environment?.name}(${item.environment?.endpoint})`
        : item.environment?.name,
      os_type: item.environment?.osType,
      parentUuid: item.parentUuid,
      isLeaf: true
    });
  }
  getHosts(recordsTemp?, startPage?) {
    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      return;
    }
    if (this.rowItem) {
      this.filesData = [
        {
          label: `${this.rowItem.environment?.name}(${this.rowItem.environment?.endpoint})`,
          uuid: this.rowItem.environment?.uuid,
          contentToggleIcon: 'aui-icon-host',
          children: [],
          isHost: true,
          extendInfo: {
            path: '/'
          }
        }
      ];
      this.osType = this.rowItem.environment?.osType;
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: ['FilesetPlugin']
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          recordsTemp = filter(recordsTemp, item => !isEmpty(item.environment));
          const hostArr = [];
          if (!MultiCluster.isMulti) {
            recordsTemp = filter(
              recordsTemp,
              item =>
                item.environment?.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          } else {
            recordsTemp = getMultiHostOps(recordsTemp);
          }
          each(recordsTemp, item => {
            if (isEmpty(item.environment?.extendInfo?.install_path)) {
              if (item.environment?.osType === DataMap.Os_Type.windows.value) {
                set(item.environment?.extendInfo, 'install_path', 'C:\\');
              } else {
                set(item.environment?.extendInfo, 'install_path', '/opt');
              }
            }

            hostArr.push({
              key: item.uuid,
              value: item.uuid,
              label: !isEmpty(item.environment?.endpoint)
                ? `${item.environment?.name}(${item.environment?.endpoint})`
                : item.environment?.name,
              os_type: item.environment?.osType,
              parentUuid: item.parentUuid,
              installPath: trim(item.environment?.extendInfo?.install_path),
              isLeaf: true
            });
          });
          this.hostOptions = hostArr;
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  getClusters(recordsTemp?, startPage?) {
    if (this.sub_type !== DataMap.Resource_Type.HDFS.value) {
      return;
    }
    if (this.rowItem) {
      this.filesData = [
        {
          label: !isEmpty(this.rowItem.environment_endpoint)
            ? `${this.rowItem.environment_name}(${this.rowItem.environment_endpoint})`
            : this.rowItem.environment_name,
          uuid: this.rowItem.environment_uuid,
          contentToggleIcon: 'aui-icon-cluster',
          children: [],
          isHost: true,
          extendInfo: {
            path: '/'
          }
        }
      ];
    }
    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.HDFS.value
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          const hostArr = [];
          each(recordsTemp, item => {
            if (
              item.linkStatus ===
              this.dataMapService.getConfig('resource_LinkStatus_Special')
                .normal.value
            ) {
              hostArr.push({
                ...item,
                key: item.uuid,
                value: item.uuid,
                label: item.name,
                isLeaf: true
              });
            }
          });
          this.hostOptions = hostArr;
          return;
        }
        this.getClusters(recordsTemp, startPage);
      });
  }

  getFiles() {
    this.lvShowCheckbox = false;
    const rootNode = {
      label: find(this.hostOptions, {
        key: this.formGroup.value.selectedHost
      }).label,
      uuid: this.formGroup.value.selectedHost,
      contentToggleIcon:
        this.sub_type === DataMap.Resource_Type.HDFS.value
          ? 'aui-icon-cluster'
          : 'aui-icon-host',
      children: [],
      isHost: true
    };
    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      assign(rootNode, {
        extendInfo: {
          path: '/'
        }
      });
    } else {
      assign(rootNode, {
        extendInfo: {
          path: this.osType === DataMap.Os_Type.windows.value ? '' : '/'
        }
      });
    }
    this.filesData = [rootNode];
  }

  hostChange(e) {
    this.fileSelection = [];
    this.fileValid$.next(false);
    this.selectionChange();
    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      this.osType = DataMap.Os_Type.linux.value;
    } else {
      this.selectHost = cloneDeep(
        find(this.hostOptions, {
          key: this.formGroup.value.selectedHost
        })
      );
      this.osType = this.selectHost?.os_type;
    }
    this.getFiles();
  }

  getFilesetResource(node, startPage?) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: this.rowItem
          ? this.rowItem.environment?.uuid
          : find(this.hostOptions, {
              key: this.formGroup.value.selectedHost
            })?.parentUuid,
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 100,
        parentId: node.extendInfo?.path || '',
        resourceType: DataMap.Resource_Type.fileset.value
      })
      .subscribe(res => {
        this.lvShowCheckbox = true;
        each(res.records, (item: any) => {
          item.label = (() => {
            if (
              (!isEmpty(this.hostOptions) &&
                find(this.hostOptions, {
                  key: this.formGroup.value.selectedHost
                }).os_type ===
                  this.dataMapService.getConfig('Os_Type').windows.value) ||
              (!isEmpty(this.rowItem) &&
                this.rowItem.environment_os_type ===
                  this.dataMapService.getConfig('Os_Type').windows.value)
            ) {
              return endsWith(node.extendInfo?.path, '\\')
                ? item.extendInfo?.path.replace(node.extendInfo?.path, '')
                : item.extendInfo?.path.replace(
                    node.extendInfo?.path + '\\',
                    ''
                  );
            } else {
              return node.extendInfo?.path === '/'
                ? item.extendInfo?.path.replace('/', '')
                : item.extendInfo?.path.replace(
                    node.extendInfo?.path + '/',
                    ''
                  );
            }
          })();
          item.isLeaf = item.extendInfo?.type === RestoreFileType.File;
          item.contentToggleIcon =
            ((!isEmpty(this.hostOptions) &&
              find(this.hostOptions, {
                key: this.formGroup.value.selectedHost
              }).os_type === DataMap.Os_Type.windows.value) ||
              (!isEmpty(this.rowItem) &&
                this.rowItem.environment_os_type ===
                  DataMap.Os_Type.windows.value)) &&
            node.isHost
              ? 'aui-icon-volume'
              : item.extendInfo?.type === RestoreFileType.Directory
              ? 'aui-icon-directory'
              : 'aui-icon-file';

          let diablePath = '';
          const selectHostPath = !isEmpty(this.rowItem)
            ? trim(this.rowItem.environment?.extendInfo?.install_path)
            : cloneDeep(this.selectHost?.installPath);
          const selectOsType = !isEmpty(this.rowItem)
            ? this.rowItem.environment_os_type
            : this.osType;
          if (selectOsType === DataMap.Os_Type.windows.value) {
            if (endsWith(selectHostPath, '\\')) {
              diablePath = selectHostPath + 'DataBackup';
            } else {
              diablePath = selectHostPath + '\\DataBackup';
            }
          } else {
            diablePath = selectHostPath + '/DataBackup';
          }
          item.disabled = startsWith(item?.extendInfo?.path, diablePath);

          if (
            !isEmpty(this.rowItem) &&
            !isEmpty(this.copyRowItemPaths) &&
            includes(this.copyRowItemPaths, item.extendInfo?.path)
          ) {
            this.fileSelection.push(item);
          }
        });
        if (
          !isEmpty(this.rowItem) &&
          JSON.parse(this.rowItem.extendInfo.paths)[0] !== '/'
        ) {
          if (node.isHost && !isEmpty(this.copyRowItemPaths)) {
            node.indeterminate = true;
          }
          if (
            this.rowItem.environment_os_type === DataMap.Os_Type.windows.value
          ) {
            this.handleWinodwsCheckboxStatus(res.records);
          } else {
            this.handleCheckboxStatus(res.records);
          }
        }
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children = [...res.records];
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(size(node.children) / 100)
          };
          node.children = [...node.children, moreClickNode];
        }
        this.setNodeIndeterminate(this.filesData);
        this.filesData = [...this.filesData];
        if (find(this.fileSelection, node)) {
          this.fileSelection = [...this.fileSelection, ...res.records];
        }
        this.fileSelection = [...this.fileSelection];
      });
  }

  getHDFSResource(node, startPage?) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: this.formGroup.value.selectedHost,
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: 100,
        parentId: !node.extendInfo?.path ? '/' : node.extendInfo?.path || '',
        resourceType: DataMap.Resource_Type.HDFS.value
      })
      .subscribe(res => {
        this.lvShowCheckbox = true;
        each(res.records, (item: any) => {
          item.label =
            item.extendInfo?.fileName === '/'
              ? item.extendInfo?.fileName.replace('/', '')
              : item.extendInfo?.fileName.replace(
                  item.extendInfo?.fileName + '/',
                  ''
                );
          item.isLeaf = item.extendInfo?.isDirectory !== 'true';
          item.contentToggleIcon =
            item.extendInfo?.isDirectory === 'true'
              ? 'aui-icon-directory'
              : 'aui-icon-file';
          if (
            !isEmpty(this.rowItem) &&
            !isEmpty(this.copyRowItemPaths) &&
            includes(this.copyRowItemPaths, item.extendInfo?.path)
          ) {
            this.fileSelection.push(item);
          }
        });
        if (
          !isEmpty(this.rowItem) &&
          JSON.parse(this.rowItem.extendInfo.paths)[0] !== '/'
        ) {
          if (node.isHost && !isEmpty(this.copyRowItemPaths)) {
            node.indeterminate = true;
          }
          this.handleCheckboxStatus(res.records);
        }
        if (
          !isEmpty(this.rowItem) &&
          node.isHost &&
          includes(this.copyRowItemPaths, node.extendInfo?.path)
        ) {
          this.fileSelection.push(node);
        }
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children = [...res.records];
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(size(node.children) / 100) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.setHDFSNodeIndeterminate(this.filesData);
        this.filesData = [...this.filesData];
        if (find(this.fileSelection, node)) {
          this.fileSelection = [...this.fileSelection, ...node.children];
        }
        this.fileSelection = [...this.fileSelection];
      });
  }

  handleCheckboxStatus(sourceData) {
    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      this.handleHDFSCheckboxStatus(sourceData);
    } else {
      sourceData.forEach(element => {
        if (
          find(this.copyRowItemPaths, item => {
            return includes(
              item,
              endsWith(element.extendInfo?.path, '/')
                ? element.extendInfo?.path
                : element.extendInfo?.path + '/'
            );
          })
        ) {
          element.indeterminate = true;
        }
      });
    }
  }

  handleHDFSCheckboxStatus(sourceData) {
    sourceData.forEach(element => {
      if (
        find(this.copyRowItemPaths, item => {
          return includes(
            item,
            endsWith(element.extendInfo.path, '/')
              ? element.extendInfo.path
              : element.extendInfo.path + '/'
          );
        })
      ) {
        element.indeterminate = true;
      }
    });
  }

  handleWinodwsCheckboxStatus(sourceData) {
    sourceData.forEach(element => {
      if (
        find(this.copyRowItemPaths, item => {
          return includes(
            item,
            endsWith(element.extendInfo?.path, '\\')
              ? element.extendInfo?.path
              : element.extendInfo?.path + '\\'
          );
        })
      ) {
        element.indeterminate = true;
      }
    });
  }

  expandedChange(node) {
    if (!node.expanded || !!size(node.children)) {
      if (this.rowItem && node.expanded) {
        setTimeout(() => {
          this.filesData = [...this.filesData];
        });
      }
      return;
    }

    if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
      this.getHDFSResource(node);
    } else {
      this.getFilesetResource(node);
    }
  }

  getPath(paths) {
    return this.getHDFSPath(paths);
  }

  getHDFSPath(paths) {
    let filterPaths = [];
    let childPaths = [];
    paths = filter(paths, item => {
      return !isEmpty(item.extendInfo?.path);
    });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'extendInfo.path');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(
        find(childPaths, node => {
          return node.extendInfo?.path === item.extendInfo?.path;
        })
      );
    });
    return map(filterPaths, 'extendInfo.path');
  }

  createFileset(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const paths = this.getPath(this.fileSelection);
      this.filterComponent.collectParams();
      const CreateResourceRequestBody = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.selectedHost,
        type: DataMap.Resource_Type.fileset.value,
        subType: DataMap.Resource_Type.fileset.value,
        extendInfo: {
          paths: JSON.stringify(
            map(paths, path => {
              return { name: path };
            })
          ),
          filters: JSON.stringify(this.filterParams)
        }
      };
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody
        })
        .subscribe({
          next: res => {
            cacheGuideResource(res);
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }

  createHDFS(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const paths = this.getPath(this.fileSelection);
      this.filterComponent.collectParams();
      const CreateResourceRequestBody = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.selectedHost,
        type: DataMap.Resource_Type.HDFS.value,
        subType: DataMap.Resource_Type.HDFSFileset.value,
        path: find(this.hostOptions, {
          key: this.formGroup.value.selectedHost
        }).endpoint,
        extendInfo: {
          paths: JSON.stringify(
            map(paths, path => {
              return { name: path };
            })
          ),
          filters: JSON.stringify(
            map(cloneDeep(this.filterParams), obj => {
              obj['filterBy'] = FilterBy[obj['type']];
              obj['type'] =
                FilterBy.FileName === obj['type'] ? 'File' : 'Folder';
              obj['rule'] = 'Fuzzy';
              obj['mode'] = FilterMode[obj['model']];
              obj['values'] = JSON.parse(obj['content']);
              return omit(obj, ['model', 'content']);
            })
          )
        }
      };
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody
        })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          errro => {
            observer.error(errro);
            observer.complete();
          }
        );
    });
  }

  excludeParentFileSelection(node) {
    if (node.parent) {
      this.fileSelection = reject(this.fileSelection, item => {
        return (
          item.extendInfo?.path === node.parent.extendInfo?.path ||
          item?.disabled
        );
      });
      if (find(node.parent?.children, { isMoreBtn: true })) {
        this.fileSelection = reject(this.fileSelection, item => {
          return item.isMoreBtn;
        });
      }
      if (this.rowItem) {
        this.copyRowItemPaths = reject(this.copyRowItemPaths, item => {
          return item === node.parent.extendInfo?.path;
        });
      }
      this.excludeParentFileSelection(node.parent);
    }
  }

  includeChildrenFileSelection(node) {
    if (!isEmpty(node.children)) {
      if (
        find(this.fileSelection, item => {
          return item.extendInfo?.path === node.extendInfo?.path;
        })
      ) {
        this.fileSelection = unionBy(
          this.fileSelection,
          node.children,
          'extendInfo.path'
        );
      } else {
        this.fileSelection = reject(this.fileSelection, item => {
          return includes(
            map(node.children, 'extendInfo.path'),
            item.extendInfo?.path
          );
        });
      }
      each(node.children, item => {
        this.includeChildrenFileSelection(item);
      });
    }
  }

  changeNodeCheckedStatus(node) {
    // 取消父
    this.excludeParentFileSelection(node);
    // 关联子
    this.includeChildrenFileSelection(node);
  }

  nodeCheck(event) {
    this.changeNodeCheckedStatus(event.node);
    if (this.rowItem) {
      if (this.sub_type === DataMap.Resource_Type.HDFS.value) {
        this.nodeHDFSCheck(event);
      } else {
        this.copyRowItemPaths = reject(this.copyRowItemPaths, item => {
          return (
            item === event.node.extendInfo?.path &&
            !includes(this.getPath(this.fileSelection), item)
          );
        });
        this.copyRowItemPaths = reject(this.copyRowItemPaths, v => {
          if (
            this.rowItem.environment_os_type === DataMap.Os_Type.windows.value
          ) {
            return includes(
              v,
              endsWith(event.node.extendInfo?.path, '\\')
                ? event.node.extendInfo?.path
                : event.node.extendInfo?.path + '\\'
            );
          } else {
            return includes(
              v,
              endsWith(event.node.extendInfo?.path, '/')
                ? event.node.extendInfo?.path
                : event.node.extendInfo?.path + '/'
            );
          }
        });
      }
    }
    this.setNodeIndeterminate(this.filesData);
    this.filesData = [...this.filesData];
    this.validPath();
    this.getSelectionPath();
    if (
      size(this.getPath(this.fileSelection)) > 64 &&
      this.sub_type === DataMap.Resource_Type.fileset.value
    ) {
      this.messageService.error(this.i18n.get('protection_max_fileset_label'), {
        lvShowCloseButton: true,
        lvMessageKey: 'filesetSLAMessageKey'
      });
    }
  }

  nodeHDFSCheck(event) {
    this.copyRowItemPaths = reject(this.copyRowItemPaths, item => {
      return (
        (item === event.node.extendInfo.path &&
          !includes(this.getPath(this.fileSelection), item)) ||
        includes(
          item,
          endsWith(event.node.extendInfo.path, '/')
            ? event.node.extendInfo.path
            : event.node.extendInfo.path + '/'
        )
      );
    });
  }

  modifyFileset(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      let paths = this.getPath(this.fileSelection);
      paths = union(paths, this.copyRowItemPaths);
      this.filterComponent.collectParams();
      const UpdateResourceRequestBody = {
        name: this.formGroup.value.name,
        parentUuid:
          this.formGroup.value.selectedHost || this.rowItem.environment?.uuid,
        type: DataMap.Resource_Type.fileset.value,
        subType: DataMap.Resource_Type.fileset.value,
        extendInfo: {
          paths: JSON.stringify(
            map(paths, path => {
              return { name: path };
            })
          ),
          filters: JSON.stringify(this.filterParams),
          templateId: ''
        }
      };
      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.rowItem.uuid,
          UpdateResourceRequestBody
        })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          errro => {
            observer.error(errro);
            observer.complete();
          }
        );
    });
  }

  modifyHDFS(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      let paths = this.getPath(this.fileSelection);
      paths = union(paths, this.copyRowItemPaths);
      this.filterComponent.collectParams();
      const UpdateResourceRequestBody = {
        name: this.formGroup.value.name,
        rootUuid: this.formGroup.value.selectedHost,
        type: DataMap.Resource_Type.HDFS.value,
        subType: DataMap.Resource_Type.HDFSFileset.value,
        extendInfo: {
          paths: JSON.stringify(
            map(paths, path => {
              return { name: path };
            })
          ),
          filters: JSON.stringify(
            map(cloneDeep(this.filterParams), obj => {
              obj['filterBy'] = FilterBy[obj['type']];
              obj['type'] =
                FilterBy.FileName === obj['type'] ? 'File' : 'Folder';
              obj['rule'] = 'Fuzzy';
              obj['mode'] = FilterMode[obj['model']];
              obj['values'] = JSON.parse(obj['content']);
              return omit(obj, ['model', 'content']);
            })
          )
        }
      };

      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.rowItem.uuid,
          UpdateResourceRequestBody
        })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  getModifyParams() {
    let paths = this.getPath(this.fileSelection);
    paths = union(paths, this.copyRowItemPaths);
    this.filterComponent.collectParams();
    const params = {
      name: this.formGroup.value.name,
      paths,
      filters: this.filterParams
    };
    return params;
  }

  resetSelection() {
    this.selectionPath = [];
    this.fileSelection = [];
    this.copyRowItemPaths = [];
    this.selectionChange();
  }

  ngOnInit() {
    this.initForm();
    this.getHosts();
    this.getClusters();
  }

  treeSearch(event) {
    this.lvTree.filter({ filterMode: 'contains', key: 'label', value: event });
    if (this.namePopover) {
      this.namePopover.hide();
    }
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }
}
