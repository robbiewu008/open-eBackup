import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BatchConfigPathComponent } from './batch-config-path.component';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [BatchConfigPathComponent],
  imports: [CommonModule, BaseModule, AlertModule],
  exports: [BatchConfigPathComponent]
})
export class BatchConfigPathModule {}
