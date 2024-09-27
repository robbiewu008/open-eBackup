import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { CAPACITY_UNIT, LicenseApiService, RouterUrl } from 'app/shared';
import { assign, each } from 'lodash';

@Component({
  selector: 'app-license-capacity',
  templateUrl: './license-capacity.component.html',
  styleUrls: ['./license-capacity.component.less']
})
export class LicenseCapacityComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  licenses = [];

  constructor(
    private router: Router,
    private licenseApiService: LicenseApiService
  ) {}

  ngOnInit() {
    this.loadData();
  }

  loadData(loading: boolean = true) {
    this.licenseApiService
      .queryLicenseUsingGET({ akLoading: loading, akDoException: false })
      .subscribe(res => {
        this.licenses = res || [];

        each(this.licenses, item => {
          assign(item, {
            capacity: (item.usedCapacity / item.totalCapacity) * 100
          });
        });
      });
  }

  gotoLicense() {
    this.router.navigateByUrl(RouterUrl.SystemLicense);
  }
}
