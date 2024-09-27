import { NgModule } from '@angular/core';
import { SummaryComponent } from './summary.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SummaryComponent],
  imports: [BaseModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}
