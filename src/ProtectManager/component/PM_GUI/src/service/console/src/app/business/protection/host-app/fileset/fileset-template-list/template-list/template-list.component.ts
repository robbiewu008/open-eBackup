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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMapService,
  FilesetTemplatesApiService,
  getPermissionMenuItem,
  GlobalService,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleOperationMap,
  WarningMessageService
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  includes,
  isEmpty,
  map,
  size,
  trim
} from 'lodash';
import { CreateTemplateComponent } from './create-template/create-template.component';
import { TemplateDetailComponent } from './template-detail/template-detail.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-template-list',
  templateUrl: './template-list.component.html',
  styleUrls: ['./template-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class TemplateListComponent implements OnInit, OnDestroy {
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE;
  pageIndex = CommonConsts.PAGE_START;
  total = CommonConsts.PAGE_TOTAL;
  selection = [];
  deleteBtnDisabled = true;
  queryName;
  tableData = [];
  filterParams = {};
  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name',
      isShow: true
    },
    {
      label: this.i18n.get('protection_os_type_label'),
      key: 'osType',
      filter: true,
      filterMap: this.dataMapService.toArray('Fileset_Template_Os_Type'),
      isShow: true
    },
    {
      label: this.i18n.get('protection_associating_filesets_label'),
      key: 'associatedFilesetsNum',
      isShow: true
    },
    {
      key: 'labelList',
      label: this.i18n.get('common_tag_label'),
      isShow: true
    }
  ];

  moreMenus = [];
  tableColumnKey = 'protection_fileset_template_table';
  columnStatus = this.rememberColumnsService.getColumnsStatus(
    this.tableColumnKey
  );

  groupCommon = GROUP_COMMON;
  activeItem;

  roleOperationMap = RoleOperationMap;

  destroy$ = new Subject();
  registerTipShow = false;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private globalService: GlobalService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    public rememberColumnsService: RememberColumnsService,
    private filesetTemplatesApiService: FilesetTemplatesApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getMoreMenus();
    this.getColumnStatus();
    this.getTemplates();
    this.virtualScroll.getScrollParam(400);
    this.getUserGuideState();
    this.showRegisterTip();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showRegisterTip();
      });
  }

  showRegisterTip() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.showTips) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

  getColumnStatus() {
    if (!isEmpty(this.columnStatus)) {
      each(this.columns, col => {
        col.isShow = this.columnStatus[col.key];
      });
    }
  }

  getTemplates() {
    const params = {
      pageNo: this.pageIndex + 1,
      pageSize: this.pageSize
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
    this.filesetTemplatesApiService
      .listUsingGET({ ...params })
      .subscribe(res => {
        this.tableData = res.records;
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  getMoreMenus() {
    const menus = [
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disabled: !size(this.selection),
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(this.selection)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disabled: !size(this.selection),
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(this.selection)
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  optsCallback = data => {
    const menus = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyHostFileset,
        disabled: !hasResourcePermission(data),
        onClick: () => this.createTemplate(data)
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.ModifyHostFileset,
        disabled: !hasResourcePermission(data),
        onClick: () => this.createTemplate(assign({}, data, { isClone: true }))
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        disabled: !!data.associatedFilesetsNum || !hasResourcePermission(data),
        tips: !!data.associatedFilesetsNum
          ? this.i18n.get('protection_delete_template_disable_label')
          : '',
        permission: OperateItems.DeleteHostFileset,
        onClick: () => this.deleteTemplate([data])
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag([data])
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag([data])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      onOk: () => {
        this.selection = [];
        this.getTemplates();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data,
      onOk: () => {
        this.selection = [];
        this.getTemplates();
      }
    });
  }

  createTemplate(item?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: item
        ? item.isClone
          ? this.i18n.get('common_clone_label')
          : this.i18n.get('common_modify_label')
        : this.i18n.get('protection_create_template_label'),
      lvContent: CreateTemplateComponent,
      lvOkDisabled: !item,
      lvWidth: MODAL_COMMON.smallModal,
      lvComponentParams: {
        rowItem: item
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateTemplateComponent;
          content.create(item).subscribe({
            next: () => {
              resolve(true);
              this.getTemplates();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  deleteTemplate(datas) {
    const names = map(datas, 'name');
    this.warningMessageService.create({
      content: this.i18n.get('protection_template_delete_label', [
        names.join()
      ]),
      onOK: () => {
        if (datas.length === 1) {
          this.filesetTemplatesApiService
            .deleteUsingDELETE({
              templateId: datas[0].uuid
            })
            .subscribe(res => {
              this.selection = [];
              this.deleteBtnDisabled = true;
              this.getTemplates();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.filesetTemplatesApiService.deleteUsingDELETE({
                templateId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            datas,
            () => {
              this.selection = [];
              this.deleteBtnDisabled = true;
              this.getTemplates();
            }
          );
        }
      }
    });
  }

  searchFilesets(name) {
    assign(this.filterParams, {
      name: trim(name)
    });
    this.getTemplates();
  }

  searchByName(name) {
    assign(this.filterParams, {
      name: trim(name)
    });
    this.searchFilesets(name);
  }

  searchByLabel(label) {
    assign(this.filterParams, {
      labelCondition: {
        labelName: trim(label)
      }
    });
    this.getTemplates();
  }

  selectionChange() {
    this.deleteBtnDisabled =
      size(
        filter(this.selection, val => {
          return !val.associated_filesets_num && hasResourcePermission(val);
        })
      ) !== size(this.selection) || !size(this.selection);
    each(this.moreMenus, item => {
      if (!size(this.selection)) {
        item.tips = '';
        return (item.disabled = true);
      }
      if (item.id === 'addTag') {
        item.disabled = !size(this.selection);
      } else if (item.id === 'removeTag') {
        item.disabled = !size(this.selection);
      }
    });
  }

  filterChange(e) {
    assign(this.filterParams, {
      osType: e.value
    });
    this.getTemplates();
  }

  pageChange(page) {
    this.pageIndex = page.pageIndex;
    this.pageSize = page.pageSize;
    this.getTemplates();
  }

  getDetail(item, activeIndex?) {
    this.activeItem = item;
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'fileset-template-detail',
      lvHeader: item.name,
      lvContent: TemplateDetailComponent,
      lvWidth: MODAL_COMMON.smallModal,
      lvComponentParams: {
        rowItem: assign({}, item),
        activeIndex: activeIndex ? '2' : '1'
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ],
      lvAfterClose: () => {
        // 关闭详情框，取消激活
        this.activeItem = {};
        this.cdr.detectChanges();
      }
    });
  }

  getAssociatedFileset(item) {
    this.getDetail(item, true);
  }

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }
}
