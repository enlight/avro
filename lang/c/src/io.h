/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0 
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License. 
 */

#include "avro_private.h"
#include <errno.h>
#include <string.h>

struct avro_reader_t_ {
	unsigned long refcount;
	const char *buf;
	int64_t len;
	int64_t read;
};

struct avro_writer_t_ {
	unsigned long refcount;
	const char *buf;
	int64_t len;
	int64_t written;
};

static inline
int avro_read_raw(avro_reader_t reader, void *buf, int64_t len)
{
	if ((reader->len - reader->read) < len) {
		return ENOSPC;
	}
	memcpy(buf, reader->buf + reader->read, len);
	reader->read += len;
	return 0;
}

static inline
int avro_skip_raw(avro_reader_t reader, int64_t len)
{
	if ((reader->len - reader->read) < len) {
		return ENOSPC;
	}
	reader->read += len;
	return 0;
}

static inline
int avro_write_raw(avro_writer_t writer, void *buf, int64_t len)
{
	if ((writer->len - writer->written) < len) {
		return ENOSPC;
	}
	memcpy((void *)(writer->buf + writer->written), buf, len);
	writer->written += len;
	return 0;
}

