import { Component, OnInit } from '@angular/core';
import { CommonConsts, DataMap, MaskRuleControllerService } from 'app/shared';

@Component({
  selector: 'aui-related-identified-rule',
  templateUrl: './related-identified-rule.component.html',
  styleUrls: ['./related-identified-rule.component.less']
})
export class RelatedIdentifiedRuleComponent implements OnInit {
  rowItem;
  dataMap = DataMap;
  ruleData = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  constructor(private policyManagerApiService: MaskRuleControllerService) {}

  getRuleData() {
    this.ruleData = [];
    this.policyManagerApiService
      .getMaskRuleReferencesUsingGET({
        maskId: this.rowItem.id,
        pageNo: this.pageIndex,
        pageSize: this.pageSize
      })
      .subscribe(res => {
        this.ruleData = res.items;
        this.total = res.total;
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getRuleData();
  }

  ngOnInit() {
    this.getRuleData();
  }
}
