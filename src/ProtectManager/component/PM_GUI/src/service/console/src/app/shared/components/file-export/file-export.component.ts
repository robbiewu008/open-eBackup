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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  CopyControllerService,
  I18NService,
  RestoreFileType,
  RestoreManagerService as RestoreServiceApi,
  DataMap,
  CAPACITY_UNIT
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  isArray,
  isEmpty,
  map,
  reject,
  size,
  unionBy
} from 'lodash';
import { Subject } from 'rxjs';
import { DownloadFlrFilesComponent } from '../download-flr-files/download-flr-files.component';
import { ExportFilesService } from '../export-files/export-files.component';

@Component({
  selector: 'aui-file-export',
  templateUrl: './file-export.component.html',
  styleUrls: ['./file-export.component.less']
})
export class FileExportComponent implements OnInit {
  @Input() rowItem;
  treeTableData = [];
  treeTableSelection = [];
  fileValid$ = new Subject<boolean>();
  unitconst = CAPACITY_UNIT;
  RestoreFileType = RestoreFileType;

  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );

  downloadFlrFilesComponent = new DownloadFlrFilesComponent(
    this.exportFilesService
  );

  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private restoreServiceApi: RestoreServiceApi,
    private copyControllerService: CopyControllerService,
    private exportFilesService: ExportFilesService
  ) {}

  ngOnInit() {
    this.getOriginalPath();
  }

  getOriginalPath() {
    this.treeTableData = [
      {
        children: [],
        hasChildren: true,
        name: this.rowItem.resource_name,
        parent_uuid: null,
        path: '/',
        rootPath: '/',
        icon: 'aui-icon-nas-file',
        size: '--'
      }
    ];
  }

  expandedChange(node) {
    if (!node.expanded || !!size(node.children)) {
      return;
    }
    this.getResource(node);
  }

  getResource(node, startPage?) {
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowItem.uuid,
        parentPath: node.rootPath || '/'
      })
      .subscribe(res => this.updataNode(res, node));
  }

  updataNode(res, node) {
    each(res.records, item => {
      assign(item, {
        name: item['path'],
        rootPath:
          node.rootPath === '/'
            ? `/${item['path']}`
            : `${node.rootPath}/${item['path']}`,
        isLeaf: item.type !== RestoreFileType.Directory,
        children: item.type === RestoreFileType.Directory ? [] : null,
        icon:
          item.type === RestoreFileType.Directory
            ? 'aui-icon-directory'
            : 'aui-icon-file',
        size: item.size
      });
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
        name: `${this.i18n.get('common_more_label')}...`,
        isMoreBtn: true,
        hasChildren: false,
        isLeaf: true,
        children: null,
        startPage: Math.floor(size(node.children) / 200)
      };
      node.children = [...node.children, moreClickNode];
    }
    if (find(this.treeTableSelection, node)) {
      this.treeTableSelection = [...this.treeTableSelection, ...res.records];
    }
    this.treeTableData = [...this.treeTableData];
  }

  selectionChange(event) {
    this.fileValid$.next(!!size(this.treeTableSelection));
  }

  getPath(paths) {
    let filterPaths = [];
    let childPaths = [];
    paths = reject(paths, { rootPath: '' });
    paths = reject(paths, { isMoreBtn: true });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'rootPath');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(find(childPaths, { rootPath: item.rootPath }));
    });

    return map(filterPaths, 'rootPath');
  }

  trackByIndex(index) {
    return index;
  }

  downloadFile() {
    const paths = this.getPath(cloneDeep(this.treeTableSelection));
    const copyId = this.rowItem.uuid;
    let memberEsn = '';
    if (this.rowItem.device_esn) {
      memberEsn = this.rowItem.device_esn;
    }
    this.downloadFlrFilesComponent.getRequestId(paths, copyId, memberEsn);
  }
}
