import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreAddHostComponent } from './postgre-add-host.component';

@NgModule({
  declarations: [PostgreAddHostComponent],
  imports: [CommonModule, BaseModule]
})
export class PostgreAddHostModule {}
