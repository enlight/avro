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
#include <string.h>
#include <errno.h>
#include "datum.h"
#include "encoding.h"
#include "allocator.h"

#define DEFAULT_FIELD_COUNT 10
#define DEFAULT_TABLE_SIZE 32

static void avro_datum_init(avro_datum_t datum, avro_type_t type)
{
	datum->type = type;
	datum->class_type = AVRO_DATUM;
	datum->refcount = 1;
}

static avro_datum_t avro_string_private(char *str,
					void (*string_free) (void *ptr))
{
	struct avro_string_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_string_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->s = str;
	datum->free = string_free;

	avro_datum_init(&datum->obj, AVRO_STRING);
	return &datum->obj;
}

avro_datum_t avro_string(const char *str)
{
	char *p = avro_strdup(str);
	if (!p) {
		return NULL;
	}
	return avro_string_private(p, g_avro_allocator.free);
}

avro_datum_t avro_givestring(const char *str)
{
	return avro_string_private((char *)str, g_avro_allocator.free);
}

avro_datum_t avro_wrapstring(const char *str)
{
	return avro_string_private((char *)str, NULL);
}

int avro_string_get(avro_datum_t datum, char **p)
{
	if (!(is_avro_datum(datum) && is_avro_string(datum)) || !p) {
		return EINVAL;
	}
	*p = avro_datum_to_string(datum)->s;
	return 0;
}

static int avro_string_set_private(avro_datum_t datum, const char *p,
				   void (*string_free) (void *ptr))
{
	struct avro_string_datum_t *string;
	if (!(is_avro_datum(datum) && is_avro_string(datum)) || !p) {
		return EINVAL;
	}
	string = avro_datum_to_string(datum);
	if (string->free) {
		string->free(string->s);
	}
	string->free = string_free;
	string->s = (char *)p;
	return 0;
}

int avro_string_set(avro_datum_t datum, const char *p)
{
	char *string_copy = avro_strdup(p);
	int rval;
	if (!string_copy) {
		return ENOMEM;
	}
	rval = avro_string_set_private(datum, p, g_avro_allocator.free);
	if (rval) {
		g_avro_allocator.free(string_copy);
	}
	return rval;
}

int avro_givestring_set(avro_datum_t datum, const char *p)
{
	return avro_string_set_private(datum, p, g_avro_allocator.free);
}

int avro_wrapstring_set(avro_datum_t datum, const char *p)
{
	return avro_string_set_private(datum, p, NULL);
}

