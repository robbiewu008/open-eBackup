import { CommonModule } from '@angular/common';
import { Component, NgModule, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import { I18NService } from '../services/i18n.service';

@Component({
  selector: 'aui-info',
  template: `
    <div class="info-content">
      <ng-container *ngIf="noBreak; else elseTemplate">
        <span>{{ content }}</span>
      </ng-container>
      <ng-template #elseTemplate>
        <span [innerHTML]="content"></span>
      </ng-template>
    </div>
  `,
  styles: [
    `
      .info-content {
        word-break: break-all;
        max-height: 240px;
        overflow: auto;
      }
    `
  ]
})
export class InfoComponent implements OnInit {
  content;
  noBreak;

  constructor(public i18n: I18NService) {}

  ngOnInit() {}
}

@NgModule({
  imports: [CommonModule, FormsModule, CheckboxModule],
  declarations: [InfoComponent],

  exports: [InfoComponent]
})
export class InfoModule {}
