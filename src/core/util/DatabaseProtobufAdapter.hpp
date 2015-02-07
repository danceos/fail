#ifndef __COMM_PROTOBUF_DATABASE_ADAPTER_H__
#define __COMM_PROTOBUF_DATABASE_ADAPTER_H__

#include <vector>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "DatabaseCampaignMessage.pb.h"
#include "util/Database.hpp"
#include "util/StringJoiner.hpp"

#include <sstream>

namespace fail {

class DatabaseProtobufAdapter {
	Database *db, *db_insert;
	MYSQL_STMT *stmt;
	std::stringstream insert_stmt;

	void error_create_table();

	/** \class TypeBridge
		A type bridge bridges the gap between a protobuf type and the
		sql database. It defines how the result table is defined, and
		how the message types are mapped onto SQL types. The whole
		message is mapped into a top level TypeBridge_message */
	struct TypeBridge {
		bool primary_key; // !< Should the field be put into the
						  // !< primary key
		const google::protobuf::FieldDescriptor *desc;

		virtual ~TypeBridge() {};

		TypeBridge(const google::protobuf::FieldDescriptor *desc);
		/* The field name in the SQL Table */
		virtual std::string name() { return desc->name(); }
		/* The expression in the create stmt for this type. */
		virtual std::string sql_create_stmt()
		{ return name() + " " + sql_type() + (desc->is_required() ? " NOT NULL": ""); };

		/* The mapped type */
		virtual std::string sql_type() = 0;
		virtual int	 element_size() { return 0; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *) { };

		/* A common function that handles NULL values for fields */
		bool insert_null(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
			const google::protobuf::Reflection *ref = msg->GetReflection();
			if (!ref->HasField(*msg, desc)) {
				bind->buffer_type = MYSQL_TYPE_NULL;
				return true; // handled
			}
			return false;
		}

		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) = 0;
	};

	struct TypeBridge_repeated : TypeBridge {
		TypeBridge *inner;
		char *buffer;
		TypeBridge_repeated(const google::protobuf::FieldDescriptor *desc, TypeBridge *inner)
			: TypeBridge(desc), inner(inner), buffer(0) {};
		virtual std::string sql_type() { return "blob"; };
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_int32 : TypeBridge {
		int32_t buffer;
		TypeBridge_int32(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};
		virtual std::string sql_type() { return "INT"; };
		virtual int element_size() { return 4; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_uint32 : TypeBridge {
		uint32_t buffer;
		TypeBridge_uint32(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};

		virtual std::string sql_type() { return "INT UNSIGNED"; };
		virtual int element_size() { return 4; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_int64 : TypeBridge {
		int64_t buffer;
		TypeBridge_int64(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};
		virtual std::string sql_type() { return "BIGINT"; };
		virtual int element_size() { return 8; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_uint64 : TypeBridge {
		uint64_t buffer;
		TypeBridge_uint64(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};

		virtual std::string sql_type() { return "BIGINT UNSIGNED"; };
		virtual int element_size() { return 8; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};
	struct TypeBridge_double : TypeBridge {
		double buffer;
		TypeBridge_double(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};

		virtual std::string sql_type() { return "DOUBLE"; };
		virtual int element_size() { return 8; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};
	struct TypeBridge_float : TypeBridge {
		float buffer;
		TypeBridge_float(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};

		virtual std::string sql_type() { return "FLOAT"; };
		virtual int element_size() { return 4; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};
	struct TypeBridge_bool : TypeBridge {
		bool buffer;
		TypeBridge_bool(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};

		virtual std::string sql_type() { return "TINYINT"; };
		virtual int element_size() { return 1; };
		virtual void copy_to(const google::protobuf::Message *msg, int i, void *);
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_string : TypeBridge {
		std::string buffer;
		TypeBridge_string(const google::protobuf::FieldDescriptor *desc)
			: TypeBridge(desc){};
		virtual std::string sql_type() { return "TEXT"; };
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_enum : TypeBridge {
		TypeBridge_enum(const google::protobuf::FieldDescriptor *desc) : TypeBridge(desc) {};
		virtual std::string sql_type();
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
	};

	struct TypeBridge_message : TypeBridge {
		const google::protobuf::Descriptor *msg_type;
		int nesting_level;
		int field_count;
		std::vector<TypeBridge *> types;
		std::vector<int> *selector;

		/* Pointer to the toplevel message */
		TypeBridge_message *parent;

		TypeBridge_message * top_level_msg() {
			TypeBridge_message *p = this;
			while (p->parent != 0) p = p->parent;
			return p;
		}

		std::vector<TypeBridge_message *> repeated_message_stack;

		TypeBridge_message(const google::protobuf::FieldDescriptor *desc,
						   const google::protobuf::Descriptor *msg_type,
						   TypeBridge_message *parent)
			: TypeBridge(desc), msg_type(msg_type), field_count(0), selector(0), parent(parent) {
			if (parent)
				nesting_level = parent->nesting_level+1;
			else
				nesting_level = 0;
		};
		virtual std::string sql_create_stmt();
		virtual std::string sql_type() { return ""; };
		virtual void bind(MYSQL_BIND *bind, const google::protobuf::Message *msg);
		/* Returns the number of enclosed fields */
		int gatherTypes(StringJoiner &insert_stmt, StringJoiner &primary_key);
	};

	TypeBridge_message top_level_msg;

	std::string result_table_name;

	int field_size_at_pos(const google::protobuf::Message *msg, std::vector<int> selector, int pos);


public:
	DatabaseProtobufAdapter() : db(0), db_insert(0), stmt(0), top_level_msg(0, 0, 0) {}
	void set_database_handle(Database *db)
	{
		this->db = db;
		if (!db_insert) {
			db_insert = db;
		}
	}
	/**
	 * Set a different database handle to be used in insert_row().  Necessary
	 * if INSERTs are done in a separate thread, while the handle set with
	 * set_database_handle() is still in use concurrently.
	 */
	void set_insert_database_handle(Database *db) { db_insert = db; }
	void create_table(const google::protobuf::Descriptor *);
	bool insert_row(const google::protobuf::Message *msg);
	std::string result_table() { return result_table_name; }

};

}

#endif