static avro_datum_t avro_bytes_private(char *bytes, int64_t size,
				       void (*bytes_free) (void *ptr))
{
	struct avro_bytes_datum_t *datum;
	datum = g_avro_allocator.malloc(sizeof(struct avro_bytes_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->bytes = bytes;
	datum->size = size;
	datum->free = bytes_free;

	avro_datum_init(&datum->obj, AVRO_BYTES);
	return &datum->obj;
}

avro_datum_t avro_bytes(const char *bytes, int64_t size)
{
	char *bytes_copy = g_avro_allocator.malloc(size);
	if (!bytes_copy) {
		return NULL;
	}
	memcpy(bytes_copy, bytes, size);
	return avro_bytes_private(bytes_copy, size, g_avro_allocator.free);
}

avro_datum_t avro_givebytes(const char *bytes, int64_t size)
{
	return avro_bytes_private((char *)bytes, size, g_avro_allocator.free);
}

avro_datum_t avro_wrapbytes(const char *bytes, int64_t size)
{
	return avro_bytes_private((char *)bytes, size, NULL);
}

static int avro_bytes_set_private(avro_datum_t datum, const char *bytes,
				  const int64_t size,
				  void (*bytes_free) (void *ptr))
{
	struct avro_bytes_datum_t *b;

	AVRO_UNUSED(size);

	if (!(is_avro_datum(datum) && is_avro_bytes(datum))) {
		return EINVAL;
	}

	b = avro_datum_to_bytes(datum);
	if (b->free) {
		b->free(b->bytes);
	}

	b->free = bytes_free;
	b->bytes = (char *)bytes;
	return 0;
}

int avro_bytes_set(avro_datum_t datum, const char *bytes, const int64_t size)
{
	int rval;
	char *bytes_copy = g_avro_allocator.malloc(size);
	if (!bytes_copy) {
		return ENOMEM;
	}
	memcpy(bytes_copy, bytes, size);
	rval = avro_bytes_set_private(datum, bytes, size, g_avro_allocator.free);
	if (rval) {
		g_avro_allocator.free(bytes_copy);
	}
	return rval;
}

int avro_givebytes_set(avro_datum_t datum, const char *bytes,
		       const int64_t size)
{
	return avro_bytes_set_private(datum, bytes, size, g_avro_allocator.free);
}

int avro_wrapbytes_set(avro_datum_t datum, const char *bytes,
		       const int64_t size)
{
	return avro_bytes_set_private(datum, bytes, size, NULL);
}

int avro_bytes_get(avro_datum_t datum, char **bytes, int64_t * size)
{
	if (!(is_avro_datum(datum) && is_avro_bytes(datum)) || !bytes || !size) {
		return EINVAL;
	}
	*bytes = avro_datum_to_bytes(datum)->bytes;
	*size = avro_datum_to_bytes(datum)->size;
	return 0;
}

avro_datum_t avro_int32(int32_t i)
{
	struct avro_int32_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_int32_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->i32 = i;

	avro_datum_init(&datum->obj, AVRO_INT32);
	return &datum->obj;
}

int avro_int32_get(avro_datum_t datum, int32_t * i)
{
	if (!(is_avro_datum(datum) && is_avro_int32(datum)) || !i) {
		return EINVAL;
	}
	*i = avro_datum_to_int32(datum)->i32;
	return 0;
}

int avro_int32_set(avro_datum_t datum, const int32_t i)
{
	struct avro_int32_datum_t *intp;
	if (!(is_avro_datum(datum) && is_avro_int32(datum))) {
		return EINVAL;
	}
	intp = avro_datum_to_int32(datum);
	intp->i32 = i;
	return 0;
}

avro_datum_t avro_int64(int64_t l)
{
	struct avro_int64_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_int64_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->i64 = l;

	avro_datum_init(&datum->obj, AVRO_INT64);
	return &datum->obj;
}

int avro_int64_get(avro_datum_t datum, int64_t * l)
{
	if (!(is_avro_datum(datum) && is_avro_int64(datum)) || !l) {
		return EINVAL;
	}
	*l = avro_datum_to_int64(datum)->i64;
	return 0;
}

int avro_int64_set(avro_datum_t datum, const int64_t l)
{
	struct avro_int64_datum_t *longp;
	if (!(is_avro_datum(datum) && is_avro_int64(datum))) {
		return EINVAL;
	}
	longp = avro_datum_to_int64(datum);
	longp->i64 = l;
	return 0;
}

avro_datum_t avro_float(float f)
{
	struct avro_float_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_float_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->f = f;

	avro_datum_init(&datum->obj, AVRO_FLOAT);
	return &datum->obj;
}

int avro_float_set(avro_datum_t datum, const float f)
{
	struct avro_float_datum_t *floatp;
	if (!(is_avro_datum(datum) && is_avro_float(datum))) {
		return EINVAL;
	}
	floatp = avro_datum_to_float(datum);
	floatp->f = f;
	return 0;
}

int avro_float_get(avro_datum_t datum, float *f)
{
	if (!(is_avro_datum(datum) && is_avro_float(datum)) || !f) {
		return EINVAL;
	}
	*f = avro_datum_to_float(datum)->f;
	return 0;
}

avro_datum_t avro_double(double d)
{
	struct avro_double_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_double_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->d = d;

	avro_datum_init(&datum->obj, AVRO_DOUBLE);
	return &datum->obj;
}

int avro_double_set(avro_datum_t datum, const double d)
{
	struct avro_double_datum_t *doublep;
	if (!(is_avro_datum(datum) && is_avro_double(datum))) {
		return EINVAL;
	}
	doublep = avro_datum_to_double(datum);
	doublep->d = d;
	return 0;
}

int avro_double_get(avro_datum_t datum, double *d)
{
	if (!(is_avro_datum(datum) && is_avro_double(datum)) || !d) {
		return EINVAL;
	}
	*d = avro_datum_to_double(datum)->d;
	return 0;
}

avro_datum_t avro_boolean(int8_t i)
{
	struct avro_boolean_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_boolean_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->i = i;
	avro_datum_init(&datum->obj, AVRO_BOOLEAN);
	return &datum->obj;
}

int avro_boolean_set(avro_datum_t datum, const int8_t i)
{
	struct avro_boolean_datum_t *booleanp;
	if (!(is_avro_datum(datum) && is_avro_boolean(datum))) {
		return EINVAL;
	}
	booleanp = avro_datum_to_boolean(datum);
	booleanp->i = i;
	return 0;
}

int avro_boolean_get(avro_datum_t datum, int8_t * i)
{
	if (!(is_avro_datum(datum) && is_avro_boolean(datum)) || !i) {
		return EINVAL;
	}
	*i = avro_datum_to_boolean(datum)->i;
	return 0;
}

avro_datum_t avro_null(void)
{
	static struct avro_obj_t obj = {
		AVRO_NULL,  // type
		AVRO_DATUM, // class_type
		1           // refcount
	};
	return &obj;
}

avro_datum_t avro_union(int64_t discriminant, avro_datum_t value)
{
	struct avro_union_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_union_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->discriminant = discriminant;
	datum->value = avro_datum_incref(value);

	avro_datum_init(&datum->obj, AVRO_UNION);
	return &datum->obj;
}

avro_datum_t avro_record(const char *name, const char *space)
{
	struct avro_record_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_record_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->name = avro_strdup(name);
	if (!datum->name) {
		g_avro_allocator.free(datum);
		return NULL;
	}
	datum->space = space ? avro_strdup(space) : NULL;
	if (space && !datum->space) {
		g_avro_allocator.free((void *)datum->name);
		g_avro_allocator.free((void *)datum);
		return NULL;
	}
	datum->field_order = (avro_atom_t*)g_avro_allocator.malloc(DEFAULT_FIELD_COUNT * sizeof(avro_atom_t));
	if (!datum->field_order) {
		if (space) {
			g_avro_allocator.free((void *)datum->space);
		}
		g_avro_allocator.free((char *)datum->name);
		g_avro_allocator.free(datum);
		return NULL;
	}
	datum->fields_byname = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->fields_byname) {
		g_avro_allocator.free(datum->field_order);
		if (space) {
			g_avro_allocator.free((void *)datum->space);
		}
		g_avro_allocator.free((char *)datum->name);
		g_avro_allocator.free(datum);
		return NULL;
	}
	datum->num_fields = 0;

	avro_datum_init(&datum->obj, AVRO_RECORD);
	return &datum->obj;
}

int
avro_record_get(const avro_datum_t datum, avro_atom_t field_name,
		avro_datum_t * field)
{
	union {
		avro_datum_t field;
		st_data_t data;
	} val;
	if (is_avro_datum(datum) && is_avro_record(datum)) {
		if (st_lookup
		    (avro_datum_to_record(datum)->fields_byname,
		     (st_data_t) field_name, &(val.data))) {
			*field = val.field;
			return 0;
		}
	}
	return EINVAL;
}

int
avro_record_set(const avro_datum_t datum, avro_atom_t field_name,
		const avro_datum_t field_value)
{
	avro_datum_t old_field;
	if (is_avro_datum(datum) && is_avro_record(datum)) {
		if (avro_record_get(datum, field_name, &old_field) == 0) {
			/* Overriding old value */
			avro_datum_decref(old_field);
		} else {
			/* Inserting new value */
			struct avro_record_datum_t *record =
			    avro_datum_to_record(datum);
			record->field_order = (avro_atom_t*)g_avro_allocator.realloc(record->field_order,
			    (record->num_fields + 1) * sizeof(avro_atom_t));
			avro_atom_incref(field_name);
			record->field_order[record->num_fields++] = field_name;
		}
		avro_datum_incref(field_value);
		avro_atom_incref(field_name);
		st_insert(avro_datum_to_record(datum)->fields_byname,
			  (st_data_t) field_name, (st_data_t) field_value);
		return 0;
	}
	return EINVAL;
}

avro_datum_t avro_enum(const char *name, int i)
{
	struct avro_enum_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_enum_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->name = avro_strdup(name);
	datum->value = i;

	avro_datum_init(&datum->obj, AVRO_ENUM);
	return &datum->obj;
}

static avro_datum_t avro_fixed_private(const char *name, const char *bytes,
				       const int64_t size,
				       void (*fixed_free) (void *ptr))
{
	struct avro_fixed_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_fixed_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->name = avro_strdup(name);
	datum->size = size;
	datum->bytes = (char *)bytes;
	datum->free = fixed_free;

	avro_datum_init(&datum->obj, AVRO_FIXED);
	return &datum->obj;
}

avro_datum_t avro_fixed(const char *name, const char *bytes, const int64_t size)
{
	char *bytes_copy = g_avro_allocator.malloc(size);
	if (!bytes_copy) {
		return NULL;
	}
	memcpy(bytes_copy, bytes, size);
	return avro_fixed_private(name, bytes, size, g_avro_allocator.free);
}

avro_datum_t avro_wrapfixed(const char *name, const char *bytes,
			    const int64_t size)
{
	return avro_fixed_private(name, bytes, size, NULL);
}

avro_datum_t avro_givefixed(const char *name, const char *bytes,
			    const int64_t size)
{
	return avro_fixed_private(name, bytes, size, g_avro_allocator.free);
}

static int avro_fixed_set_private(avro_datum_t datum, const char *bytes,
				  const int64_t size,
				  void (*fixed_free) (void *ptr))
{
	struct avro_fixed_datum_t *fixed;

	AVRO_UNUSED(size);

	if (!(is_avro_datum(datum) && is_avro_fixed(datum))) {
		return EINVAL;
	}

	fixed = avro_datum_to_fixed(datum);
	if (fixed->free) {
		fixed->free(fixed->bytes);
	}

	fixed->free = fixed_free;
	fixed->bytes = (char *)bytes;
	return 0;
}

int avro_fixed_set(avro_datum_t datum, const char *bytes, const int64_t size)
{
	int rval;
	char *bytes_copy = g_avro_allocator.malloc(size);
	if (!bytes_copy) {
		return ENOMEM;
	}
	memcpy(bytes_copy, bytes, size);
	rval = avro_fixed_set_private(datum, bytes, size, g_avro_allocator.free);
	if (rval) {
		g_avro_allocator.free(bytes_copy);
	}
	return rval;
}

int avro_givefixed_set(avro_datum_t datum, const char *bytes,
		       const int64_t size)
{
	return avro_fixed_set_private(datum, bytes, size, g_avro_allocator.free);
}

int avro_wrapfixed_set(avro_datum_t datum, const char *bytes,
		       const int64_t size)
{
	return avro_fixed_set_private(datum, bytes, size, NULL);
}

int avro_fixed_get(avro_datum_t datum, char **bytes, int64_t * size)
{
	if (!(is_avro_datum(datum) && is_avro_fixed(datum)) || !bytes || !size) {
		return EINVAL;
	}
	*bytes = avro_datum_to_fixed(datum)->bytes;
	*size = avro_datum_to_fixed(datum)->size;
	return 0;
}

avro_datum_t avro_map(void)
{
	struct avro_map_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_map_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->map = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->map) {
		g_avro_allocator.free(datum);
		return NULL;
	}

	avro_datum_init(&datum->obj, AVRO_MAP);
	return &datum->obj;
}

