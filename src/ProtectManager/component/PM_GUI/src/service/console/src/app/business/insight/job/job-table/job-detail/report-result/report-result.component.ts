import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DataMap, I18NService, TaskService } from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-report-result',
  templateUrl: './report-result.component.html',
  styleUrls: ['./report-result.component.less']
})
export class ReportResultComponent implements OnInit {
  id;
  time;
  formGroup: FormGroup;
  dataMap = DataMap;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private taskService: TaskService
  ) {}

  ngOnInit() {
    this.initForm();
  }
  initForm() {
    this.formGroup = this.fb.group({
      status: new FormControl(DataMap.snapShotJobStatus.success.value)
    });
  }
  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        status: this.formGroup.value.status,
        taskId: this.id
      };
      this.taskService
        .DeliverTaskStatus({ deliverTaskReq: params as any })
        .subscribe(res => {});
    });
  }
}
