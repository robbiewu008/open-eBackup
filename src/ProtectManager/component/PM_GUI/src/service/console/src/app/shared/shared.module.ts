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
  NgModule,
  ModuleWithProviders,
  APP_INITIALIZER,
  Optional,
  SkipSelf
} from '@angular/core';
import { SharedConfig } from './shared.config';
import { I18NService } from './services';
import { WhiteboxService } from './services/whitebox.service';
import { HttpInterceptorProviders } from './httpInterceptors';

@NgModule({
  imports: [],
  declarations: [],
  exports: []
})
export class SharedModule {
  constructor(@Optional() @SkipSelf() parentModule: SharedModule) {
    if (parentModule) {
      // shareModule 只能注入一次
      throw new Error(
        'SharedModule is already loaded. Import it in the AppModule only'
      );
    }
  }

  static forRoot(): ModuleWithProviders<SharedModule> {
    return {
      ngModule: SharedModule,
      providers: [
        HttpInterceptorProviders,
        {
          provide: APP_INITIALIZER,
          useFactory: SharedConfig.config,
          deps: [I18NService, WhiteboxService],
          multi: true
        }
      ]
    };
  }
}
