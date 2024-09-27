import { Component, OnInit } from '@angular/core';
import { SupportLicense } from 'app/shared';

@Component({
  selector: 'aui-detection-setting',
  templateUrl: './detection-setting.component.html',
  styleUrls: ['./detection-setting.component.less']
})
export class DetectionSettingComponent implements OnInit {
  activeIndex = 'setting';
  supportLicense = SupportLicense;

  constructor() {}

  ngOnInit() {
    if (SupportLicense.isFile) {
      this.activeIndex = 'setting';
    } else {
      this.activeIndex = 'rule';
    }
  }
}
