import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { isEmpty, reduce } from 'lodash';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';
import { CookieService } from '../services/cookie.service';

@Injectable()
export class CollectionDataInterceptor implements HttpInterceptor {
  constructor(private cookieService: CookieService) {
    this.init();
  }

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    return next.handle(req).pipe(
      tap(res => {
        if (res instanceof HttpResponse && res.status === 200) {
          const reg = /(?=\/rest).+/,
            path = res.url.match(reg),
            enable = this.cookieService.get('enable_data_collection') == 'true';

          try {
            if (enable && path) {
              const { method, params, body, headers } = req,
                _params = params.keys().reduce((object, key) => {
                  object[key] = params.get(key);
                  return object;
                }, {}),
                _headers = headers.keys().reduce((object, key) => {
                  object[key] = headers.get(key);
                  return object;
                }, {}),
                _path = path.toString(),
                _url = `${method}  ${_path}`,
                _values = reduce(
                  {
                    method,
                    url: _url,
                    body: body,
                    params: _params,
                    headers: _headers,
                    response: res.body
                  },
                  (object, value, key) => {
                    if (!isEmpty(value)) {
                      object[key] = value;
                    }
                    return object;
                  },
                  {}
                );
              localStorage.setItem(_url, JSON.stringify(_values));
            }
          } catch (error) {
            console.error(`parse body failed, the url is ${path}`);
          }
        }
      })
    );
  }

  private init() {
    window['clear_data'] = this.clear;
    window['download_data'] = this.download;
  }

  private download() {
    const reg = /.+(?=\?)/,
      storage = {};

    for (let i = 0, len = localStorage.length; i < len; i++) {
      const key = localStorage.key(i);
      const value = localStorage.getItem(key);
      const url = key.match(reg);
      storage[url ? url.toString() : key] = JSON.parse(value);
    }

    const data = JSON.stringify(storage);
    const a = document.createElement('a');
    a.href = 'data:application/json;charset=utf-8,' + data;
    a.download = `data_${new Date().valueOf()}.json`;
    a.click();
  }

  private clear() {
    for (const key in localStorage) {
      localStorage.removeItem(key);
    }
  }
}
