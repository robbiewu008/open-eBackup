import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';
import { ExceptionService } from '../services';

@Injectable()
export class ExceptionInterceptor implements HttpInterceptor {
  constructor(private exceptionService: ExceptionService) {}

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    const doException = req.params.get('akDoException') !== 'false';

    // 删除akDoException参数
    req = req.clone({
      params: req.params.delete('akDoException')
    });

    return next.handle(req).pipe(
      tap(
        data => {
          if (data instanceof HttpResponse) {
            if (doException && this.exceptionService.isException(data)) {
              this.exceptionService.doException(data);
            }
          }
        },
        error => {
          if (doException) {
            this.exceptionService.doException(error);
          }
        }
      )
    );
  }
}
