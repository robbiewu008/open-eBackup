import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { ExportFilesService } from '../export-files/export-files.component';
import { assign } from 'lodash';

@Component({
  selector: 'aui-download-flr-files',
  templateUrl: './download-flr-files.component.html',
  styleUrls: ['./download-flr-files.component.less']
})
export class DownloadFlrFilesComponent implements OnInit {
  constructor(private exportFilesService: ExportFilesService) {}

  ngOnInit() {}

  getRequestId(paths, copyId, memberEsn?, tip?) {
    const params = { copyId, paths };
    if (memberEsn) {
      params['memberEsn'] = memberEsn;
    }
    const data = { params, type: DataMap.Export_Query_Type.copy.value };
    if (tip) {
      assign(data, { tip });
    }
    this.exportFilesService.create({ data });
  }
}
