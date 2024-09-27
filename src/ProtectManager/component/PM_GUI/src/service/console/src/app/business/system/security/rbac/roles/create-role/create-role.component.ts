import { Component, Input, OnInit } from '@angular/core';
import {
  ApiService,
  I18NService,
  RoleApiService,
  UsersApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { FormGroup } from '@angular/forms';
import { ProButton } from 'app/shared/components/pro-button/interface';

@Component({
  selector: 'aui-create-role',
  templateUrl: './create-role.component.html',
  styleUrls: ['./create-role.component.less']
})
export class CreateRoleComponent implements OnInit {
  @Input() openPage;
  @Input() data;
  rowData;
  isModify = false;
  optsConfig;
  formGroup = new FormGroup({});
  disableButton;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public usersApiService: UsersApiService,
    public roleApiService: RoleApiService,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.rowData = this.data.rowData;
    this.isModify = this.data.isModify;
    this.disableButton = !this.data.rowData;
    this.formGroup.statusChanges.subscribe(res => {
      this.disableButton = res !== 'VALID';
    });
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'cancel',
        label: this.i18n.get('common_cancel_label'),
        onClick: () => {
          this.back();
        }
      },
      {
        id: 'ok',
        type: 'primary',
        label: this.i18n.get('common_ok_label'),
        onClick: () => {
          this.onOK();
        }
      }
    ];
    this.optsConfig = [...opts];
  }

  back() {
    this.openPage.emit();
  }

  onOK() {
    const request = this.formGroup.getRawValue();
    if (this.isModify) {
      this.roleApiService
        .updateRole({
          id: this.rowData.roleId,
          request: request
        })
        .subscribe(() => this.openPage.emit());
    } else {
      this.roleApiService
        .createRole({
          request: request
        })
        .subscribe(() => this.openPage.emit());
    }
  }
}
