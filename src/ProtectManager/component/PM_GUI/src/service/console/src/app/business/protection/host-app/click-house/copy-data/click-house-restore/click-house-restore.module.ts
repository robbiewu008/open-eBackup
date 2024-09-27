import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { ClickHouseRestoreComponent } from './click-house-restore.component';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [ClickHouseRestoreComponent]
})
export class ClickHouseRestoreModule {}
