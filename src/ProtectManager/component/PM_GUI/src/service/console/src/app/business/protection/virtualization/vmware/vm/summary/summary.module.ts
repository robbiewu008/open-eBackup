import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { SummaryComponent } from './summary.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [BaseModule, BaseInfoModule, CustomTableSearchModule]
})
export class SummaryModule {}
