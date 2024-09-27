import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseConfigModule } from 'app/shared/components/database-config/database-config.module';
import { LiveMountOptionsComponent } from './live-mount-options.component';

@NgModule({
  declarations: [LiveMountOptionsComponent],
  imports: [BaseModule, DatabaseConfigModule],
  exports: [LiveMountOptionsComponent]
})
export class LiveMountOptionsModule {}
