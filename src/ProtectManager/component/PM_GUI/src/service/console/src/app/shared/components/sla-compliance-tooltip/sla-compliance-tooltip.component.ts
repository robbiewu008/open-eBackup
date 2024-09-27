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
