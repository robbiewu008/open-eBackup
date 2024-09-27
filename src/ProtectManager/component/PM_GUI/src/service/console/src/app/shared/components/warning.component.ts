import { Component, NgModule, OnInit, TemplateRef } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import { I18NService } from 'app/shared/services/i18n.service';
import { Subject } from 'rxjs';
import { NgIf, NgTemplateOutlet } from '@angular/common';

@Component({
  selector: 'aui-warning',
  template: `
    <div class="warning-content">
      <span *ngIf="contentIsString" [innerHTML]="content"></span>
      <ng-container *ngIf="contentIsTemplateRef">
        <ng-template *ngTemplateOutlet="content"></ng-template>
      </ng-container>
    </div>

    <div class="warning-checkbox">
      <label
        lv-checkbox
        [(ngModel)]="status"
        (ngModelChange)="warningConfirmChange($event)"
        >{{ i18n.get('common_warning_confirm_label') }}</label
      >
    </div>
  `,
  styles: [
    `
      .warning-content {
        max-height: 240px;
        overflow: auto;
      }
    `
  ]
})
export class WarningComponent implements OnInit {
  status;
  content;
  isChecked$ = new Subject<boolean>();
  contentIsString = false;
  contentIsTemplateRef = false;

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.contentIsString = typeof this.content === 'string';
    this.contentIsTemplateRef = this.content instanceof TemplateRef;
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }
}

@NgModule({
  imports: [FormsModule, CheckboxModule, NgIf, NgTemplateOutlet],
  declarations: [WarningComponent],

  exports: [WarningComponent]
})
export class WarningModule {}
