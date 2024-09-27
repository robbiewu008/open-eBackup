import {
  Component,
  Input,
  OnInit,
  OnChanges,
  SimpleChanges
} from '@angular/core';
import { DataMap } from 'app/shared/consts';
import { DataMapService, I18NService } from 'app/shared/services';

@Component({
  selector: 'aui-file-indexed',
  template: `
    <i
      [lv-tooltip]="content"
      [lv-icon]="icon"
      [ngClass]="{
        'lv-m-rotate': indexed === dataMap.CopyData_fileIndex.indexing.value
      }"
    ></i>
  `,
  styles: [
    `
      i {
        width: 20px;
        height: 20px;
      }
    `
  ]
})
export class FileIndexedComponent implements OnInit, OnChanges {
  @Input() indexed;

  content;
  icon;
  dataMap = DataMap;

  constructor(
    private dataMapService: DataMapService,
    private i18n: I18NService
  ) {}

  ngOnChanges(changes: SimpleChanges) {
    if (changes.indexed) {
      this.init();
    }
  }

  ngOnInit() {
    this.init();
  }

  init() {
    const config = this.dataMapService.getValueConfig(
      'CopyData_fileIndex',
      this.indexed
    );
    this.icon = config ? config.icon : 'aui-file-unIndexed';
    this.content = this.i18n.get(config ? config.label : '--');
  }
}
