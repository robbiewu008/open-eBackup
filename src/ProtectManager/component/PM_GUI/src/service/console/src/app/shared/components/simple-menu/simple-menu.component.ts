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
  ViewChild,
  ElementRef,
  ViewContainerRef,
  TemplateRef,
  Input
} from '@angular/core';
import { Overlay, OverlayConfig } from '@angular/cdk/overlay';
import { OverlayService } from '@iux/live';
import { TemplatePortal } from '@angular/cdk/portal';
import { cloneDeep as _cloneDeep } from 'lodash';
import { Router } from '@angular/router';

@Component({
  selector: 'aui-simple-menu',
  templateUrl: 'simple-menu.component.html',
  styles: [
    `
      .aui-header-menu-simple {
        margin-right: 20px;
      }
    `
  ]
})
export class SimpleMenuComponent implements OnInit {
  @Input() menus;
  @Input() activeId;
  iconMenus;
  overlayRef;
  @ViewChild('menuTpl', { static: false }) template: TemplateRef<any>;
  @ViewChild('overlayBtn', { read: ElementRef, static: false })
  overlayBtn: ElementRef<any>;

  constructor(
    private viewContainerRef: ViewContainerRef,
    private overlayService: OverlayService,
    private overlay: Overlay,
    private router: Router
  ) {}

  ngOnInit() {
    this.initData();
  }

  initData() {
    const convert = data => {
      const link = data.routerLink;
      if (!link) {
        return data;
      }
      data.onClick = () => {
        this.router.navigateByUrl(link);
      };
      delete data.routerLink;
      return data;
    };
    this.iconMenus = _cloneDeep(this.menus).map((item: any) => {
      if (item.items && item.items.length) {
        item.items = item.items[0].items.map(convert);
      }
      return convert(item);
    });
  }

  openMenu() {
    if (!this.overlayRef || !this.overlayRef.overlayRef.hasAttached()) {
      const config = new OverlayConfig({
        hasBackdrop: false,
        positionStrategy: this.overlay
          .position()
          .flexibleConnectedTo(this.overlayBtn)
          .withPositions([
            {
              originX: 'start',
              offsetX: -25,
              offsetY: 20,
              originY: 'bottom',
              overlayX: 'start',
              overlayY: 'top'
            }
          ])
      });

      const portal = new TemplatePortal(this.template, this.viewContainerRef);

      this.overlayRef = this.overlayService.open(portal, config);
    }
  }

  close() {
    this.overlayRef.close();
  }
}
