import { Component, OnDestroy, OnInit } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-postgre-sql',
  templateUrl: './postgre-sql.component.html',
  styleUrls: ['./postgre-sql.component.less']
})
export class PostgreSqlComponent implements OnInit, OnDestroy {
  activeIndex = DataMap.Resource_Type.PostgreSQLCluster.value;
  dataMap = DataMap;

  destroy$ = new Subject();

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
