import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { GlobalService } from 'app/shared';
import { Subscription } from 'rxjs';

@Component({
  selector: 'aui-custom-modal-operate',
  templateUrl: './custom-modal-operate.component.html',
  styleUrls: ['./custom-modal-operate.component.less']
})
export class CustomModalOperateComponent implements OnInit, OnDestroy {
  header;
  @Input() item;
  subscription$: Subscription;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy() {
    this.subscription$.unsubscribe();
  }

  ngOnInit() {
    this.init();
    this.autoRefreshOpt();
  }

  init() {
    this.header =
      this.item.name && this.item.ip
        ? `${this.item.name}(${this.item.ip})`
        : `${this.item.name}`;
  }

  optCallback = data => {
    return this.item.optItems || [];
  };

  autoRefreshOpt() {
    this.subscription$ = this.globalService
      .getState('autoReshResource')
      .subscribe(res => {
        if (this.item.optItemsFn) {
          this.optCallback = data => {
            return this.item.optItemsFn(res) || [];
          };
        }
      });
  }
}
