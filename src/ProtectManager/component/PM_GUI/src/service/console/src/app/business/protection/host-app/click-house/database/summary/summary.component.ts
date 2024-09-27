import { Component, OnInit } from '@angular/core';
import {
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { assign, toNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-database-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.css']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;

  constructor(
    private i18n: I18NService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = assign(data, {
      link_status: toNumber(data.linkStatus)
    });
    this.type = DataMap.Resource_Type.ClickHouseDatabase.value;
    this.getDetail(data.environment_uuid || data?.environment?.uuid).subscribe(
      item => {
        this.source.link_status = (item as any).linkStatus;
        this.globalService.emitStore({
          action: 'autoReshResource',
          state: this.source
        });
      }
    );
  }

  getDetail(uuid) {
    return new Observable<object>((observer: Observer<object>) => {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: uuid
        })
        .subscribe(
          item => {
            observer.next(item);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  getData() {}
}
