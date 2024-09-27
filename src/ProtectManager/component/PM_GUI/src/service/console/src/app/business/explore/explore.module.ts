import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ExploreRoutingModule } from './explore-routing.module';
import { ExploreComponent } from './explore.component';

@NgModule({
  imports: [BaseModule, ExploreRoutingModule],
  declarations: [ExploreComponent]
})
export class ExploreModule {}
