import { Component, OnInit } from '@angular/core';
import { MenuItem } from '@iux/live';
import { ApiMultiClustersService, ResourceService } from 'app/shared';
import {
  DataMap,
  ResourceType,
  RouterUrl,
  SupportLicense
} from 'app/shared/consts';
import {
  CookieService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { ResourceCatalogsService } from 'app/shared/services/resource-catalogs.service';
import { assign, each, includes, isArray } from 'lodash';

@Component({
  selector: 'aui-resource-display-badge',
  templateUrl: './resource-display-badge.component.html',
  styleUrls: ['./resource-display-badge.component.css']
})
export class ResourceDisplayBadgeComponent implements OnInit {
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
    private dataMapService: DataMapService,
    public cookieService: CookieService,
    private resourceCatalogsService: ResourceCatalogsService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.initData();
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
          assign(this.protection, {
            protected: res.protectedCount,
            unprotected: res.unprotectedCount
          });
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
          assign(this.protection, {
            protected: protectedCount,
            unprotected: unprotectedCount
          });
        });
    }
  }

  getResourceTypeByKey(resourceKey) {
    const resourceArr = [];
    each(this.appUtilsService.getApplicationConfig()[resourceKey], item => {
      if (isArray(item.key)) {
        resourceArr.push(...item.key);
      } else {
        resourceArr.push(item.key);
      }
    });
    return resourceArr;
  }

  initData() {
    if (this.isCloudBackup) {
      this.resourceDetailUrl = [RouterUrl.ProtectionLocalFileSystem];
    }
    if (this.isHyperdetect) {
      this.resourceDetailUrl = [RouterUrl.ProtectionLocalResource];
    }
    this.resourceCatalogsService.getResourceCatalog().subscribe(items => {
      this.options = [
        {
          id: '1',
          label: this.i18n.get('common_all_resource_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_all_resource_label');
            this.resourceDetailUrl = ['/protection/summary'];
            this.resourceType = 'All';
            this.getProtectionStatus();
          }
        },
        {
          id: '3',
          label: this.i18n.get('common_database_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_database_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionHostAppOracle];
            this.resourceType = ResourceType.DATABASE;
            this.getProtectionStatus(this.getResourceTypeByKey('database'));
          }
        },
        {
          id: '4',
          label: this.i18n.get('common_bigdata_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_bigdata_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionHostAppMongoDB];
            this.resourceType = ResourceType.BIG_DATA;
            this.getProtectionStatus(this.getResourceTypeByKey('bigData'));
          }
        },
        {
          id: '5',
          label: this.i18n.get('common_virtualization_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_virtualization_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionVirtualizationVmware];
            this.resourceType = 'Virtualization';
            this.getProtectionStatus(
              this.getResourceTypeByKey('virtualization')
            );
          }
        },

        {
          id: '6',
          label: this.i18n.get('common_container_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_container_label');
            this.resourceDetailUrl = [
              RouterUrl.ProtectionVirtualizationKubernetes
            ];
            this.resourceType = 'Container';
            this.getProtectionStatus(this.getResourceTypeByKey('container'));
          }
        },
        {
          id: '7',
          label: this.i18n.get('common_huawei_clouds_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_huawei_clouds_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionCloudHuaweiStack];
            this.resourceType = 'Cloud';
            this.getProtectionStatus(this.getResourceTypeByKey('cloud'));
          }
        },
        {
          id: '8',
          label: this.i18n.get('common_application_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_application_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionActiveDirectory];
            this.resourceType = 'Application';
            this.getProtectionStatus(this.getResourceTypeByKey('application'));
          }
        },
        {
          id: '9',
          label: this.i18n.get('common_file_systems_label'),
          onClick: data => {
            this.resourceText = this.i18n.get('common_file_systems_label');
            this.resourceDetailUrl = [RouterUrl.ProtectionStorageDeviceInfo];
            this.resourceType = 'FileSystem';
            if (
              this.appUtilsService.isDistributed ||
              this.appUtilsService.isDecouple
            ) {
              this.getProtectionStatus([
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.ObjectStorage.value,
                DataMap.Resource_Type.ObjectSet.value
              ]);
            } else {
              this.getProtectionStatus(
                this.getResourceTypeByKey('fileService')
              );
            }
          }
        }
      ];
    });
  }
}
