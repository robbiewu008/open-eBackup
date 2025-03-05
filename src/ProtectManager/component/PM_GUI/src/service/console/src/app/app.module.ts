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
import { CdkConnectedOverlay } from '@angular/cdk/overlay';
import { HashLocationStrategy, LocationStrategy } from '@angular/common';
import { HttpClientModule } from '@angular/common/http';
import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { CoreModule as LIVECoreModule } from '@iux/live';
import { SharedModule } from 'app/shared/shared.module';
import { AppRouterModule } from './app-router.module';
import { AppComponent } from './app.component';
import { LogoTitleModule } from './shared/components/logo-title/logo-title.module';
import { ApiModule } from './shared/api/api.module';
import { BaseModule } from './shared/base.module';
import { SimpleMenuModule } from './shared/components/simple-menu/simple-menu.module';
import { ModifyPasswordModule } from './shared/components/user-manager/modify-password.module';
import { GlobalClustersFilterModule } from './shared/components/global-clusters-filter/global-clusters-filter.module';
import { ExportQueryResultsModule } from './shared/components/export-query-results/export-query-results.module';
import { UserGuideModule } from './business/user-guide/user-guide.module';
import { ConfigProviderModule } from '@iux/live';

@NgModule({
  declarations: [AppComponent],
  imports: [
    BrowserModule,
    BrowserAnimationsModule,
    HttpClientModule,
    LIVECoreModule.forRoot(),
    SharedModule.forRoot(),
    BaseModule,
    AppRouterModule,
    LogoTitleModule,
    SimpleMenuModule,
    ModifyPasswordModule,
    GlobalClustersFilterModule,
    ExportQueryResultsModule,
    UserGuideModule,
    ConfigProviderModule,
    ApiModule.forRoot({ rootUrl: '/console/rest' }),
    CdkConnectedOverlay
  ],
  providers: [{ provide: LocationStrategy, useClass: HashLocationStrategy }],
  bootstrap: [AppComponent]
})
export class AppModule {}
