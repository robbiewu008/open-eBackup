import {
  ChangeDetectionStrategy,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewEncapsulation
} from '@angular/core';
import { TypeUtils } from '@iux/live';

@Component({
  selector: 'lv-pro-cols-display',
  templateUrl: './pro-cols-display.component.html',
  styleUrls: ['./pro-cols-display.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  encapsulation: ViewEncapsulation.None
})
export class ProColsDisplayComponent implements OnInit {
  @Input() value: string[];
  @Input() data: { key: string; name: string }[];
  @Input() ignoringColsType: 'hide' | 'disable' = 'hide'; // 非控制列的处理方式
  @Output() valueChange = new EventEmitter<any>();

  constructor() {}

  public typeUtils = TypeUtils;

  ngOnInit() {}

  _displayChange(e) {
    this.valueChange.emit(e);
  }

  _getDisableStatus(data, value, col) {
    let _value = value;
    data.filter(item => {
      if (item.hidden === 'ignoring') {
        _value = _value.filter(v => v !== item.key);
      }
    });

    return _value.length === 1 && col.key === _value[0];
  }
}
