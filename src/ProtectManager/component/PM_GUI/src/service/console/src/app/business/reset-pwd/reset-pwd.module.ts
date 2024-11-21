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
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ResetPwdRoutingModule } from './reset-pwd-routing.module';
import { ResetPwdComponent } from './reset-pwd.component';
import { LogoTitleModule } from 'app/shared/components/logo-title/logo-title.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ResetPwdComponent],
  imports: [CommonModule, BaseModule, LogoTitleModule, ResetPwdRoutingModule]
})
export class ResetPwdModule {}
