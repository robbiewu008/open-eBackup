import { Component, OnDestroy, OnInit } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { RegisterInstanceComponent } from './register-instance/register-instance.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { RegisterDistributedInstanceComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/register-distributed-instance/register-distributed-instance.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';
@Component({
  selector: 'aui-tdsql',
  templateUrl: './tdsql.component.html',
  styleUrls: ['./tdsql.component.less']
})
export class TdsqlComponent implements OnInit, OnDestroy {
  activeIndex = 0;
  instanceConfig;
  databaseConfig;
  distributedInstanceConfig;

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    const commonConfig = {
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'environmentName',
        'mysql_version',
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
        'connectivityTest',
        'modify',
        'deleteResource'
      ]
    };
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.tdsqlCluster.value,
      tableCols: ['uuid', 'name', 'linkStatus', 'authorizedUser', 'operation'],
      tableOpts: [
        'register',
        'resourceAuth',
        'resourceReclaiming',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterClusterComponent
    };

    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.tdsqlInstance.value,
      ...commonConfig,
      registerComponent: RegisterInstanceComponent
    };

    this.distributedInstanceConfig = {
      activeIndex: DataMap.Resource_Type.tdsqlDistributedInstance.value,
      ...commonConfig,
      registerComponent: RegisterDistributedInstanceComponent
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
