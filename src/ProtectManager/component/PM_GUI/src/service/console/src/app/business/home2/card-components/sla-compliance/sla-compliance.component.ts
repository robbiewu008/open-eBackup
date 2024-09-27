import { Component, ElementRef, OnInit, Input } from '@angular/core';
import { ApiMultiClustersService, ProjectedObjectApiService } from 'app/shared';
import { CookieService, I18NService } from 'app/shared/services';

@Component({
  selector: 'sla-compliance',
  templateUrl: './sla-compliance.component.html',
  styleUrls: ['./sla-compliance.component.less']
})
export class SlaComplianceComponent implements OnInit {
  @Input() cardInfo: any = {};
  isAllCluster = true;
  inCompliance = 0;
  outOfCompliance = 0;
  percent: number | string = 100;
  constructor(
    private i18n: I18NService,
    public cookieService: CookieService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.getSlaCompliance();
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  getSlaCompliance() {
    this.cardInfo.loading = true;
    if (this.isAllCluster) {
      this.multiClustersServiceApi
        .getMultiClusterSla({ akLoading: false })
        .subscribe(res => {
          this.inCompliance = res.inCompliance;
          this.outOfCompliance = res.outOfCompliance;
          this.percent = (
            (res.inCompliance / (res.inCompliance + res.outOfCompliance)) *
              100 || 0
          ).toFixed(0);
          this.cardInfo.loading = false;
        });
    } else {
      this.projectedObjectApiService
        .queryProtectionCompliance({ akLoading: false })
        .subscribe((res: any) => {
          this.inCompliance = res.in_compliance;
          this.outOfCompliance = res.out_of_compliance;
          this.percent = (
            (res.in_compliance / (res.in_compliance + res.out_of_compliance)) *
              100 || 0
          ).toFixed(0);
          this.cardInfo.loading = false;
        });
    }
  }
}
