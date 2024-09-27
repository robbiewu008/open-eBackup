import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
@Component({
  selector: 'aui-environment-info',
  templateUrl: './environment-info.component.html',
  styleUrls: ['./environment-info.component.less']
})
export class EnvironmentInfoComponent implements OnInit {
  treeSelection;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(private modal: ModalRef) {}

  ngOnInit() {
    this.updateHeader();
  }

  updateHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
