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
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  ViewEncapsulation
} from '@angular/core';
import { doFnResult, TypeUtils, MenuItem } from '@iux/live';
import { merge as _merge, includes } from 'lodash';
import { Observable, Subject, takeUntil } from 'rxjs';
import { ProButton } from './interface';
import {
  GlobalService,
  I18NService,
  RoleOperationAuth,
  RoleOperationMap
} from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

const DEFAULT_CONFIG: ProButton = {
  type: 'default',
  size: 'default',
  label: 'More'
};

@Component({
  selector: 'lv-pro-button',
  templateUrl: './pro-button.component.html',
  styleUrls: ['./pro-button.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  encapsulation: ViewEncapsulation.None,
  host: {
    '[style.display]': '_getDisplayStatus(button) ? "" : "none"'
  }
})
export class ProButtonComponent implements OnInit, AfterViewInit, OnDestroy {
  @Input() bindData: any[]; // 按钮操作的对象
  @Input() config: ProButton;
  @Input() mode: 'link' | 'button' = 'button';

  isGroup = false;
  button;
  menus: any = [];
  loadingText: string;
  isLoading = false;
  popoverShow = false;
  public typeUtils = TypeUtils;

  destroy$ = new Subject();

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  constructor(
    private cdr: ChangeDetectorRef,
    private i18n: I18NService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngAfterViewInit(): void {
    this._showRegisterTip();
  }

  ngOnInit() {
    if (!this.config.label) {
      this.config.label = this.i18n.get('common_more_label');
    }
    this.button = _merge({}, DEFAULT_CONFIG, this.config);
    this._getUserGuideState();
  }

  _getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        if (res.showTips) {
          this.button.popoverShow = true;
          USER_GUIDE_CACHE_DATA.showTips = true;
          this._showRegisterTip();
        }
      });
  }

  _showRegisterTip() {
    if (
      this.button.popoverShow &&
      USER_GUIDE_CACHE_DATA.showTips &&
      includes(this.roleOperationAuth, this.roleOperationMap.manageResource)
    ) {
      setTimeout(() => {
        this.popoverShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.popoverShow = false;
    this.cdr.detectChanges();
  };

  _getDisableStatus(button) {
    const _data = this.bindData || [];
    return button.disableCheck ? button.disableCheck(_data) : false;
  }

  _getDisplayStatus(button) {
    const _data = this.bindData || [];
    return button.displayCheck ? button.displayCheck(_data) : true;
  }

  _getDisableTips(button) {
    const _data = this.bindData || [];
    return button.disabledTipsCheck ? button.disabledTipsCheck(_data) : '';
  }

  _buttonClick($event) {
    const call: any =
      this.button && this.button.onClick
        ? this.button.onClick(this.bindData, $event)
        : undefined;
    if (call instanceof Promise || call instanceof Observable) {
      this.isLoading = true;
      if (this.typeUtils.isRealString(this.button.loadingText)) {
        this.loadingText = this.button.loadingText;
      } else {
        this.loadingText = this.i18n.get('common_loading_label');
      }
    }
    doFnResult(call, _result => {
      this.isLoading = false;
      this.cdr.markForCheck();
    });
  }

  _moreBtnClick(e, button) {
    this.menus = [];
    this._dropdownMenuMap(button.items, this.menus);

    this.cdr.markForCheck();
    e.stopPropagation();
    e.preventDefault();
  }

  _dropdownMenuMap(data, menus) {
    data.map((item, index) => {
      const obj: MenuItem = {
        id: item.id || index,
        label: item.label,
        icon: item.icon,
        divide: item.divide,
        tips: this._getDisableStatus(item)
          ? this._getDisableTips(item) || item.disabledTips
          : '',
        disabled: this._getDisableStatus(item),
        hidden: !this._getDisplayStatus(item),
        items: []
      };
      if (item.items) {
        this._dropdownMenuMap(item.items, obj.items);
      } else {
        delete obj.items;
      }
      if (item.onClick) {
        obj['onClick'] = event => {
          item.onClick(this.bindData, event);
          event.event.stopPropagation();
          event.event.preventDefault();
        };
      }
      menus.push(obj);
    });
  }
}
