import { CommonModule } from '@angular/common';
import {
  Component,
  Input,
  NgModule,
  OnChanges,
  SimpleChanges,
  OnInit
} from '@angular/core';
import { IconModule } from '@iux/live';
import { DataMapService, I18NService } from '../services';

@Component({
  selector: 'sla-type',
  template: `
    <ng-container *ngIf="!!name; else elseTemplate">
      <ng-container [ngSwitch]="name">
        <ng-container *ngSwitchCase="'Gold'">
          <i lv-icon="aui-sla-gold" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchCase="'Silver'">
          <i lv-icon="aui-sla-silver" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchCase="'Bronze'">
          <i lv-icon="aui-sla-bronze" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchDefault>
          <i lv-icon="aui-sla-myvmprotect" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
      </ng-container>
    </ng-container>
    <ng-template #elseTemplate>
      --
    </ng-template>
  `
})
export class SlaTypeComponent implements OnInit, OnChanges {
  @Input() name: string;

  constructor(
    public I18N: I18NService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {}

  ngOnChanges(changes: SimpleChanges) {}
}

@NgModule({
  declarations: [SlaTypeComponent],
  imports: [CommonModule, IconModule],
  exports: [SlaTypeComponent]
})
export class SlaTypeModule {}
