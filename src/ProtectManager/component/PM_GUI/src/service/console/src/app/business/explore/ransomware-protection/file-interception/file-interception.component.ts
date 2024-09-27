import { Component, OnInit } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  FsFileExtensionFilterManagementService
} from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-file-interception',
  templateUrl: './file-interception.component.html',
  styleUrls: ['./file-interception.component.less']
})
export class FileInterceptionComponent implements OnInit {
  activeIndex = 'fileSystem';
  totalFileSystem = 0;
  protectedFileSystem = 0;
  totalRule = 0;

  constructor(
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngOnInit() {
    this.getFilesystem();
    this.getFilesystem(true);
    this.getFilterRule();
  }

  refreshFileSystem() {
    this.getFilesystem(false, false);
    this.getFilesystem(true, false);
  }

  refreshRule() {
    this.getFilterRule(false);
  }

  getFilesystem(protectedFlag = false, mask = true) {
    const params = {
      pageNum: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    if (protectedFlag) {
      assign(params, {
        configStatus: [DataMap.File_Extension_Status.enable.value]
      });
    }
    this.fsFileExtensionFilterManagementService
      .getFsFileBlockConfigUsingGET(params)
      .subscribe(res => {
        if (protectedFlag) {
          this.protectedFileSystem = res.totalCount;
        } else {
          this.totalFileSystem = res.totalCount;
        }
      });
  }

  getFilterRule(mask = true) {
    const params: any = {
      pageNum: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE
    };
    this.fsFileExtensionFilterManagementService
      .getFsExtensionFilterUsingGET({ ...params, akLoading: mask })
      .subscribe(res => {
        this.totalRule = res.totalCount;
      });
  }
}
