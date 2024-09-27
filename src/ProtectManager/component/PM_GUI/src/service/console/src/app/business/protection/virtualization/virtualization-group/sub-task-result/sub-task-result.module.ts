import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SubTaskResultComponent } from './sub-task-result.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [SubTaskResultComponent],
  imports: [BaseModule, CommonModule, ProTableModule, ProButtonModule],
  exports: [SubTaskResultComponent]
})
export class SubTaskResultModule {}
