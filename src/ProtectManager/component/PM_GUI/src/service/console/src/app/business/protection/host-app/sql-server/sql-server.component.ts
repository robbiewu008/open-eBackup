import { Component, OnDestroy, OnInit } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-sql-server',
  templateUrl: './sql-server.component.html',
  styleUrls: ['./sql-server.component.less']
})
export class SQLServerComponent implements OnInit, OnDestroy {
  activeIndex = DataMap.Resource_Type.SQLServerCluster.value;
  clusterConfig;
  instanceConfig;
  groupConfig;
  databaseConfig;
  subType = DataMap.Resource_Type.SQLServer.value;

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.SQLServerCluster.value,
      tableCols: ['uuid', 'name', 'linkStatus', 'authorizedUser', 'operation'],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.SQLServerInstance.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'nodeIpAddress',
        'version',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup'
      ]
    };

    this.groupConfig = {
      id: DataMap.Resource_Type.SQLServerGroup.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup'
      ]
    };

    this.databaseConfig = {
      id: DataMap.Resource_Type.SQLServerDatabase.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'ownedInstance',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup'
      ]
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
