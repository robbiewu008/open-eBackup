const fs = require('fs'),
  path = require('path');
const baseFilename = 'init.json',
  sourceFile = getFilePath(baseFilename),
  swaggerFile = getFilePath('../swagger.json');

combineJsonFile();

function combineJsonFile() {
  const json = getAllJson()
    .map(file => readJsonFile(getFilePath(file)))
    .map(json => {
      convertPath(json.paths);
      deleteKeys(json, 'swagger', 'info', 'basePath', 'host');
      return json;
    })
    .reduce((a, b) => {
      return assign(a, b);
    });

  const result = assign(readJsonFile(sourceFile), json);
  writerJsonFile(result);
}

function convertPath(paths) {
  for (var url in paths) {
    var methods = paths[url];
    for (var method in methods) {
      var parameters = methods[method].parameters;
      if (!parameters) {
        methods[method]['parameters'] = [];
        continue;
      }
      parameters.forEach(function(parameter, key) {
        // TODO: remove Date https://github.com/google/shaka-player/issues/159
        if (parameter.name === 'Date') {
          parameters.splice(key, 1);
          console.log('Remove Date');
        }

        if (parameter.name === 'X-Request-ID') {
          parameters.splice(key, 1);
          console.log('Remove X-Request-ID');
        }

        if (['x-auth-token', 'X-Auth-Token'].includes(parameter.name)) {
          parameters.splice(key, 1);
          console.log('Remove X-Auth-Token');
        }
      });
    }
  }
}

function writerJsonFile(json) {
  fs.writeFile(swaggerFile, JSON.stringify(json, null, 2), 'utf8', function(
    err
  ) {
    if (err) {
      console.error(err);
      return;
    }
    console.log('写入swagger.json成功');
  });
}

function deleteKeys() {
  const source = arguments[0],
    keys = [...arguments].slice(1);
  keys.forEach(item => {
    delete source[item];
  });
}

function readJsonFile(path) {
  return JSON.parse(fs.readFileSync(path, 'utf-8'));
}

function getAllJson() {
  return fs
    .readdirSync(path.resolve(__dirname, '../json'))
    .filter(file => file.split('.').pop() === 'json');
}

function getFilePath(name) {
  if (name === 'init.json' || name === 'swagger.json') {
    return path.resolve(__dirname, name);
  }
  return path.resolve(path.resolve(__dirname, '../json'), name);
}

function assign(source1, source2) {
  Object.keys(source2).forEach(key => {
    let temp1 = source1[key],
      temp2 = source2[key];
    if (isPlainObject(temp1)) {
      source1[key] = extend(true, temp1, temp2);
    } else if (Array.isArray(temp1)) {
      temp1.push(...temp2);
    } else {
      source1[key] = temp2;
    }
  });
  return source1;
}

function extend() {
  var src,
    copyIsArray,
    copy,
    name,
    options,
    clone,
    target = arguments[0] || {},
    i = 1,
    length = arguments.length,
    deep = false;

  // Handle a deep copy situation
  if (typeof target === 'boolean') {
    deep = target;

    // skip the boolean and the target
    target = arguments[i] || {};
    i++;
  }

  // Handle case when target is a string or something (possible in deep copy)
  if (typeof target !== 'object') {
    target = {};
  }

  // extend jQuery itself if only one argument is passed
  if (i === length) {
    target = this;
    i--;
  }

  for (; i < length; i++) {
    // Only deal with non-null/undefined values
    if ((options = arguments[i]) != null) {
      // Extend the base object
      for (name in options) {
        src = target[name];
        copy = options[name];

        // Prevent never-ending loop
        if (target === copy) {
          continue;
        }

        // Recurse if we're merging plain objects or arrays
        if (
          deep &&
          copy &&
          (isPlainObject(copy) || (copyIsArray = Array.isArray(copy)))
        ) {
          if (copyIsArray) {
            copyIsArray = false;
            clone = src && Array.isArray(src) ? src : [];
          } else {
            clone = src && isPlainObject(src) ? src : {};
          }

          // Never move original objects, clone them
          target[name] = extend(deep, clone, copy);

          // Don't bring in undefined values
        } else if (copy !== undefined) {
          target[name] = copy;
        }
      }
    }
  }

  // Return the modified object
  return target;
}

function isPlainObject(source) {
  return Object.prototype.toString.call(source) === '[object Object]';
}
