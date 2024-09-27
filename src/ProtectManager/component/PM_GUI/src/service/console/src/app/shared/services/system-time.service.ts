import { Injectable } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import { SystemApiService } from '../api/services';

@Injectable({
  providedIn: 'root'
})
export class SystemTimeService {
  constructor(private systemApiService: SystemApiService) {}

  getSystemTime(load = true, cluster?): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.systemApiService
        .getSystemTimeUsingGET({
          akLoading: load,
          akDoException: false,
          memberEsn: cluster?.memberEsn
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
