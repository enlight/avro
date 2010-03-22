Avro = {}
Avro.raw = {}

Avro.raw.create_byte_buffer_from_string = function(data_string) {
    var byte_buffer = {
        read: 0,
        data: new Array(data_string.length),
        read_byte: function() {
            return this.data[this.read++];
        },
        read_bytes: function(length) {
            var bytes = new Array(length);
            for (var i = 0; i < length; i++) {
                bytes[i] = this.data[this.read++]
            }
            return bytes;
        }
    };
    for (var i = 0; i < data_string.length; i++) {
        byte_buffer.data[i] = data_string.charCodeAt(i);
    }
    return byte_buffer;
}

Avro.raw.decode_long = function(byte_buffer) {
    var b, value = 0, offset = 0;
    do {
        if (offset == 10) {
            /*
             * illegal byte sequence 
             */
            return Number.NaN;
        }
        b = byte_buffer.read_byte();
        value |= (b & 0x7F) << (7 * offset);
        ++offset;
    }
    while (b & 0x80);
    value = ((value >> 1) ^ -(value & 1));
    return value;
}

Avro.raw.decode_bytes = function(byte_buffer) {
    var length = this.decode_long(byte_buffer);
    return byte_buffer.read_bytes(length);
}

Avro.raw.decode_string = function(byte_buffer) {
    var bytes = this.decode_bytes(byte_buffer);
    var str = ''
    var i = 0;
    var c = c1 = c2 = 0;
    while (i < bytes.length) {
        c = bytes[i]
        if (c < 128) {
            str += String.fromCharCode(c);
            i++;
        } else if((c > 191) && (c < 224)) {
            c2 = bytes[i+1];
            str += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
            i += 2;
        } else {
            c2 = bytes[i+1];
            c3 = bytes[i+2];
            str += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
            i += 3;
        }
    }
    return str;
}

Avro.raw.decode_float = function(byte_buffer) {
}

Avro.raw.decode_double = function(byte_buffer) {
}

Avro.raw.decode_boolean = function(byte_buffer) {
    var byte = byte_buffer.read_byte();
    if (byte == 0) {
        return false;
    }
    return true;
}

Avro.raw.decode_null = function(byte_buffer) {
    // Nothing to do.
}

