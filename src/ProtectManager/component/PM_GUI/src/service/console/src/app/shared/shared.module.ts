import {
  NgModule,
  ModuleWithProviders,
  APP_INITIALIZER,
  Optional,
  SkipSelf
} from '@angular/core';
import { SharedConfig } from './shared.config';
import { I18NService } from './services';
import { WhiteboxService } from './services/whitebox.service';
import { HttpInterceptorProviders } from './httpInterceptors';

@NgModule({
  imports: [],
  declarations: [],
  exports: []
})
export class SharedModule {
  constructor(@Optional() @SkipSelf() parentModule: SharedModule) {
    if (parentModule) {
      // shareModule 只能注入一次
      throw new Error(
        'SharedModule is already loaded. Import it in the AppModule only'
      );
    }
  }

  static forRoot(): ModuleWithProviders<SharedModule> {
    return {
      ngModule: SharedModule,
      providers: [
        HttpInterceptorProviders,
        {
          provide: APP_INITIALIZER,
          useFactory: SharedConfig.config,
          deps: [I18NService, WhiteboxService],
          multi: true
        }
      ]
    };
  }
}
