print('enteing mt');
var os = require('os');
var mt2 = require('mt2');

print('ttt');
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

a = 15;

print('exiting mt');