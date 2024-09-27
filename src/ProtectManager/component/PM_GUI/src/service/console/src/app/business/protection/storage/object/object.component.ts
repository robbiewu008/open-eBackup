import { Component, OnDestroy, OnInit } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { RegisterObjectComponent } from './object-service/register-object/register-object.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-object',
  templateUrl: './object.component.html',
  styleUrls: ['./object.component.less']
})
export class ObjectComponent implements OnInit, OnDestroy {
  activeIndex: string = 'object-storage';
  setConfig;

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.setConfig = {
      activeIndex: DataMap.Resource_Type.ObjectSet.value,
      tableCols: [
        'uuid',
        'name',
        'environmentName',
        'storageType',
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
        'deleteResource',
        'modify'
      ],
      registerComponent: RegisterObjectComponent
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
