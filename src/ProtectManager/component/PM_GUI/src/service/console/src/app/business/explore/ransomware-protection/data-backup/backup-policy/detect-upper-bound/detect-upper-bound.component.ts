import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DETECT_UPPER_BOUND_POINTER, I18NService } from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-detect-upper-bound',
  templateUrl: './detect-upper-bound.component.html',
  styleUrls: ['./detect-upper-bound.component.less']
})
export class DetectUpperBoundComponent implements OnInit {
  @Input() formGroup: FormGroup;

  defaultPoint = 2;

  points = DETECT_UPPER_BOUND_POINTER.map(item => {
    return assign(item, {
      label: this.i18n.get(item.label)
    });
  });

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {
    if (!this.formGroup?.get('upper_bound')) {
      this.formGroup.addControl(
        'upper_bound',
        new FormControl(this.defaultPoint)
      );
    }
  }

  getUpperBound(upper_bound) {
    if (upper_bound === 1) {
      return 7;
    }
    if (upper_bound === 2) {
      return 6;
    }
    return 5;
  }

  setUpperBound(backup) {
    return backup.ext_parameters?.upper_bound === 5
      ? 3
      : backup.ext_parameters?.upper_bound === 7
      ? 1
      : 2;
  }
}
