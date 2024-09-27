import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { UpdateAgentComponent } from './update-agent.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [UpdateAgentComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [UpdateAgentComponent]
})
export class UpdateAgentModule {}
