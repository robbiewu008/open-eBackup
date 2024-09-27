import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseConfigModule } from 'app/shared/components/database-config/database-config.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { LiveMountOptionsComponent } from './live-mount-options.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [LiveMountOptionsComponent],
  imports: [BaseModule, DatabaseConfigModule, ProTableModule, SelectTagModule],
  exports: [LiveMountOptionsComponent]
})
export class LiveMountOptionsModule {}
