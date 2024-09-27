import {
  Component,
  OnInit,
  NgModule,
  Input,
  Output,
  EventEmitter
} from '@angular/core';
import { CommonModule } from '@angular/common';
import { each, assign, isEmpty } from 'lodash';
import { RememberColumnsService } from '../services/remember-columns.service';
import { CheckboxModule } from '@iux/live';
import { FormsModule } from '@angular/forms';

@Component({
  selector: 'column-filter-tpl',
  template: `
    <div class="lv-filter-select">
      <div class="lv-filter-select-container">
        <div class="lv-filter-select-list" style="max-height: 4rem;">
          <ng-container *ngFor="let item of columns">
            <ng-container *ngIf="!item.hidden">
              <div
                class="lv-filter-select-item lv-filter-select-multiple"
                [ngClass]="{ selected: item.isShow }"
                style="cursor: default"
                (click)="toggleAlarmSelect()"
              >
                <label lv-checkbox [(ngModel)]="item.isShow">
                  {{ item.label }}
                </label>
              </div>
            </ng-container>
          </ng-container>
        </div>
      </div>
    </div>
  `
})
export class ColumnFilterTplComponent implements OnInit {
  @Input() tableKey: string;
  @Input() columns;
  @Output() columnsStatusChange = new EventEmitter();

  constructor(private rememberColumnsService: RememberColumnsService) {}

  toggleAlarmSelect() {
    if (isEmpty(this.tableKey)) {
      return;
    }
    const columnsStatus = {};
    each(this.columns, col => {
      assign(columnsStatus, {
        [col.key]: col.isShow
      });
    });
    this.rememberColumnsService.setColumnsStatus(this.tableKey, columnsStatus);
    this.columnsStatusChange.emit(columnsStatus);
  }

  ngOnInit() {}
}

@NgModule({
  declarations: [ColumnFilterTplComponent],
  imports: [CommonModule, CheckboxModule, FormsModule],
  exports: [ColumnFilterTplComponent]
})
export class ColumnFilterTplModule {}
