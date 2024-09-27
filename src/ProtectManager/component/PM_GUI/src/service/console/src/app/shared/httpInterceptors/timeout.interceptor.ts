import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { timeout } from 'rxjs/operators';
import { HttpExtParams } from './http.params';

@Injectable()
export class TimeoutInterceptor extends HttpExtParams
  implements HttpInterceptor {
  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    // 删除akTimeout参数
    req = req.clone({
      params: req.params.delete('akTimeout')
    });

    return next
      .handle(req)
      .pipe(timeout(+req.params.get('akTimeout') || this.akTimeout));
  }
}
