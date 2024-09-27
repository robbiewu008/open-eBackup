import { Component, OnInit } from '@angular/core';
import { Subject } from 'rxjs';
import { I18NService } from '@iux/live';
import { find } from 'lodash';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-destroy-live-mount',
  templateUrl: './destroy-live-mount.component.html',
  styleUrls: ['./destroy-live-mount.component.less']
})
export class DestroyLiveMountComponent implements OnInit {
  items;
  status;
  reserveCopy = true;
  forceDelete = false;
  forceDeleteShow = true;
  isChecked$ = new Subject<boolean>();

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.forceDeleteShow =
      this.items &&
      !find(this.items, {
        status: DataMap.LiveMount_Status.available.value
      });
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }
}
