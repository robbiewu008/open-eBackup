import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GaussDBTRestoreComponent } from './gauss-t-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [GaussDBTRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class GaussDBTRestoreModule {}
