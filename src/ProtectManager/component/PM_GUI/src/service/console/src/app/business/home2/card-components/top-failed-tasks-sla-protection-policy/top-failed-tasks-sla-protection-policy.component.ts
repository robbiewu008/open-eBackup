import { Component, Input } from '@angular/core';
import { ReportDataService, I18NService } from 'app/shared';

@Component({
  selector: 'top-failed-tasks-sla-protection-policy',
  templateUrl: './top-failed-tasks-sla-protection-policy.component.html',
  styleUrls: ['./top-failed-tasks-sla-protection-policy.component.less']
})
export class TopFailedTasksSlaProtectionPolicyComponent {
  @Input() cardInfo: any = {};
  data = [];
  constructor(
    private reportDataService: ReportDataService,
    private i18n: I18NService
  ) {}

  ngOnInit(): void {
    this.refreshData();
  }

  refreshData() {
    this.cardInfo.loading = true;
    this.reportDataService
      .QueryProtectTask({
        akLoading: false,
        akOperationTips: false,
        QueryProtectTaskRequestBody: {
          timeRange: 'LAST_THREE_MONTH',
          dataQueryTypeEnum: 'SLA'
        }
      })
      .subscribe(res => {
        let { slaTaskSummary } = res;
        this.data = slaTaskSummary;
        this.cardInfo.loading = false;
      });
  }
}
