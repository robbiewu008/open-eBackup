function callBack(err, called, reject) {
  if (!called) {
    called = true;
    reject(err);
    return called;
  }
  return called;
}
function resolver(x, resolve) {
  if (!(x !== null && (typeof x === 'object' || typeof x === 'function'))) {
    resolve(x);
  }
}
function resolvePromise(promise2, x, resolve, reject) {
  if (x === promise2) {
    return reject(new TypeError('Chaining cycle detected for promise'));
  }
  var called;
  resolver(x, resolve);
  if (!(x !== null && (typeof x === 'object' || typeof x === 'function'))) {
    return reject(new TypeError('type error'));
  }
  try {
    var then = x.then;
    if (typeof then !== 'function') {
      resolve(x);
    } else {
      then.call(
        x,
        function(y) {
          if (!called) {
            called = true;
            resolvePromise(promise2, y, resolve, reject);
          }
        },
        function(err) {
          called = callBack(err, called, reject);
        }
      );
    }
  } catch (e) {
    called = callBack(e, called, reject);
  }
}

function Promise(executor) {
  this.state = 'pending';
  this.value = undefined;
  this.reason = undefined;
  this.onResolvedCallbacks = [];
  this.onRejectedCallbacks = [];

  function resolve(value) {
    if (this.state === 'pending') {
      this.state = 'fulfilled';
      this.value = value;
      this.onResolvedCallbacks.forEach(function(fn) {
        fn();
      });
    }
  }

  function reject(reason) {
    if (this.state === 'pending') {
      this.state = 'rejected';
      this.reason = reason;
      this.onRejectedCallbacks.forEach(function(fn) {
        fn();
      });
    }
  }

  try {
    executor(resolve.bind(this), reject.bind(this));
  } catch (err) {
    reject(err);
  }
}

Promise.prototype = {
  then: function(onFulfilled, onRejected) {
    onFulfilled =
      typeof onFulfilled === 'function'
        ? onFulfilled
        : function(value) {
            return value;
          };
    onRejected =
      typeof onRejected === 'function'
        ? onRejected
        : function(value) {
            throw value;
          };
    var promise2 = new Promise(
      function(resolve, reject) {
        if (this.state === 'fulfilled') {
          setTimeout(
            function() {
              try {
                resolvePromise(
                  promise2,
                  onFulfilled(this.value),
                  resolve,
                  reject
                );
              } catch (e) {
                reject(e);
              }
            }.bind(this),
            0
          );
        }
        if (this.state === 'rejected') {
          setTimeout(
            function() {
              try {
                resolvePromise(
                  promise2,
                  onRejected(this.reason),
                  resolve,
                  reject
                );
              } catch (e) {
                reject(e);
              }
            }.bind(this),
            0
          );
        }
        if (this.state === 'pending') {
          this.onResolvedCallbacks.push(
            function() {
              setTimeout(
                function() {
                  try {
                    resolvePromise(
                      promise2,
                      onFulfilled(this.value),
                      resolve,
                      reject
                    );
                  } catch (e) {
                    reject(e);
                  }
                }.bind(this),
                0
              );
            }.bind(this)
          );
          this.onRejectedCallbacks.push(
            function() {
              setTimeout(
                function() {
                  try {
                    resolvePromise(
                      promise2,
                      onRejected(this.reason),
                      resolve,
                      reject
                    );
                  } catch (e) {
                    reject(e);
                  }
                }.bind(this),
                0
              );
            }.bind(this)
          );
        }
      }.bind(this)
    );
    return promise2;
  }
};

// resolve方法
Promise.resolve = function(val) {
  return new Promise(function(resolve, reject) {
    resolve(val);
  });
};
// reject方法
Promise.reject = function(val) {
  return new Promise(function(resolve, reject) {
    reject(val);
  });
};
// race方法
Promise.race = function(promises) {
  return new Promise(function(resolve, reject) {
    for (var i = 0; i < promises.length; i++) {
      promises[i].then(resolve, reject);
    }
  });
};
// all方法(获取所有的promise，都执行then，把结果放到数组，一起返回)
Promise.all = function(promises) {
  var arr = [];
  var i = 0;

  function processData(index, data, resolve) {
    arr[index] = data;
    i++;
    if (i === promises.length) {
      resolve(arr);
    }
  }

  function thenFun(data) {
    processData(i, data, resolve);
  }
  return new Promise(function(resolve, reject) {
    for (var i = 0; i < promises.length; i++) {
      promises[i].then(thenFun, reject);
    }
  });
};
