import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TaskCountComponent } from './task-count.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [TaskCountComponent],
  exports: [TaskCountComponent]
})
export class TaskCountModule {}
