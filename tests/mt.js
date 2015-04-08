print('enteing mt');
var mt2 = require('mt2');
var os = require('os');

exports.export_func_1 = function () {
	print('I exported this function export_func_1.'); };

exports.export_func_2 = function () {
	return mt2.mt2_t(); };

exports.export_func_3 = function () {
	return os.platform(); };

exports.t_export = function (func) {
	if (exports.export_func_2()) {
		return func;
	}
};

exports.export_func_4 = exports.t_export(function () { });

a = 15;