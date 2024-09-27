import {
  Component,
  EventEmitter,
  OnDestroy,
  OnInit,
  Output
} from '@angular/core';
import { CommonConsts, DataMap, SftpManagerApiService } from 'app/shared';
import { includes } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-process-loading',
  templateUrl: './process-loading.component.html',
  styleUrls: ['./process-loading.component.less']
})
export class ProcessLoadingComponent implements OnInit, OnDestroy {
  timeSub$: Subscription;
  destroy$ = new Subject();
  result = {} as any;
  dataMap = DataMap;
  status = DataMap.Standard_Service_Status.running.value;

  @Output() onComplete = new EventEmitter<any>();
  @Output() onTetry = new EventEmitter<any>();

  constructor(public sftpManagerApiService: SftpManagerApiService) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {}

  getStatus() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.sftpManagerApiService.queryServiceUsingGET({
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.status = res.status;
        if (includes([0, 1], res.status)) {
          this.timeSub$.unsubscribe();
        }
        this.onComplete.emit(includes([0, 1], res.status));
      });
  }
}
