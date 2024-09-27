import {
  Component,
  OnInit,
  Input,
  Output,
  EventEmitter,
  OnDestroy
} from '@angular/core';
import { Subscription, Subject, timer } from 'rxjs';
import {
  DataMap,
  SystemApiService,
  CommonConsts,
  I18NService,
  Modify_Network_View_Type
} from 'app/shared';
import { Router } from '@angular/router';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-modify-network-process',
  templateUrl: './modify-network-process.component.html',
  styleUrls: ['./modify-network-process.component.less']
})
export class ModifyNetworkProcessComponent implements OnInit, OnDestroy {
  backupTimeSub$: Subscription;
  archiveTimeSub$: Subscription;
  backupDestroy$ = new Subject();
  archiveDestroy$ = new Subject();
  backupResult = {};
  archiveResult = {};
  initStatus = DataMap.System_Init_Status;
  type = Modify_Network_View_Type;

  @Output() onResetBackupChange = new EventEmitter<any>();
  @Output() onResetArchiveChange = new EventEmitter<any>();
  @Input() viewType;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private systemApiService: SystemApiService
  ) {}

  ngOnDestroy() {
    if (this.viewType === this.type.BackupView) {
      this.backupDestroy$.next(true);
      this.backupDestroy$.complete();
    } else {
      this.archiveDestroy$.next(true);
      this.archiveDestroy$.complete();
    }
  }

  ngOnInit() {}

  getModifyBackupStatus() {}

  getModifyArchiveStatus() {}

  onResetBackup() {
    this.onResetBackupChange.emit();
  }

  onResetArchive() {
    this.onResetArchiveChange.emit();
  }
}
