import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { DatabaseTemplateComponent } from '../database-template/database-template.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { RegisterTenantComponent } from './register-tenant/register-tenant.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';
@Component({
  selector: 'aui-ocean-base',
  templateUrl: './ocean-base.component.html',
  styleUrls: ['./ocean-base.component.less']
})
export class OceanBaseComponent implements OnInit, OnDestroy {
  activeIndex = 0;
  clusterConfig;
  tenantConfig;

  destroy$ = new Subject();

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      activeIndex: DataMap.Resource_Type.OceanBaseCluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'version',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'authorizedUser',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterClusterComponent
    };
    this.tenantConfig = {
      activeIndex: DataMap.Resource_Type.OceanBaseTenant.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'environmentName',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterTenantComponent
    };

    this.showGuideTab();
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showGuideTab();
      });
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }
}
