import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LiveMountMigrateComponent } from './live-mount-migrate.component';

@NgModule({
  declarations: [LiveMountMigrateComponent],
  imports: [CommonModule, BaseModule],
  exports: [LiveMountMigrateComponent]
})
export class LiveMountMigrateModule {}
