import { Component, OnInit } from '@angular/core';
import { CommonConsts, PolicyControllerService } from 'app/shared';

@Component({
  selector: 'aui-related-object',
  templateUrl: './related-object.component.html',
  styleUrls: ['./related-object.component.less']
})
export class RelatedObjectComponent implements OnInit {
  rowItem;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  dbData = [];
  constructor(private policyManagerApiService: PolicyControllerService) {}

  initDb() {
    this.policyManagerApiService
      .getPolicyReferencesUsingGET({
        pageSize: this.pageSize,
        pageNo: this.pageIndex,
        policyId: this.rowItem.id
      })
      .subscribe(res => {
        this.dbData = res.items;
        this.total = res.total;
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.initDb();
  }

  ngOnInit() {
    this.initDb();
  }
}
