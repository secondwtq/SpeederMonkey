
// this file is based on the following work:
//		node.js, module.js, node_contextify.cc @ node.js

'use strict';

var context = _native_module.create_global_env();

var node_module = function (name) {
	this.name = name;
	this.exports = { };
	this.__filename = "";
};

node_module.exports = { };
node_module.globalPaths = [ '.', './lib' ];
node_module.native_modules = [  [ 'os', _node_native_os ], [ 'fs', _node_native_fs ] ];
node_module.initial_libs = [ 'util', 'path' ];
node_module.import_in_new_context = false;

var require;

(function () {

	var _cache = { };
	var exports = { };

	var initial_mod = function (name) {
		return _cache[name].exports; };

	var try_file = function (req_path) {
		if (!initial_mod('fs').existsSync(req_path)) return false;
		var stats = initial_mod('fs').statSync(req_path);
		if (stats && !stats.isDirectory()) {
			return initial_mod('fs').realpathSync(req_path);
		}
		return false;
	};

	var resolve_filename = function (req, parent) {
		var paths = [ ];

		if (parent !== undefined) {
            paths.push(initial_mod('path').dirname(parent.__filename)); }
		paths = paths.concat(node_module.globalPaths);

		if (req.charAt(0) === '/') paths = [ '' ];
		var has_trasl = (req.slice(-1) === '/');

		var filename = false;
		for (var i = 0, len = paths.length; i < len; i++) {
			var base_path = initial_mod('path').resolve(paths[i], req);
			if (!has_trasl) {
				filename = try_file(base_path);

				if (!filename)
					filename = try_file(base_path + '.js');
			}

			if (filename) { break; }
		}

		return filename;
	};

	var wrap = function (source) {
		return '(function (exports, require, module, __filename) { ' + source.toString() + '\n});'; };

	var init_native = function () {
		var init_mod = function (name, exp) {
			var ret = new node_module(name);
			ret.exports = exp;
			
			_cache[name] = ret;
		};

		init_mod('os', _node_native_os);
		init_mod('fs', _node_native_fs);
	};

	var init_internal = function () {
		for (var i = 0; i < node_module.initial_libs.length; i++)
			node_module._load_js_module('lib/' + node_module.initial_libs[i] + '.js',
										node_module.initial_libs[i]);
	}

	var init_path = function () {
		var paths = [ ]; };

	node_module.add_cached = function (module) {
		_cache[module.name] = module; };

	node_module._load_js_module = function (path, name, new_context) {
		var source = require('fs').readFileSync(path);
		return node_module._load_js_source(source, name, new_context);
	};

	node_module._load_js_module = function (path, name, new_context) {

		if (_cache[name])
			return _cache[name].exports;

		if (new_context === undefined) new_context = node_module.import_in_new_context;

		var source = require('fs').readFileSync(path);
		var mod;

		if (typeof new_context === 'object' && new_context.module) {
			mod = new_context.module; 
		} else {
			mod = new node_module(name);
			node_module.add_cached(mod);
			mod.__filename = path;
		}
		node_module.add_cached(mod);

		var require_func = function (name, new_context) {
			return mod.require(name, new_context); };

		if (new_context !== false) {
			var context;

			if (new_context === true) {
				context = _native_module.create_global_env();
				context.print = print;

				context.require = require_func;
				context.process = process;
				context.module = mod;
				context.__filename = mod.__filename;
				context.exports = mod.exports;
			} else { context = new_context; }

			// TODO: we still have a bug,
			//			intermediate name resolve triggers a 'internal - too much rec.'
			///			instead of 'undefined' exception.

			_native_module.eval_in_sandbox(source, context, name);
		} else {
			source = wrap(source);
			_native_module.eval(source, name)
				(mod.exports, require_func, mod, path);
		}

		return mod.exports;
	};

	node_module.resolver_general = function (name, new_context, parent) {
		var filename = resolve_filename(name, parent);
		if (filename) {
			return node_module._load_js_module(filename, name, new_context); }
		return undefined;
	};

	node_module.prototype.require = function (name, new_context) {
		if (_cache[name])
			return _cache[name].exports;
		if (name === 'util' || name === 'path')
			return node_module._load_js_module(name+'.js', name, new_context);
		return node_module.resolver_general(name, new_context, this);
	};

	require = node_module.prototype.require;
	init_native();
	init_internal();
}) ();

if (process.argv[1]) {
	node_module.resolver_general(process.argv[1], false, undefined);
} else { print("No module provided!"); }
