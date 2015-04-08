
function isUndefined(arg) {
  return arg === void 0;
}

var exp = { };

exp.isUndefined = isUndefined;

var test = function() {
  // Allow for deprecating things in the process of starting up.
  if (isUndefined(process));
  var a = function () { };

  return function () {
    return a.apply(); };
};

exp.p = test();
