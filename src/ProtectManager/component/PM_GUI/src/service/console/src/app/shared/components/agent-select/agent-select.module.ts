import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { IconModule, SelectModule } from '@iux/live';
import { StatusModule } from '../status';
import { AgentSelectComponent } from './agent-select.component';

@NgModule({
  declarations: [AgentSelectComponent],
  imports: [CommonModule, SelectModule, IconModule, FormsModule, StatusModule],
  exports: [AgentSelectComponent]
})
export class AgentSelectModule {}
