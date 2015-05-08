
// node.js module.js node_contextify.cc

module = function (name) {
	this.name = name;
	this.exports = { };
};

var _module_init = function () {

	var _cache = { };
	var exports = { };

	var wrap = function (source) {
		return '(function (exports, require) { ' + source.toString() + '\n});'; };

	var init_native = function () {
		var init_mod = function (name, exp) {
			var ret = new module(name);
			ret.exports = exp;
			
			_cache[name] = ret;
		};

		init_mod('os', _node_native_os);
		init_mod('path', _node_native_path);
		init_mod('fs', _node_native_fs);
	};

	module.prototype.require = function (name, new_context) {

		if (_cache[name])
			return _cache[name].exports;

		if (new_context === undefined) { new_context = true; }

		var source = readfile(name+'.js');
		var mod = new module(name);
		_cache[name] = mod;

		if (new_context) {
			var context = _native_module.create_global_env();
			context.print = print;

			context.exports = mod.exports;
			context.require = mod.require;
			_native_module.eval_in_sandbox(source, context, name);
		} else {
			source = wrap(source);
			_native_module.eval(source, name)(mod.exports, _require);
		}

		return mod.exports;
	};

	require = module.prototype.require;
	init_native();
};

_module_init();
