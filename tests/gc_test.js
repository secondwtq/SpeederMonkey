/**
 * Created by secondatke on 15-6-2.
 */

var p_global = new parent();

function test1() {
    var p_inside = new parent();
}

test1();

collectgarbage();

p_global.func();