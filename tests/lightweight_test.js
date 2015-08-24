var t = vx_test;

// var t = function (a) {
// 	this.test = parseInt(a);
// 	this.objref = undefined;
// }

// t.prototype.test_func = function (i) {
// 	return i; };

// 0.54s
// -O3 - 0.33s, still, the pot for memory ops.
//var obj_array = { };
//var val_array = { };
//for (var i = 0; i < 600000; i++) {
//	obj_array[i] = new t(i);
//}
//for (var i = 0; i < 600000; i++) {
//	val_array[i] = obj_array[i].test;
//}

var __extends = this.__extends || function (d, b) {
        for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
        function __() { this.constructor = d; }
        __.prototype = b.prototype;
        d.prototype = new __();
    };

var cpt = new CopyTestCopied();

var CopyTestExtended = (function () {
    function CopyTestExtended() { }
    return CopyTestExtended;
})();
CopyTestExtended.prototype = CopyTest.prototype;
CopyTestExtended.prototype.constructor = CopyTestExtended;

var CopyTestExtendedEx = (function (_super) {
    __extends(CopyTestExtendedEx, _super);
    function CopyTestExtendedEx(cpt) {
        _super.call(this, cpt); }
    CopyTestExtendedEx.prototype.testExtend = function () {
        print("this is test extend");
    };
    return CopyTestExtendedEx;
})(CopyTestExtended);

function createCopyTestExtended() {
    var ret = CopyTest.reproto(this.prototype);
    CopyTest.attachNew.call(ret, arguments[0]);
    this.apply(ret, arguments);
    return ret;
}

function createCopyTestExtended2(cpt) {
    var ret = CopyTest.reproto(CopyTestExtended.prototype);
    CopyTest.attachNew.call(ret, cpt);
    CopyTestExtended.apply(ret, cpt);
    return ret;
}

function createCopyTestExtended3(ext, cpt) {
    var ret = CopyTest.reproto(ext.prototype);
    CopyTest.attachNew.call(ret, cpt);
    ext.apply(ret, cpt);
    return ret;
}

var obj_array = { };
for (var i = 0; i < 600000; i++) {
    //obj_array[i] = createCopyTestExtended3(CopyTestExtended, cpt);
    //obj_array[i] = createCopyTestExtended2(cpt);
    obj_array[i] = new CopyTest(cpt);
    //obj_array[i] = createCopyTestExtended.call(CopyTestExtendedEx, cpt);
}

// 0.77 - 0.80s
// -O3 - 0.07 - 0.09s
//var a = new t(10);
//for (var i = 0; i < 5000000; i++) {
//	a.test_func(i);
//}

// 1.16 - 1.21s, several GCs is triggered
// no improvement with intrusive
// -O3 - 0.09s
//var a = new t(10);
//var b = new t(12);
//for (var i = 0; i < 5000000; i++) {
// 	a.test_func_objptr(i, b);
//}

// 0.57 - 0.60s, several GCs is triggered
// intrusive - 0.45 - 0.47s
//
// -O3 - 0.33 - 0.35s,
//	and it seems GC and memory allocations takes the grater
//
// var obj_array = { };
// var val_array = null;
// for (var i = 0; i < 250000; i++) {
// 	obj_array[i] = new t(i);
// 	obj_array[i].objref = new t(2*i);
// }
// for (var i = 0; i < 250000; i++)
// 	val_array = obj_array[i].objref.test;

// 1.63s - 2.00s, several GCs is triggered
// 	but the intrusive version is much better,
//	0.49 - 0.51s, no GC.
//
// -O3 1.08 - 1.09s
//
//var a = new t(1);
//a.objref = new t(2);
//var b;
//for (var i = 0; i < 5000000; i++) {
//	b = a.objref;
//	b = undefined;
//}
