import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SetQuotaComponent } from './set-quota.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SetQuotaComponent],
  imports: [CommonModule, BaseModule],
  exports: [SetQuotaComponent]
})
export class SetQuotaModule {}
