print('entering mt2');

exports.mt2_t = function () {
	print('this is mt2'); }

exports.export_func_2 = function () {
	return 1; };

exports.t_export = function (func) {
	if (exports.export_func_2()) {
		return func;
	}
};

exports.export_func_4 = exports.t_export(function () { });

print('exiting mt2');