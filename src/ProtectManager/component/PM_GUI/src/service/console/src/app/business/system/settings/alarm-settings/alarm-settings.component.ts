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
import { Component, ElementRef, OnInit, Renderer2 } from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { I18NService, LocalStorageApiService, LANGUAGE } from 'app/shared';
import { defer, toString } from 'lodash';

@Component({
  selector: 'aui-alarm-settings',
  templateUrl: './alarm-settings.component.html',
  styleUrls: ['./alarm-settings.component.css']
})
export class AlarmSettingsComponent implements OnInit {
  desc: SafeHtml;
  constructor(
    private el: ElementRef,
    private renderer: Renderer2,
    private sanitizer: DomSanitizer,
    private i18n: I18NService,
    private localStorageApiService: LocalStorageApiService
  ) {
    this.desc = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_alarms_settings_tips_label')
    );
  }

  ngOnInit() {
    defer(() => {
      this.bindClick();
    });
  }

  bindClick() {
    const node = this.el.nativeElement.querySelector(
      '#open-device-alarm-settings'
    );
    if (node) {
      this.renderer.listen(node, 'click', () => {
        this.localStorageApiService
          .getStorageTokenUsingGET({})
          .subscribe(res => {
            const language =
              this.i18n.language.toLowerCase() === LANGUAGE.CN ? 'zh' : 'en';
            const url = `https://${encodeURI(res.ip)}:${encodeURI(
              toString(res.port)
            )}/deviceManager/devicemanager/feature/login/crossDomainLogin.html?passphrase=${encodeURIComponent(
              res.token
            )}&language=${encodeURIComponent(language)}`;
            window.open(url, '_blank');
          });
      });
    }
  }
}
