
// node.js, module.js, node_contextify.cc

module = function (name) {
	this.name = name;
	this.exports = { };
};

module.exports = { };
module.globalPaths = [ ];

var _module_init = function () {

	var _cache = { };
	var exports = { };

	var wrap = function (source) {
		return '(function (exports, require, __filename) { ' + source.toString() + '\n});'; };

	var init_native = function () {
		var init_mod = function (name, exp) {
			var ret = new module(name);
			ret.exports = exp;
			
			_cache[name] = ret;
		};

		init_mod('os', _node_native_os);
		init_mod('fs', _node_native_fs);
	};

	var init_internal = function () {
		var init_int = function (name) {
			module._load_js_module(name + '.js', name);
		}

		init_int('util');
		init_int('path');
	}

	var init_path = function () {
		var paths = [ ];
	};

	module._load_js_module = function (path, name, new_context) {

		if (_cache[name])
			return _cache[name].exports;

		if (new_context === undefined) { new_context = true; }

		var source = readfile(path);
		var mod = new module(name);
		_cache[name] = mod;

		if (new_context) {
			var context = _native_module.create_global_env();
			context.print = print;

			context.exports = mod.exports;
			context.require = mod.require;
			context.process = process;
			context.__filename = "";

			_native_module.eval_in_sandbox(source, context, name);
		} else {
			source = wrap(source);
			_native_module.eval(source, name)(mod.exports, module.prototype.require);
		}

		return mod.exports;
	};

	module.resolver_general = function (name, new_context) {
		return module._load_js_module(name + '.js', name, new_context); };

	module.prototype.require = function (name, new_context) {
		return module.resolver_general(name, new_context);
	};

	require = module.prototype.require;
	init_native();
	init_internal();
};

_module_init();
