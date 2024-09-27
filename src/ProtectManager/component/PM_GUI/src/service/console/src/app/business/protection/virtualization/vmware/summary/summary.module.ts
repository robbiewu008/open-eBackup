import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components/custom-modal-operate';
import { SummaryComponent } from './summary.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [BaseModule, CustomModalOperateModule, CustomTableSearchModule]
})
export class SummaryModule {}