int
avro_map_get(const avro_datum_t datum, const char *key, avro_datum_t * value)
{
	struct avro_map_datum_t *map;
	union {
		avro_datum_t datum;
		st_data_t data;
	} val;

	if (!(is_avro_datum(datum) && is_avro_map(datum) && key && value)) {
		return EINVAL;
	}

	map = avro_datum_to_map(datum);
	if (st_lookup(map->map, (st_data_t) key, &(val.data))) {
		*value = val.datum;
		return 0;
	}
	return EINVAL;
}

int
avro_map_set(const avro_datum_t datum, const char *key,
	     const avro_datum_t value)
{
	char *save_key = (char *)key;
	avro_datum_t old_datum;

	if (!is_avro_datum(datum) || !is_avro_map(datum) || !key
	    || !is_avro_datum(value)) {
		return EINVAL;
	}

	if (avro_map_get(datum, key, &old_datum) == 0) {
		/* Overwriting an old value */
		avro_datum_decref(old_datum);
	} else {
		/* Inserting a new value */
		save_key = avro_strdup(key);
		if (!save_key) {
			return ENOMEM;
		}
	}
	avro_datum_incref(value);
	st_insert(avro_datum_to_map(datum)->map, (st_data_t) save_key,
		  (st_data_t) value);
	return 0;
}

