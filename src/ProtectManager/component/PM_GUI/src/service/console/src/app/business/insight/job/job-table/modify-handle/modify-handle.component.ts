import { Component, OnInit } from '@angular/core';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { BaseUtilService, I18NService, JobAPIService } from 'app/shared';

@Component({
  selector: 'aui-modify-handle',
  templateUrl: './modify-handle.component.html',
  styleUrls: ['./modify-handle.component.css']
})
export class ModifyHandleComponent implements OnInit {
  MAX_LENGTH = 1024;
  formGroup: FormGroup;
  row;
  markErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH
    ])
  };

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private jobApiService?: JobAPIService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      mark: new FormControl(this.row?.mark || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)
        ],
        updateOn: 'change'
      })
    });
  }

  onOK() {
    return this.jobApiService.setJobMarkAndMarkStatusUsingPOST({
      jobId: this.row?.jobId,
      modifyMarkRequest: {
        mark: this.formGroup.value.mark
      }
    });
  }
}
