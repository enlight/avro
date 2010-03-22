assert_equal = function(a, b) {
    if (a != b) {
        alert('' + a + ' != ' + b);
    }
}
var bb = Avro.raw.create_byte_buffer_from_string;
assert_equal(Avro.raw.decode_long(bb('\x06')), 3);
assert_equal(Avro.raw.decode_long(bb('\x03')), -2);
assert_equal(Avro.raw.decode_string(bb('\x06foo')), 'foo')
