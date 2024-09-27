import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DataMap, DataMapService } from 'app/shared';
import { includes } from 'lodash';

@Component({
  selector: 'aui-set-quota',
  templateUrl: './set-quota.component.html',
  styleUrls: ['./set-quota.component.less']
})
export class SetQuotaComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  sizeUnitOptions = this.dataMapService
    .toArray('Capacity_Unit')
    .filter(item => {
      item.isLeaf = true;
      return includes(
        [DataMap.Capacity_Unit.gb.value, DataMap.Capacity_Unit.mb.value],
        item.value
      );
    });

  constructor(
    private fb: FormBuilder,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      quota: new FormControl('1'),
      size: new FormControl(''),
      unit: new FormControl(DataMap.Capacity_Unit.gb.value)
    });
  }
}
