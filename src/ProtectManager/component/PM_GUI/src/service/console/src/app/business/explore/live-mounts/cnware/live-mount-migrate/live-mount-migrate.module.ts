import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountMigrateComponent } from './live-mount-migrate.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [LiveMountMigrateComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class LiveMountMigrateModule {}
