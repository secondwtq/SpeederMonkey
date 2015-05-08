var t = vx_test;

// var t = function (a) {
// 	this.test = parseInt(a);
// 	this.objref = undefined;
// }

// t.prototype.test_func = function (i) {
// 	return i; };

// obj_array = { };
// val_array = { };
// for (var i = 0; i < 600000; i++) {
// 	obj_array[i] = new t(i);
// }
// for (var i = 0; i < 600000; i++) {
// 	val_array[i] = obj_array[i].test;
// }
// print(val_array[2333]);

var a = new t(10);
for (var i = 0; i < 5000000; i++) {
	a.test_func(i);
}

// var a = new t(10);
// var b = new t(12);
// for (var i = 0; i < 5000000; i++) {
// 	a.test_func_objptr(i, b);
// }

// var obj_array = { };
// var val_array = null;
// for (var i = 0; i < 250000; i++) {
// 	obj_array[i] = new t(i);
// 	obj_array[i].objref = new t(2*i);
// }
// for (var i = 0; i < 250000; i++)
// 	val_array = obj_array[i].objref.test;

// var a = new t(1);
// a.objref = new t(2);
// var b;
// for (var i = 0; i < 10000000; i++)
// 	b = a.objref;