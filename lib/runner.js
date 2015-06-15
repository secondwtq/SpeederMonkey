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

var new_context = function () {
    var ret = _native_module.create_global_env();

    for (var i = 0; i < blacklist.length; i++) {
    	ret[blacklist[i]] = undefined; }

    ret.print = speeder.print;
    ret.getline = speeder.getline;
    ret.process = new process_runner();

    return ret;
};

var run = function (source, context) {
	if (context === undefined) {
		context = new_context(); }

	context.module = new node_module('JSRunnerApp');
	context.exports = context.module.exports;

	var org_require = context.module.require;
	var require_new = function (name) {
		if (module_blacklist.indexOf(name) !== -1) {
			return undefined; }
		var sub_context = new_context();

		sub_context.module = new node_module(name);
		sub_context.exports = sub_context.module.exports;
		sub_context.module.require = require_new;
		sub_context.require = function (name) {
			return sub_context.module.require(name); };

		return org_require.call(sub_context.module, name, sub_context);
	};
	context.module.require = require_new;
	context.require = function (name) {
		return context.module.require(name); };

	_native_module.eval_in_sandbox(source, context, 'JSRunnerApp');
};

module.exports = {
    new_context: new_context,
    run: run
};
