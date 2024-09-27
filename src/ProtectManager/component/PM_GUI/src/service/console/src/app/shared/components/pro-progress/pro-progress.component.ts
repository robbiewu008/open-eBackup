import {
  ChangeDetectionStrategy,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { I18NService, TypeUtils } from '@iux/live';
import { merge as _merge, isNil as _isNil } from 'lodash';

const DEFAULT_CONFIG = {
  // progress 配置
  value: 0,
  width: 0,
  colors: [],
  status: 'normal',
  showLabel: true,
  label: null,
  extra: null,
  // tooltip 配置
  tooltip: null,
  tooltipTheme: 'light'
};

@Component({
  selector: 'lv-pro-progress',
  templateUrl: './pro-progress.component.html',
  styleUrls: ['./pro-progress.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ProProgressComponent implements OnInit {
  @Input() value;
  @Input() config;

  initConfig;
  public typeUtils = TypeUtils;

  constructor() {}

  isNull() {
    return _isNil(this.value) || this.value === '' || false;
  }

  ngOnInit() {
    this.initConfig = _merge({}, DEFAULT_CONFIG, this.config);
    this.initConfig.showLabel = this.initConfig.label === false ? false : true;
  }
}
