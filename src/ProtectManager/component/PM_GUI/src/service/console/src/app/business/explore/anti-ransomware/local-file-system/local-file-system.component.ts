import { Component } from '@angular/core';
import { DataMap, SupportLicense } from 'app/shared';

@Component({
  selector: 'aui-local-file-system',
  templateUrl: './local-file-system.component.html',
  styleUrls: ['./local-file-system.component.less']
})
export class LocalFileSystemComponent {
  activeIndex = 'nas';
  dataMap = DataMap;
  supportLicense = SupportLicense;

  ngOnInit() {
    if (SupportLicense.isFile) {
      this.activeIndex = 'nas';
    } else {
      this.activeIndex = 'san';
    }
  }
}
