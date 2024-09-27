import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreatLogicPortComponent } from './creat-logic-port.component';

@NgModule({
  declarations: [CreatLogicPortComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    AlertModule
  ],
  exports: [CreatLogicPortComponent]
})
export class CreatLogicPortModule {}
