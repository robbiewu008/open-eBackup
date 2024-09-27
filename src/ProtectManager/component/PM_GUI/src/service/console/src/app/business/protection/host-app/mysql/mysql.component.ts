import { Component, OnDestroy, OnInit } from '@angular/core';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { GlobalService } from 'app/shared';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-mysql',
  templateUrl: './mysql.component.html',
  styleUrls: ['./mysql.component.less']
})
export class MysqlComponent implements OnInit, OnDestroy {
  activeIndex = 0;

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