avro_datum_t avro_array(void)
{
	struct avro_array_datum_t *datum =
	    g_avro_allocator.malloc(sizeof(struct avro_array_datum_t));
	if (!datum) {
		return NULL;
	}
	datum->els = g_avro_allocator.malloc(DEFAULT_TABLE_SIZE * sizeof(avro_datum_t));
	if (!datum->els) {
		g_avro_allocator.free(datum);
		return NULL;
	}
	datum->num_els = 0;

	avro_datum_init(&datum->obj, AVRO_ARRAY);
	return &datum->obj;
}

int
avro_array_get(const avro_datum_t array_datum, int64_t index, avro_datum_t * value)
{
	if (is_avro_datum(array_datum) && is_avro_array(array_datum) && (index >= 0)) {
        const struct avro_array_datum_t * array = avro_datum_to_array(array_datum);
		if ((index >= 0) && (index < array->num_els)) {
            *value = array->els[index];
            return 0;
		}
    }
    return EINVAL;
}

int
avro_array_append_datum(const avro_datum_t array_datum,
			const avro_datum_t datum)
{
	struct avro_array_datum_t *array;
	if (!is_avro_datum(array_datum) || !is_avro_array(array_datum)
	    || !is_avro_datum(datum)) {
		return EINVAL;
	}
	array = avro_datum_to_array(array_datum);
	array->els = (avro_datum_t *)g_avro_allocator.realloc(array->els,
			    (array->num_els + 1) * sizeof(avro_datum_t));
	array->els[array->num_els++] = avro_datum_incref(datum);
	return 0;
}

