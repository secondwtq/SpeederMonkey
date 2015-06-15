/**
 * Created by secondwtq on 15-6-15.
 */

var is_buffer = function (arg) {
	return false; };

var pool_size = 8 * 1024;
var default_encoding = 'utf8';

var Buffer = function (subject, encoding) {
	if (!is_buffer(this)) {
		return new Buffer(subject, encoding); }

	if (util.isNumber(subject)) {
		this.length = +subject;
	} else if (util.isString(subject)) {
		if (!util.isString(encoding) || encoding.length === 0) { encoding = default_encoding; }
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
	if (util.isString(subject)) {
		var len = this.write(subject, encoding);
		if (len < this.length) {
			this.length = len;
			this._buffer._truncate(len); }
	} else if (util.isBuffer(subject)) {
		subject._buffer._copy(this._buffer, 0, 0, this.length);
	} else if (util.isNumber(subject.length) || util.isArray(subject)) {
		// TODO: UIntArray
	}
};

Buffer.prototype.toString = function (encoding, start, end) {
	start = start >>> 0;
	end = util.isUndefined(end) || end === Infinity ? this.length : end >>> 0;
	if (!encoding) encoding = default_encoding;
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
	// TODO: type guard
	return _native_buffer.compare(this._buffer, other._buffer) === 0;
};

Buffer.prototype.compare = function (x, y) {
	// TODO: type guard
	return _native_buffer.compare(x._buffer, y._buffer);
};

Buffer.prototype.inspect = function () {
	var r = '';
	var max = exports.INSPECT_MAX_BYTES;
	if (this.length > 0) {
		r = this.toString('hex', 0, max).match(/.{2}/g).join(' ');
		if (this.length > max) {
			r += ' ... '; }
	}
	return '<' + this.constructor.name + ' ' + r + '>';
};

Buffer.prototype.fill = function (val, start, end) {
	start = start >> 0;
	end = (end === undefined) ? this.length : end >> 0;
	if (start < 0 || end > this.length) { throw new RangeError('out of range index'); }
	if (end <= start) return this;

	if (typeof val !== 'string') { val = val >>> 0; }
	else if (val.length === 1) {
		var code = val.charCodeAt(0);
		if (code < 256) { val = code; }
		else {
			// TODO: fill with encoding
			return;
		}
	}

	this._buffer._fill_num(val, start, end);
	return this;
};

Buffer.prototype.write = function (string, offset, length, encoding) {
	// (string)
	if (util.isUndefined(offset)) {
		encoding = default_encoding;
		length = this.length;
		offset = 0;
	}
	// (string, encoding)
	else if (util.isUndefined(length) && util.isString(offset)) {
		encoding = offset;
		length = this.length;
		offset = 0;
	}
	// (string, offset[, length][, encoding])
	else if (isFinite(offset)) {
		offset = offset >>> 0;
		if (isFinite(length)) {
			length = length >>> 0;
			if (util.isUndefined(encoding)) {
				encoding = default_encoding; }
		} else {
			encoding = length;
			length = undefined;
		}
	}

	var remaining = this.length - offset;
	if (util.isUndefined(length) || length > remaining) {
		length = remaining; }
	encoding = !!encoding ? (encoding+'').toLowerCase() : default_encoding;
	if (string.length > 0 && (length < 0 || offset < 0)) {
		throw new RangeError('attempt to write outside buffer bounds'); }

	var write_map = {
		'hex': this.hexWrite, 'utf8': this.utf8Write, 'utf-8': this.utf8Write,
		'ascii': this.asciiWrite, 'binary': this.binaryWrite, 'base64': this.base64Write,
		'ucs2': this.ucs2Write,
		'ucs-2': this.ucs2Write,
		'utf16le': this.ucs2Write,
		'utf-16le': this.ucs2Write
	};
	var ret;
	if (write_map[encoding] !== undefined) {
		ret = write_map[encoding](string, offset, length);
	} else { throw new TypeError('Unknown encoding: ' + encoding); }
	return ret;
};

Buffer.prototype.toJSON = function () {
	return {
		type: 'Buffer', data: [ ] // TODO: toArray
	};
};

Buffer.isEncoding = function (encoding) {
	switch ((encoding+'').toLowerCase()) {
		case 'hex':
		case 'utf8':
		case 'utf-8':
		case 'ascii':
		case 'binary':
		case 'base64':
		case 'ucs2':
		case 'ucs-2':
		case 'utf16le':
		case 'utf-16le':
		case 'raw':
			return true;

		default:
			return false;
	}
};

exports.INSPECT_MAX_BYTES = 50;
exports.Buffer = Buffer;

