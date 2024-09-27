import { Component, OnDestroy, OnInit } from '@angular/core';
import { GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-opengauss',
  templateUrl: './opengauss.component.html',
  styleUrls: ['./opengauss.component.less']
})
export class OpengaussComponent implements OnInit, OnDestroy {
  activeIndex = 'cluster';

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
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
