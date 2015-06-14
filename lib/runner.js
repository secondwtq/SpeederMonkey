/**
 * Created by secondwtq on 15-6-15.
 */

'use strict';

var new_context = function () {
    var ret = _native_module.create_global_env();

    ret.require = function (name) {
      return require(name);
    };

    ret.print = print;
};

module.exports = {
    new_context: new_context
};
