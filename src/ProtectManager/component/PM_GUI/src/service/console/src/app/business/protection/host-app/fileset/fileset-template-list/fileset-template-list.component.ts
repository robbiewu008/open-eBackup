import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { FilesetComponent } from '../fileset.component';
import { TemplateListComponent } from './template-list/template-list.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { GlobalService } from 'app/shared';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-fileset-template-list',
  templateUrl: './fileset-template-list.component.html',
  styleUrls: ['./fileset-template-list.component.less']
})
export class FilesetTemplateListComponent implements OnInit, OnDestroy {
  activeIndex: string = 'fileset';

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
