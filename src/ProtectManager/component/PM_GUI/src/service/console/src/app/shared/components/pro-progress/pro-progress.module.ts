import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ProgressModule, TooltipModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { ProProgressComponent } from './pro-progress.component';

@NgModule({
  declarations: [ProProgressComponent],
  imports: [
    CommonModule,
    FormsModule,
    TooltipModule,
    ProgressModule,
    BaseModule
  ],
  exports: [ProProgressComponent]
})
export class ProProgressModule {}
