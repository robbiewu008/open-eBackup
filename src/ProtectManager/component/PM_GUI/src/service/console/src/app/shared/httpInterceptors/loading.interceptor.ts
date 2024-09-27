import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { finalize } from 'rxjs/operators';
import {
  HttpInterceptor,
  HttpRequest,
  HttpHandler,
  HttpEvent
} from '@angular/common/http';
import { LoadingService } from '@iux/live';

@Injectable()
export class LoadingInterceptor implements HttpInterceptor {
  constructor(private loadingService: LoadingService) {}

  maskCount = 0;

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    const showLoading = req.params.get('akLoading') !== 'false';

    if (showLoading) {
      this.maskCount++;
      this.loadingService.show();
    }

    // 删除akLoading参数
    req = req.clone({
      params: req.params.delete('akLoading')
    });

    return next.handle(req).pipe(
      finalize(() => {
        if (showLoading) {
          if (--this.maskCount === 0) {
            this.loadingService.hide();
          }
        }
      })
    );
  }
}
