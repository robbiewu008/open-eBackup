export class HttpExtParams {
  // 统一URL前缀
  akPrefix: string | 'none';

  // 是否显示loading效果
  akLoading = true;

  // 超时时间，默认30分钟
  akTimeout: number = 30 * 60 * 1e3;

  // 是否使用公共异常
  akDoException = true;

  // 是否逃逸注销
  akEscapeSession = false;
}
