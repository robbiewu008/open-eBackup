import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { GlobalService, ResourceType } from 'app/shared';
import { DatabaseListComponent } from './database-list/database-list.component';
import { HostClusterListComponent } from './host-cluster-list/host-cluster-list.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { takeUntil } from 'rxjs/operators';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-oracle',
  templateUrl: './oracle.component.html',
  styleUrls: ['./oracle.component.less']
})
export class OracleComponent implements OnInit, OnDestroy {
  resourceType = ResourceType;
  activeIndex = this.resourceType.HOST;

  destroy$ = new Subject();

  @ViewChild(DatabaseListComponent, { static: false })
  databaseListComponent: DatabaseListComponent;

  @ViewChild(HostClusterListComponent, { static: false })
  hostClusterListComponent: HostClusterListComponent;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
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