static int atom_datum_free_foreach(avro_atom_t key, avro_datum_t datum, void *arg)
{
	AVRO_UNUSED(arg);

	avro_datum_decref(datum);
	avro_atom_decref(key);
	return ST_DELETE;
}

static int char_datum_free_foreach(char *key, avro_datum_t datum, void *arg)
{
	AVRO_UNUSED(arg);

	avro_datum_decref(datum);
	g_avro_allocator.free(key);
	return ST_DELETE;
}

static void avro_datum_free(avro_datum_t datum)
{
	if (is_avro_datum(datum)) {
		switch (avro_typeof(datum)) {
		case AVRO_STRING:{
				struct avro_string_datum_t *string;
				string = avro_datum_to_string(datum);
				if (string->free) {
					string->free(string->s);
				}
				g_avro_allocator.free(string);
			}
			break;
		case AVRO_BYTES:{
				struct avro_bytes_datum_t *bytes;
				bytes = avro_datum_to_bytes(datum);
				if (bytes->free) {
					bytes->free(bytes->bytes);
				}
				g_avro_allocator.free(bytes);
			}
			break;
		case AVRO_INT32:{
				struct avro_int32_datum_t *i;
				i = avro_datum_to_int32(datum);
				g_avro_allocator.free(i);
			}
			break;
		case AVRO_INT64:{
				struct avro_int64_datum_t *l;
				l = avro_datum_to_int64(datum);
				g_avro_allocator.free(l);
			}
			break;
		case AVRO_FLOAT:{
				struct avro_float_datum_t *f;
				f = avro_datum_to_float(datum);
				g_avro_allocator.free(f);
			}
			break;
		case AVRO_DOUBLE:{
				struct avro_double_datum_t *d;
				d = avro_datum_to_double(datum);
				g_avro_allocator.free(d);
			}
			break;
		case AVRO_BOOLEAN:{
				struct avro_boolean_datum_t *b;
				b = avro_datum_to_boolean(datum);
				g_avro_allocator.free(b);
			}
			break;
		case AVRO_NULL:
			/* Nothing allocated */
			break;

		case AVRO_RECORD:{
				int i;
				struct avro_record_datum_t *record;
				record = avro_datum_to_record(datum);
				g_avro_allocator.free((void *)record->name);
				if (record->space) {
					g_avro_allocator.free((void *)record->space);
				}
				st_foreach(record->fields_byname,
					   atom_datum_free_foreach, 0);
				for (i = 0; i < record->num_fields; i++) {
					avro_atom_decref(record->field_order[i]);
				}
				g_avro_allocator.free(record->field_order);
				st_free_table(record->fields_byname);
				g_avro_allocator.free(record);
			}
			break;
		case AVRO_ENUM:{
				struct avro_enum_datum_t *enump;
				enump = avro_datum_to_enum(datum);
				g_avro_allocator.free((void *)enump->name);
				g_avro_allocator.free(enump);
			}
			break;
		case AVRO_FIXED:{
				struct avro_fixed_datum_t *fixed;
				fixed = avro_datum_to_fixed(datum);
				g_avro_allocator.free((void *)fixed->name);
				if (fixed->free) {
					fixed->free((void *)fixed->bytes);
				}
				g_avro_allocator.free(fixed);
			}
			break;
		case AVRO_MAP:{
				struct avro_map_datum_t *map;
				map = avro_datum_to_map(datum);
				st_foreach(map->map, char_datum_free_foreach,
					   0);
				st_free_table(map->map);
				g_avro_allocator.free(map);
			}
			break;
		case AVRO_ARRAY:{
				int i;
				struct avro_array_datum_t *array;
				array = avro_datum_to_array(datum);
				for (i = 0; i < array->num_els; i++) {
					avro_datum_decref(array->els[i]);
				}
				g_avro_allocator.free(array->els);
				g_avro_allocator.free(array);
			}
			break;
		case AVRO_UNION:{
				struct avro_union_datum_t *unionp;
				unionp = avro_datum_to_union(datum);
				avro_datum_decref(unionp->value);
				g_avro_allocator.free(unionp);
			}
			break;
		case AVRO_LINK:{
				/* TODO */
			}
			break;
		}
	}
}

avro_datum_t avro_datum_incref(avro_datum_t datum)
{
	if (datum && datum->refcount != (unsigned int)-1) {
		++datum->refcount;
	}
	return datum;
}

void avro_datum_decref(avro_datum_t datum)
{
	if (datum && datum->refcount != (unsigned int)-1
	    && --datum->refcount == 0) {
		avro_datum_free(datum);
	}
}

void avro_datum_print(avro_datum_t value, FILE * fp)
{
	AVRO_UNUSED(value);
	AVRO_UNUSED(fp);
}
