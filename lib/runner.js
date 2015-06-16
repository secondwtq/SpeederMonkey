/**
 * Created by secondwtq on 15-6-15.
 */

'use strict';

var process_runner = function () {
	this.argv = process.argv.slice(2); };

process_runner.prototype.cwd = function () { };

process_runner.prototype.exit = function () { };

var blacklist = [
	'_native_buffer', '_native_module', 'process'
];

var module_blacklist = [ 'fs' ];

var new_context = function (name) {
    var ret = _native_module.create_global_env();

    for (var i = 0; i < blacklist.length; i++) {
    	ret[blacklist[i]] = undefined; }

    ret.print = speeder.print;
    ret.getline = speeder.getline;
    ret.process = new process_runner();

    ret.module = new node_module('JSRunnerApp');
    ret.exports = ret.module.exports;
    ret.require = function (name) {
		return ret.module.require(name); };
	ret.require_source = function (name, source) {
		return ret.module.require_source(name, source); };

    return ret;
};

var run = function (source, context) {
	if (context === undefined) {
		context = new_context('JSRunnerApp'); }

	var org_require = context.module.require;
	var org_require_source = context.module.require_source;
	
	var require_new, require_source_new;
	var get_context = function (name) {
		var _context = new_context(name);
		_context.module.require = require_new;
		_context.module.require_source = require_source_new;
		return _context;
	}

	require_new = function (name) {
		if (module_blacklist.indexOf(name) !== -1) { return undefined; }
		var _context = get_context(name);
		return org_require.call(_context.module, name, _context);
	};
	require_source_new = function (name, source) {
		var _context = get_context(name);
		return org_require_source.call(_context.module, name, source, _context); };

	context.module.require = require_new;	
	context.module.require_source = require_source_new;

	_native_module.eval_in_sandbox(source, context, 'JSRunnerApp');

	return context.module.exports;
};

module.exports = {
    new_context: new_context,
    run: run
};
