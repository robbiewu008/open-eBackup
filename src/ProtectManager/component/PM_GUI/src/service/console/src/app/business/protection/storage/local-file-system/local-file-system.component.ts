import { I18NService } from 'app/shared';
import { Component, OnInit, ViewChild } from '@angular/core';
import { DoradoFileSystemComponent } from '../dorado-file-system/dorado-file-system.component';

@Component({
  selector: 'aui-local-file-system',
  templateUrl: './local-file-system.component.html',
  styleUrls: ['./local-file-system.component.less']
})
export class LocalFileSystemComponent implements OnInit {
  header = this.i18n.get('common_local_file_systems_label');

  @ViewChild(DoradoFileSystemComponent, { static: false })
  DoradoFileSystemComponent: DoradoFileSystemComponent;

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
