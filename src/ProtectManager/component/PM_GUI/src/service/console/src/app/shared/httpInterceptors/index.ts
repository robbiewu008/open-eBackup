import { HTTP_INTERCEPTORS } from '@angular/common/http';
import { CollectionDataInterceptor } from './collection-data.interceptor';
import { DownloadInterceptor } from './download.interceptor';
import { ExceptionInterceptor } from './exception.interceptor';
import { HeaderInterceptor } from './header.interceptor';
import { LoadingInterceptor } from './loading.interceptor';
import { OperationTipsInterceptor } from './operation-tips.interceptor';
import { TimeoutInterceptor } from './timeout.interceptor';
import { UrlInterceptor } from './url.interceptor';
import { PermissionInterceptor } from './permission.interceptor';

export const HttpInterceptorProviders = [
  {
    provide: HTTP_INTERCEPTORS,
    useClass: LoadingInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: CollectionDataInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: ExceptionInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: TimeoutInterceptor,
    multi: true
  },
  { provide: HTTP_INTERCEPTORS, useClass: UrlInterceptor, multi: true },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: HeaderInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: DownloadInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: OperationTipsInterceptor,
    multi: true
  },
  {
    provide: HTTP_INTERCEPTORS,
    useClass: PermissionInterceptor,
    multi: true
  }
];

export * from './http.params';
