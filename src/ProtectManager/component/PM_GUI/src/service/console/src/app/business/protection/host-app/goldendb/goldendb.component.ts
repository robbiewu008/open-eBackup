import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { InstanceDatabaseComponent } from '../gaussdb-dws/instance-database/instance-database.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-goldendb',
  templateUrl: './goldendb.component.html',
  styleUrls: ['./goldendb.component.less']
})
export class GoldendbComponent implements OnInit, OnDestroy {
  activeIndex = DataMap.Resource_Type.goldendbCluter.value;
  clusterConfig;
  instanceConfig;
  subType = DataMap.Resource_Type.goldendb.value;

  destroy$ = new Subject();

  @ViewChild(InstanceDatabaseComponent, { static: false })
  instanceDatabaseComponent: InstanceDatabaseComponent;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.goldendbCluter.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'nodeIpAddress',
        'version',
        'authorizedUser',
        'operation'
      ],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.goldendbInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'nodeIpAddress',
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
        'manualBackup',
        'deleteResource'
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

  onChange() {
    this.instanceDatabaseComponent.ngOnInit();
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }
}
