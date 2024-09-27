import { AfterViewInit, Component } from '@angular/core';
import { Router } from '@angular/router';
import { DetectReportsService, RouterUrl } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { size } from 'lodash';

@Component({
  selector: 'aui-exceptions-file',
  templateUrl: './exceptions-file.component.html',
  styleUrls: ['./exceptions-file.component.less']
})
export class ExceptionsFileComponent implements AfterViewInit {
  tableData = [];

  total = 0;

  constructor(
    private router: Router,
    private appUtilsService: AppUtilsService,
    private detectReportsService: DetectReportsService
  ) {}

  ngAfterViewInit() {
    this.loadTabelData();
  }

  loadTabelData(loading: boolean = true) {
    this.detectReportsService
      .getExceptionFileSystems({
        akLoading: loading
      })
      .subscribe(res => {
        this.tableData = res.exceptionFileSystemList;
        this.total = size(res.exceptionFileSystemList);
      });
  }

  gotoSnapshot() {
    this.router.navigateByUrl(RouterUrl.ExploreSnapShotData);
  }

  gotoDetectionDetail(item) {
    this.appUtilsService.setCacheValue('resourceName', item.resourceName);
    this.router.navigateByUrl(RouterUrl.ExploreSnapShotData);
  }
}
