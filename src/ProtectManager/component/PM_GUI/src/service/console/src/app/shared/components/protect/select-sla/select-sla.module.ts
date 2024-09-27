import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SlaCreateModule } from 'app/business/protection/policy/sla/sla-create/sla-create.module';
import { BaseModule } from 'app/shared';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { SelectSlaComponent } from './select-sla.component';
import { CustomTableSearchModule } from '../../custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [SelectSlaComponent],
  imports: [
    CommonModule,
    BaseModule,
    SlaServiceModule,
    SlaCreateModule,
    SlaServiceModule,
    CustomTableSearchModule
  ],

  exports: [SelectSlaComponent]
})
export class SelectSlaModule {}
