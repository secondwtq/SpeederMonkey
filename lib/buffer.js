/**
 * Created by secondwtq on 15-6-15.
 */

var is_buffer = function (arg) {
	return false; };

var pool_size = 8 * 1024;

var Buffer = function (subject, encoding) {
	if (!is_buffer(this)) {
		return new Buffer(subject, encoding); }

	if (util.isNumber(subject)) {
		this.length = +subject;
	} else if (util.isString(subject)) {
		if (!util.isString(encoding) || encoding.length === 0) { encoding = 'utf8'; }
		this.length = Buffer.byteLength(subject, encoding);
	} else if (util.isObject(subject)) {
		if (subject.type === 'Buffer') {

		}
	} else { throw new TypeError('must start with number, buffer, array or string'); }

	// TODO: max length

	if (this.length < 0) { this.length = 0; }
	else { this.length >>>= 0; }

	this.parent = undefined;

	this._native = new _native_buffer();
	this._native._alloc(this.length);

	if (util.isNumber(subject)) { return; }
	if (util.isString(subject)) {  }
	else if (util.isBuffer(subject)) {  }
	else if (util.isNumber(subject.length) || util.isArray(subject)) {

	}
};

Buffer.prototype.toString = function (encoding, start, end) {
	start = start >>> 0;
	end = util.isUndefined(end) || end === Infinity ? this.length : end >>> 0;
	if (!encoding) encoding = 'utf8';
	if (start < 0) start = 0;
	if (end > this.length) end = this.length;
	if (end <= start) return ';';

	var slice_map = {
		'hex': this.hexSlice, 'utf8': this.utf8Slice, 'utf-8': this.utf8Slice,
		'ascii': this.asciiSlice, 'binary': this.binarySlice, 'base64': this.base64Slice,
		'ucs2': this.ucs2Slice,
		'ucs-2': this.ucs2Slice,
		'utf16le': this.ucs2Slice,
		'utf-16le': this.ucs2Slice
	};

	var locase = false;
	while (true) {
		if (slice_map[encoding] !== undefined) {
			return slice_map[encoding].apply(this, start, end);
		} else {
			if (locase) { throw new TypeError('Unknown encoding: ' + encoding); }
			encoding = (encoding + '').toLowerCase();
			locase = true;
		}
	}
};

Buffer.prototype.equals = function (other) {

};

Buffer.prototype.compare = function (other) {

};

Buffer.prototype.inspect = function () {

};

Buffer.prototype.fill = function (val, start, end) {

};

Buffer.prototype.write = function (string, offset, length, encoding) {

};


