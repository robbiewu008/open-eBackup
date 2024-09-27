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
import { Component, Input, OnInit } from '@angular/core';
import { I18NService } from '../../services';
@Component({
  selector: 'aui-sla-compliance-tooltip',
  templateUrl: './sla-compliance-tooltip.component.html',
  styleUrls: ['./sla-compliance-tooltip.component.less']
})
export class SlaComplianceTooltipComponent implements OnInit {
  @Input('lvTooltipPosition') lvTooltipPosition = 'top';
  @Input('lvTooltipClassName') lvTooltipClassName = 'sla-compliance-tooltip';
  @Input('lvTooltipTheme') lvTooltipTheme = 'light';
  @Input('margin') margin = false;
  slaComplianceTooltipInfo = this.i18n.get(
    'protection_sla_compliance_help_label'
  );
  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
