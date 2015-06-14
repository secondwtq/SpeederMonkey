/**
 * Created by secondatke on 15-6-2.
 */

var p_global = new parent();

function test1() {
    var p_inside = new parent();
}

collectgarbage();

p_global.func();