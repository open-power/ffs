/*
 * Copyright (c) International Business Machines Corp., 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *   File: db.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: SQlite wrapper
 *   Note:
 *   Date: 02/25/11
 */

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "assert.h"
#include "err.h"

#include "db.h"

/* =======================================================================*/

int db_init(db_t * self, const char *path)
{
	assert(self != NULL);

	self->path = strdup(path);
	if (self->path == NULL) {
		ERRNO(errno);
		return -1;
	}

	self->db = NULL;

	return 0;
}

int db_delete(db_t * self)
{
	if (self != NULL) {
		db_close(self);
		if (self->path != NULL)
			free((char *)self->path);
		memset(self, 0, sizeof(*self));
	}

	return 0;
}

int db_execute(db_t * self, const char *fmt, ...)
{
	assert(self != NULL);

	va_list ap;
	va_start(ap, fmt);
	char *sql = NULL;
	vasprintf(&sql, fmt, ap);

	assert(sql != NULL);

	int rc = sqlite3_exec(self->db, sql, NULL, NULL, NULL);
	if (sql != NULL)
		free(sql);

	if (rc != DB_OK) {
		DBERR(self->db);
		return -1;
	}

	return 0;
}

int db_open(db_t * self, int flags)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->db != NULL)
		db_close(self);

	rc = sqlite3_open_v2(self->path, &self->db, flags, NULL);
	if (rc != DB_OK) {
		DBERR(self->db);
		return -1;
	}

	return rc;
}

int db_close(db_t * self)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->db != NULL) {
		/* finalize statements */
		rc = sqlite3_close(self->db);
	}

	return rc;
}

int db_register_function(db_t * self, const char *name, int argc, db_f func,
			 void *data)
{
	assert(self != NULL);

	int rc = DB_OK;

	rc = sqlite3_create_function(self->db, name, argc, SQLITE_ANY, data,
				     func, NULL, NULL);
	if (rc != 0) {
		DBERR(self->db);
		return -1;
	}

	return rc;
}

int db_begin(db_t * self, transaction_type_t type)
{
	assert(self != NULL);

	const char *tran_type = NULL;

	switch (type) {
	case tt_DEFERRED:
		tran_type = "DEFERRED";
		break;
	case tt_IMMEDIATE:
		tran_type = "IMMEDIATE";
		break;
	case tt_EXCLUSIVE:
		tran_type = "EXCLUSIVE";
		break;
	default:
		UNEXPECTED("'%d' invalid transaction type", type);
		return -1;
	}

	return db_execute(self, "BEGIN %s TRANSACTION", tran_type);
}

int db_commit(db_t * self)
{
	assert(self != NULL);
	return db_execute(self, "COMMIT TRANSACTION");
}

int db_rollback(db_t * self)
{
	assert(self != NULL);
	return db_execute(self, "ROLLBACK TRANSACTION");
}

/* =======================================================================*/

int statement_init(statement_t * self, db_t * db)
{
	assert(self != NULL);
	assert(db != NULL);

	self->db = db;
	self->stmt = NULL;

	return 0;
}

int statement_delete(statement_t * self)
{
	assert(self != NULL);

	if (self->stmt != NULL) {
		statement_finalize(self);
		self->stmt = NULL;
	}

	self->db = NULL;
	free(self);

	return 0;
}

int statement_prepare(statement_t * self, const char *sql)
{
	assert(self != NULL);
	assert(sql != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_finalize(self->stmt);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	rc = sqlite3_prepare_v2(self->db->db, sql, -1, &self->stmt, NULL);
	if (rc != DB_OK) {
		SQLERR(self->db->db, self->stmt);
		return -1;
	}

	return rc;
}

int statement_step(statement_t * self)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_step(self->stmt);

		switch (rc) {
		case DB_DONE:
		case DB_ROW:
			break;
		default:
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	return rc;
}

int statement_reset(statement_t * self)
{
	self = NULL;
	return 0;
}

int statement_finalize(statement_t * self)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_finalize(self->stmt);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
		self->stmt = NULL;
	}

	return rc;
}

int statement_bind_int(statement_t * self, int pos, int val)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_bind_int(self->stmt, pos, val);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	return rc;
}

int statement_bind_int64(statement_t * self, int pos, int64_t val)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_bind_int64(self->stmt, pos, val);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	return rc;
}

int statement_bind_text(statement_t * self, int pos, const char *val)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_bind_text(self->stmt, pos, val, -1,
				       SQLITE_TRANSIENT);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	return rc;
}

int statement_bind_blob(statement_t * self, int pos, const void *val, int size)
{
	assert(self != NULL);

	int rc = DB_OK;

	if (self->stmt != NULL) {
		rc = sqlite3_bind_blob(self->stmt, pos, val, size,
				       SQLITE_TRANSIENT);
		if (rc != DB_OK) {
			SQLERR(self->db->db, self->stmt);
			return -1;
		}
	}

	return rc;
}

int statement_column_int(statement_t * self, int pos)
{
	assert(self != NULL);
	return sqlite3_column_int(self->stmt, pos);
}

int64_t statement_column_int64(statement_t * self, int pos)
{
	assert(self != NULL);
	return sqlite3_column_int64(self->stmt, pos);
}

int statement_column_bytes(statement_t * self, int pos)
{
	assert(self != NULL);
	return sqlite3_column_bytes(self->stmt, pos);
}

const unsigned char *statement_column_text(statement_t * self, int pos)
{
	assert(self != NULL);

	const unsigned char *rc = NULL;

	if (self->stmt != NULL) {
		rc = sqlite3_column_text(self->stmt, pos);
		if (rc == NULL) {
			SQLERR(self->db->db, self->stmt);
			return NULL;
		}
	}

	return rc;
}

const void *statement_column_blob(statement_t * self, int pos)
{
	assert(self != NULL);

	const void *rc = NULL;

	if (self->stmt != NULL) {
		rc = sqlite3_column_blob(self->stmt, pos);
		if (rc == NULL) {
			SQLERR(self->db->db, self->stmt);
			return NULL;
		}
	}

	return rc;
}

/* =======================================================================*/
