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
#include <stdlib.h>
#include "dump.h"
#include "io.h"

avro_reader_t avro_reader_memory(const char *buf, int64_t len)
{
	avro_reader_t mem_reader =
	    malloc(sizeof(struct avro_reader_t_));
	if (!mem_reader) {
		return NULL;
	}
	mem_reader->refcount = 1;
	mem_reader->buf = buf;
	mem_reader->len = len;
	mem_reader->read = 0;
	return mem_reader;
}

avro_writer_t avro_writer_memory(const char *buf, int64_t len)
{
	avro_writer_t mem_writer =
	    malloc(sizeof(struct avro_writer_t_));
	if (!mem_writer) {
		return NULL;
	}
	mem_writer->refcount = 1;
	mem_writer->buf = buf;
	mem_writer->len = len;
	mem_writer->written = 0;
	return mem_writer;
}

int avro_read(avro_reader_t reader, void *buf, int64_t len)
{
	if (buf && len >= 0) {
		if (len > 0) {
			return avro_read_raw(reader, buf, len);
		}
		return 0;
	}
	return EINVAL;
}

int avro_skip(avro_reader_t reader, int64_t len)
{
	if (len >= 0) {
		return avro_skip_raw(reader, len);
	}
	return 0;
}

int avro_write(avro_writer_t writer, void *buf, int64_t len)
{
	if (buf && len >= 0) {
		if (len) {
			return avro_write_raw(writer, buf, len);
		}
		return 0;
	}
	return EINVAL;
}

void avro_writer_reset(avro_writer_t writer)
{
	writer->written = 0;
}

int64_t avro_writer_tell(avro_writer_t writer)
{
	return writer->written;
}

void avro_writer_dump(avro_writer_t writer, FILE * fp)
{
	dump(fp, (char *)writer->buf, writer->written);
}

void avro_reader_dump(avro_reader_t reader, FILE * fp)
{
	dump(fp, (char *)reader->buf, reader->read);
}

void avro_reader_free(avro_reader_t reader)
{
	free(reader);
}

void avro_writer_free(avro_writer_t writer)
{
	free(writer);
}

