import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';
import { CookieService, I18NService, RoleType } from 'app/shared';

@Component({
  selector: 'report-list-item',
  templateUrl: './list-item.component.html',
  styleUrls: ['./list-item.component.less']
})
export class ListItemComponent implements OnInit {
  @Input() item: any;
  @Input() showProgress: boolean = false;
  @Input() progressValue: number = 0;

  @Output() selectedChange = new EventEmitter<object | string>();
  @Output() remove = new EventEmitter<void>();
  @Output() view = new EventEmitter<void>();

  _checked: boolean;
  contentItems = [];
  roleType = RoleType;
  _isEn = this.i18n.isEn;

  constructor(public i18n: I18NService, public cookieService: CookieService) {}

  get checked() {
    return this._checked;
  }

  set checked(val) {
    this.selectedChange.emit(val ? this.item : this.item.uuid);
    this._checked = val;
  }

  ngOnInit() {
    this.initContenItems();
  }

  initContenItems() {
    this.contentItems = [
      {
        label: 'protection_storage_device_label',
        value: `${this.item.storageName} （${this.item.storageEndpoint}）`
      },
      {
        label: 'common_tenant_label',
        value: this.item.tenantName
      },
      {
        label: 'common_file_system_label',
        value: this.item.fileSystemName
      },
      {
        label: 'insight_report_scope_label',
        value: `${this.item.inputDetectStartTime} - ${this.item.inputDetectEndTime}`
      }
    ];
  }

  removeItem() {
    if (this.showProgress) {
      return;
    }
    this.remove.emit();
  }

  viewItem() {
    if (this.showProgress) {
      return;
    }
    this.view.emit();
  }
}
