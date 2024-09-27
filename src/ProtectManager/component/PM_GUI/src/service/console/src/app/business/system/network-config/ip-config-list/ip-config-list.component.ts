import { Component, OnInit, Injectable } from '@angular/core';
import { MODAL_COMMON, I18NService } from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';

@Component({
  selector: 'aui-ip-config-list',
  templateUrl: './ip-config-list.component.html',
  styleUrls: ['./ip-config-list.component.less']
})
export class IpConfigListComponent implements OnInit {
  data;

  constructor() {}

  ngOnInit() {}
}

@Injectable({
  providedIn: 'root'
})
export class IpConfigListService {
  private ipConfigListComponent = IpConfigListComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService
  ) {}

  create(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.smallWidth,
      lvModalKey: 'ip_segment_list',
      ...{
        lvHeader: this.i18n.get('common_more_label'),
        lvContent: this.ipConfigListComponent,
        lvComponentParams: {
          data
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      }
    });
  }
}
