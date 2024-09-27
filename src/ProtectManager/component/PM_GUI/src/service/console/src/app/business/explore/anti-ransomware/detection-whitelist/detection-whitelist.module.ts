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
import { DetectionWhitelistComponent } from './detection-whitelist.component';
import { BaseModule } from 'app/shared';
import { BlockingRuleListModule } from '../blocking-rule-list/blocking-rule-list.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddWhitelistRuleModule } from './add-whitelist-rule/add-whitelist-rule.module';

@NgModule({
  declarations: [DetectionWhitelistComponent],
  imports: [
    CommonModule,
    BaseModule,
    BlockingRuleListModule,
    ProTableModule,
    ProButtonModule,
    AddWhitelistRuleModule
  ],
  exports: [DetectionWhitelistComponent]
})
export class DetectionWhitelistModule {}
