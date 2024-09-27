import { Component, OnInit, Input } from '@angular/core';
import { MenuItem } from '@iux/live';
import { ApiMultiClustersService, ResourceService } from 'app/shared';
import { DataMap, SupportLicense } from 'app/shared/consts';
import { CookieService, I18NService } from 'app/shared/services';
import { assign, each, includes } from 'lodash';
@Component({
  selector: 'protection-situation',
  templateUrl: './protection-situation.component.html',
  styleUrls: ['./protection-situation.component.less']
})
export class ProtectionSituationComponent implements OnInit {
  @Input() cardInfo: any = {};
  options: MenuItem[];
  resourceDetailUrl = ['/protection/summary'];
  resourceText = this.i18n.get('common_all_resource_label');
  protection = {
    protected: 0,
    unprotected: 0
  };
  resourceType = 'All';
  isMultiCluster = true;
  isHyperdetect = includes(
    [DataMap.Deploy_Type.hyperdetect.value],
    this.i18n.get('deploy_type')
  );
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(
    private i18n: I18NService,
    private resourceApiService: ResourceService,
    public cookieService: CookieService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.cardInfo.loading = true;
    this.getProtectionStatus();
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  getProtectionStatus(subType?) {
    if (this.isMultiCluster) {
      this.multiClustersServiceApi
        .getMultiClusterResources({
          akLoading: false,
          resourceType: this.resourceType
        })
        .subscribe(res => {
          this.protection = {
            protected: res.protectedCount,
            unprotected: res.unprotectedCount
          };
          this.cardInfo.loading = false;
        });
    } else {
      const params = { akLoading: false };
      if (subType) {
        assign(params, {
          subType
        });
      }
      this.resourceApiService
        .summaryProtectionResourceV1ResourceProtectionSummaryGet(
          includes(
            [
              DataMap.Deploy_Type.cloudbackup.value,
              DataMap.Deploy_Type.cloudbackup2.value
            ],
            this.i18n.get('deploy_type')
          )
            ? assign(params, {
                subType: [DataMap.Resource_Type.LocalFileSystem.value]
              })
            : this.isHyperdetect
            ? SupportLicense.isBoth
              ? assign(params, {
                  subType: [
                    DataMap.Resource_Type.LocalFileSystem.value,
                    DataMap.Resource_Type.LocalLun.value
                  ]
                })
              : SupportLicense.isFile
              ? assign(params, {
                  subType: [DataMap.Resource_Type.LocalFileSystem.value]
                })
              : assign(params, {
                  subType: [DataMap.Resource_Type.LocalLun.value]
                })
            : params
        )
        .subscribe(res => {
          let protectedCount = 0;
          let unprotectedCount = 0;
          each(res.summary, item => {
            protectedCount += item.protected_count;
            unprotectedCount += item.unprotected_count;
          });
          this.protection = {
            protected: protectedCount,
            unprotected: unprotectedCount
          };
          this.cardInfo.loading = false;
        });
    }
  }
}
