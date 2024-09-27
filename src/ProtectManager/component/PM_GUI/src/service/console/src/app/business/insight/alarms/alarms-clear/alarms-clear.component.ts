import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import { I18NService } from 'app/shared';
import { map } from 'lodash';

@Component({
  selector: 'aui-alarms-clear',
  templateUrl: './alarms-clear.component.html',
  styleUrls: ['./alarms-clear.component.less']
})
export class AlarmsClearComponent implements OnInit {
  selectionData;
  isAlarm;
  isCyberEngine;
  pageSize = 10;
  pageIndex = 0;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(public i18n: I18NService, private modal: ModalRef) {}

  ngOnInit() {
    this.updateHeader();
    this.isCyberEngine && this.updateCyberEngineData();
  }

  updateCyberEngineData() {
    this.selectionData = map(this.selectionData, item => ({
      ...item,
      name: item.alarmName,
      objctType: item.sourceType
    }));
  }

  updateHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
