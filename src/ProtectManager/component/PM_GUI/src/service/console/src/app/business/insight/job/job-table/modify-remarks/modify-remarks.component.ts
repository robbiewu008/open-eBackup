import { Component, OnInit } from '@angular/core';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { BaseUtilService, JobAPIService } from 'app/shared';
import { JobBo } from 'app/shared/api/models';
import { get } from 'lodash';

@Component({
  selector: 'aui-modify-remarks',
  templateUrl: './modify-remarks.component.html',
  styleUrls: ['./modify-remarks.component.css']
})
export class ModifyRemarksComponent implements OnInit {
  public readonly MAX_LENGTH = 64;
  formGroup: FormGroup;
  row: JobBo;

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private jobApiService?: JobAPIService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    const tag = get(JSON.parse(this.row.extendStr), 'tag', void 0);
    this.formGroup = this.fb.group({
      remarks: new FormControl(tag ?? '', {
        validators: [this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)],
        updateOn: 'change'
      })
    });
  }

  onOK() {
    return this.jobApiService.setJobTagUsingPUT({
      jobId: this.row.jobId,
      tag: this.formGroup.value.remarks
    });
  }
}
