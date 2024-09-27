import { Component, OnInit } from '@angular/core';
import { BackupClustersHaApiService } from 'app/shared';
import { Observable, Observer, defer } from 'rxjs';

@Component({
  selector: 'aui-delete-ha',
  templateUrl: './delete-ha.component.html',
  styleUrls: ['./delete-ha.component.less']
})
export class DeleteHaComponent implements OnInit {
  drawData: any;
  isShow = false;
  constructor(public haApiService: BackupClustersHaApiService) {}

  ngOnInit(): void {
    if (this.drawData.status === 28) {
      this.isShow = true;
    } else {
      this.isShow = false;
    }
  }

  deleteHaInfo(param): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.haApiService.removeHaConfig({ isForce: param }).subscribe({
        next: () => {
          observer.next();
          observer.complete();
        },
        error: error => {
          observer.error(error);
          observer.complete();
        }
      });
    });
  }
}
