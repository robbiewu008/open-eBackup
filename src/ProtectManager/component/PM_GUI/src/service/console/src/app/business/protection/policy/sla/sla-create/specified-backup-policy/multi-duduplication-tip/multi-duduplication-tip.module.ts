import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MultiDuduplicationTipComponent } from './multi-duduplication-tip.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [MultiDuduplicationTipComponent],
  imports: [CommonModule, BaseModule],
  exports: [MultiDuduplicationTipComponent]
})
export class MultiDuduplicationTipModule {}
