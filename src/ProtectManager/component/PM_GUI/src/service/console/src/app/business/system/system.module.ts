import { NgModule } from '@angular/core';
import { SystemComponent } from './system.component';
import { SystemRouterModule } from './system-router.module';
import { LayoutModule, MenuModule, GroupModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  imports: [
    BaseModule,
    SystemRouterModule,
    LayoutModule,
    MenuModule,
    GroupModule
  ],
  declarations: [SystemComponent]
})
export class SystemModule {}
