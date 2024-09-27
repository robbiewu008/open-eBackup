import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-feedback-warning',
  templateUrl: './feedback-warning.component.html',
  styleUrls: ['./feedback-warning.component.less']
})
export class FeedbackWarningComponent implements OnInit {
  status;
  isSecuritySnap = false;
  feedbackTplLabel = '';

  isFeedbackChecked$ = new Subject<boolean>();

  constructor(public i18n: I18NService) {}

  ngOnInit() {}

  warningConfirmChange(e) {
    this.isFeedbackChecked$.next(e);
  }
}
