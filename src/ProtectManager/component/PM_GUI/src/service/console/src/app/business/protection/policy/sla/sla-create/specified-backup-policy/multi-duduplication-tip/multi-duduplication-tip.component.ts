import { Component, Input, OnInit } from '@angular/core';
import { FormGroup } from '@angular/forms';
import { CommonConsts, CookieService, MultiCluster } from 'app/shared';

@Component({
  selector: 'aui-multi-duduplication-tip',
  templateUrl: './multi-duduplication-tip.component.html',
  styleUrls: ['./multi-duduplication-tip.component.less']
})
export class MultiDuduplicationTipComponent implements OnInit {
  multiCluster = MultiCluster;
  @Input() formGroup: FormGroup;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  constructor(private cookieService: CookieService) {}

  ngOnInit(): void {}
}
