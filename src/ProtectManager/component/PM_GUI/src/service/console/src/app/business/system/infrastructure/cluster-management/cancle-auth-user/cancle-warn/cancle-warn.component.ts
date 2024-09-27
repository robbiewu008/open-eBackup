import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import { CommonConsts, I18NService } from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { size } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-cancle-warn',
  templateUrl: './cancle-warn.component.html',
  styleUrls: ['./cancle-warn.component.less']
})
export class CancleWarnComponent implements OnInit {
  rowItem;
  userTableData;
  userTableConfig: TableConfig;
  confirmChecked = false;
  confrimLable;
  isChecked$ = new Subject<boolean>();
  @ViewChild('headerTpl', { static: true })
  headerTpl: TemplateRef<any>;

  constructor(private modal: ModalRef, private i18n: I18NService) {}

  ngOnInit() {
    this.initHeader();
    this.initTableConfig();
  }

  initHeader() {
    this.confrimLable = this.i18n.get('system_cancle_auth_warn_label', [
      this.rowItem?.clusterName
    ]);
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  confirmChange(e) {
    this.isChecked$.next(e);
  }

  initTableConfig() {
    this.userTableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'userName',
            name: this.i18n.get('system_local_cluster_username_label')
          },
          {
            key: 'managedUserName',
            name: this.i18n.get('system_managed_cluster_username_label')
          }
        ],
        compareWith: 'userId',
        showLoading: false,
        colDisplayControl: false,
        size: 'small'
      },
      pagination: {
        mode: 'simple',
        pageIndex: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    this.userTableData = {
      data: this.rowItem.cancelData,
      total: size(this.rowItem.cancelData)
    };
  }
}
