#include <iostream>
#include <sstream>
#include <assert.h>
#include "DatabaseProtobufAdapter.hpp"
#include "util/Logger.hpp"
#include "util/StringJoiner.hpp"

static fail::Logger LOG("DatabaseProtobufAdapter", true);


using namespace fail;
using namespace google::protobuf;

DatabaseProtobufAdapter::TypeBridge::TypeBridge(const FieldDescriptor *desc)
	: desc(desc) {
	/* We get the value of the field option extension in_primary_key
	   to determine which fields should be added additionally to the
	   primary key, besides pilot_id */
	if (desc) { // top-level type bridge has no desc pointer
		const FieldOptions& field_options = desc->options();
		this->primary_key = field_options.GetExtension(sql_primary_key);
	} else { // initialize top-level type bridge, too
		primary_key = false;
	}
}


std::string DatabaseProtobufAdapter::TypeBridge_enum::sql_type() {
	/* enum types are mapped onto SQL enum types, therefore we have to
	   gather all the stringified names for the enum, values */
	const google::protobuf::EnumDescriptor *e = desc->enum_type();
	std::stringstream ss;
	ss << "ENUM (";
	for (int i = 0; i < e->value_count(); i++) {
		ss << "'" << e->value(i)->name() << "'";
		if (i != e->value_count() - 1)
			ss << ", ";
	}
	   ss << ")";
	return ss.str();
}

std::string DatabaseProtobufAdapter::TypeBridge_message::sql_create_stmt() {
	/* The create statement for a message TypeBridge just propagates
	   the call onto the enclosed types (including inner
	   TypeBridge_message objects */
	std::stringstream ss;
	for (std::vector<TypeBridge *>::iterator it = types.begin();
		 it != types.end(); ++it) {
		TypeBridge *bridge = *it;
		ss << bridge->sql_create_stmt();
		if (it+1 != types.end())
			ss << ", ";
	}
	return ss.str();
}


void DatabaseProtobufAdapter::TypeBridge_int64::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_LONGLONG;
	bind->is_unsigned = 0;
	buffer = ref->GetInt64(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_uint64::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_LONGLONG;
	bind->is_unsigned = 1;
	buffer = ref->GetUInt64(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_int32::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_LONG;
	bind->is_unsigned = 0;
	buffer = ref->GetInt32(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_uint32::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_LONG;
	bind->is_unsigned = 1;
	buffer = ref->GetUInt32(*msg, desc);
	bind->buffer = &buffer;
}
void DatabaseProtobufAdapter::TypeBridge_float::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_FLOAT;
	buffer = ref->GetFloat(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_double::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_DOUBLE;
	buffer = ref->GetDouble(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_bool::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_TINY;
	bind->is_unsigned = 1;
	buffer = ref->GetBool(*msg, desc);
	bind->buffer = &buffer;
}

void DatabaseProtobufAdapter::TypeBridge_string::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	buffer = ref->GetString(*msg, desc);

	bind->buffer_type = MYSQL_TYPE_STRING;
	bind->buffer = (void *) buffer.c_str();
	bind->buffer_length = buffer.length();
}
void DatabaseProtobufAdapter::TypeBridge_enum::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	/* Handle the NULL case */
	if (insert_null(bind, msg)) return;

	bind->buffer_type = MYSQL_TYPE_STRING;
	bind->buffer = (void *) ref->GetEnum(*msg, desc)->name().c_str();
	bind->buffer_length = ref->GetEnum(*msg, desc)->name().length();
}

void DatabaseProtobufAdapter::TypeBridge_message::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	std::stringstream ss;
	for (std::vector<TypeBridge *>::iterator it = types.begin();
		 it != types.end(); ++it) {
		TypeBridge *bridge = *it;
		TypeBridge_message *msg_bridge = dynamic_cast<TypeBridge_message *> (bridge);
		if (msg_bridge != 0) {
			const Message *inner_msg = 0;
			if (msg_bridge->desc->is_repeated()) {
				std::vector<int> &selector = *top_level_msg()->selector;
				inner_msg = &ref->GetRepeatedMessage(*msg, msg_bridge->desc, selector[nesting_level+1]);
			} else {
				inner_msg = &ref->GetMessage(*msg, msg_bridge->desc);
			}
			msg_bridge->bind(bind, inner_msg);
			/* Increment bind pointer */
			bind = &bind[msg_bridge->field_count];
		} else {
			/* Bind the plain field and increment the binding pointer */
			bridge->bind(bind, msg);
			bind ++;
		}
	}
}

void DatabaseProtobufAdapter::TypeBridge_repeated::bind(MYSQL_BIND *bind, const google::protobuf::Message *msg) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	size_t count = ref->FieldSize(*msg,desc);
	if (count == 0) {
		bind->buffer_type = MYSQL_TYPE_NULL;
		return;
	}

	if (buffer) delete[] buffer;
	buffer = new char[count * inner->element_size()];
	assert (inner->element_size() > 0);

	char *p = buffer;
	for (unsigned i = 0; i < count; i++) {
		inner->copy_to(msg, i, p);
		p += inner->element_size();
	}

	bind->buffer_type = MYSQL_TYPE_BLOB;
	bind->buffer = buffer;
	bind->buffer_length = count * inner->element_size();
}

void DatabaseProtobufAdapter::TypeBridge_int32::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(int32_t *) p = ref->GetRepeatedInt32(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_uint32::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(uint32_t *) p = ref->GetRepeatedUInt32(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_int64::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(int64_t *) p = ref->GetRepeatedInt64(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_uint64::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(uint64_t *) p = ref->GetRepeatedUInt64(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_double::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(double *) p = ref->GetRepeatedDouble(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_float::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(float *) p = ref->GetRepeatedFloat(*msg, desc, i);
}
void DatabaseProtobufAdapter::TypeBridge_bool::copy_to(const google::protobuf::Message *msg, int i, void *p) {
	const google::protobuf::Reflection *ref = msg->GetReflection();
	*(char *) p = ref->GetRepeatedBool(*msg, desc, i) ? '1' : '0';
}

void DatabaseProtobufAdapter::error_create_table() {
	std::cerr << "ERROR: Cannot create the result table from message description" << std::endl;
	std::cerr << "       The form only from of messages is: [required DatabaseCampaignMessage, repeated group X {}]" << std::endl;
	exit(-1);
}

int DatabaseProtobufAdapter::TypeBridge_message::gatherTypes(StringJoiner &insert_stmt, StringJoiner &primary_key) {
	/* Clear old state */
	repeated_message_stack.clear();
	repeated_message_stack.push_back(this);
	for (std::vector<TypeBridge *>::iterator it = types.begin(); it != types.end(); ++it)
		delete *it;
	types.clear();

	size_t count = msg_type->field_count();
	field_count = 0;

	for (unsigned i = 0; i < count; i++) {
		const FieldDescriptor *field = msg_type->field(i);
		assert(field != 0);

		TypeBridge *bridge = 0;
		TypeBridge_message *msg_bridge;
		bool can_be_repeated = true; // default value

		// For repeated messages
		TypeBridge_message *top_level_msg = 0;

		const FieldOptions& field_options = field->options();
		if (field_options.GetExtension(sql_ignore)) {
			// Field should be ignored
			continue;
		}

		switch (field->cpp_type()) {
		case FieldDescriptor::CPPTYPE_INT32:
			bridge = new TypeBridge_int32(field);
			break;
		case FieldDescriptor::CPPTYPE_UINT32:
			bridge = new TypeBridge_uint32(field);
			break;
		case FieldDescriptor::CPPTYPE_INT64:
			bridge = new TypeBridge_int64(field);
			break;
		case FieldDescriptor::CPPTYPE_UINT64:
			bridge = new TypeBridge_uint64(field);
			break;
		case FieldDescriptor::CPPTYPE_DOUBLE:
			bridge = new TypeBridge_double(field);
			break;
		case FieldDescriptor::CPPTYPE_FLOAT:
			bridge = new TypeBridge_float(field);
			break;
		case FieldDescriptor::CPPTYPE_BOOL:
			bridge = new TypeBridge_bool(field);
			break;
		case FieldDescriptor::CPPTYPE_ENUM:
			can_be_repeated = false;
			bridge = new TypeBridge_enum(field);
			break;
		case FieldDescriptor::CPPTYPE_STRING:
			can_be_repeated = false;
			bridge = new TypeBridge_string(field);
			break;
		case FieldDescriptor::CPPTYPE_MESSAGE:
			if (field->is_repeated()) {
				top_level_msg = this->top_level_msg();
				/* Here we check wether we are on the repeated path
				   from the root, when we are a repeated entry is ok.
				*/
				if (!(this == top_level_msg->repeated_message_stack.back())) {
					LOG << "Cannot handle repeated inner message in two different paths: " << field->name() << std::endl;
					exit(0);
				}
			}

			if (field->is_optional()) {
				LOG << "Cannot handle optional inner message: " << field->name() << std::endl;
				exit(-1);
			}

			msg_bridge = new TypeBridge_message(field, field->message_type(), this);
			bridge = msg_bridge;
			types.push_back(bridge);
			if (field->is_repeated()) {
				while (1) {
					TypeBridge_message *back = top_level_msg->repeated_message_stack.back();
					if (back == msg_bridge)
						break;
					TypeBridge_message *p = msg_bridge;
					while (back != 0 && p->parent != back && p->parent != 0) {
						p = p->parent;
					}
					top_level_msg->repeated_message_stack.push_back(p);
				}
			}

			field_count += msg_bridge->gatherTypes(insert_stmt, primary_key);

			/* Do not the normal field adding process */
			continue;

		default:
			std::cerr << "unsupported field: " << field->name() << std::endl;
			exit(-1);
			break;
		}

		if (field->is_repeated() && ! can_be_repeated) {
			LOG << "Cannot handle repeated field: " << field->name() << std::endl;
			exit(-1);
		}
		if (field->is_repeated()) {
			bridge = new TypeBridge_repeated(field, bridge);
		}

		types.push_back(bridge);
		field_count ++;
		insert_stmt.push_back(field->name());
		if (bridge->primary_key)
			primary_key.push_back(field->name());
	}

	return field_count;
}


void DatabaseProtobufAdapter::create_table(const Descriptor *toplevel_desc) {
	assert (toplevel_desc != 0);

	std::stringstream create_table_stmt;
	StringJoiner insert_join, primary_join, question_marks;

	insert_stmt.str("");

	result_table_name = "result_" + toplevel_desc->name();

	/* Fill our top level dummy type bridge with the fields from the
	   example message */
	top_level_msg.msg_type = toplevel_desc;
	int fields = top_level_msg.gatherTypes(insert_join, primary_join);
	for (int i = 0; i < fields; i++)
		question_marks.push_back("?");

	create_table_stmt << "CREATE TABLE IF NOT EXISTS " << result_table_name << "(";
	create_table_stmt << top_level_msg.sql_create_stmt() << ", PRIMARY KEY(" << primary_join.join(", ") << "))";
	create_table_stmt << " ENGINE=MyISAM";

	insert_stmt << "INSERT INTO " << result_table_name << "(" << insert_join.join(",");
	insert_stmt << ") VALUES (" << question_marks.join(",") << ")";

	// Create the Table
	db->query(create_table_stmt.str().c_str());
}



int DatabaseProtobufAdapter::field_size_at_pos(const Message *msg, std::vector<int> selector, int pos) {
	std::vector< TypeBridge_message *>::iterator it;
	if (top_level_msg.repeated_message_stack.size() <= 1)
		return 1;

	int i = 1;
	for (it = top_level_msg.repeated_message_stack.begin() + 1;
		 i != pos && it != top_level_msg.repeated_message_stack.end(); ++it) {

		TypeBridge_message *bridge = *it;
		const Reflection *ref = msg->GetReflection();
		if (bridge->desc && bridge->desc->is_repeated()) {
			msg = &ref->GetRepeatedMessage(*msg, bridge->desc, selector[i]);
		} else {
			assert(selector[i] == 0);
			msg = &ref->GetMessage(*msg, bridge->desc);
		}
		i++;
	}
	const Reflection *ref = msg->GetReflection();
	return ref->FieldSize(*msg, top_level_msg.repeated_message_stack[i]->desc);
}

bool DatabaseProtobufAdapter::insert_row(const google::protobuf::Message *msg) {
	assert (msg->GetDescriptor() != 0 && msg->GetReflection() != 0);

	if (!stmt) {
		// Prepare the insert statement
		// We didn't do that right in create_table() because we need to use the
		// right DB connection for that (which may not have existed yet then).
		stmt = mysql_stmt_init(db_insert->getHandle());
		if (mysql_stmt_prepare(stmt, insert_stmt.str().c_str(), insert_stmt.str().length())) {
			LOG << "query '" << insert_stmt.str() << "' failed: " << mysql_error(db_insert->getHandle()) << std::endl;
			exit(-1);
		}
	}

	MYSQL_BIND *bind = new MYSQL_BIND[top_level_msg.field_count];

	/* We determine how many columns should be produced */
	std::vector<int> selector    (top_level_msg.repeated_message_stack.size());

	while (true) {
		// INSERT WITH SELECTOR
		top_level_msg.selector = &selector;

		// Use the top_level_msg TypeBridge to bind all parameters
		// into the MYSQL_BIND structure
		memset(bind, 0, sizeof(*bind) * (top_level_msg.field_count));
		top_level_msg.bind(bind, msg);

		// Insert the binded row
		if (mysql_stmt_bind_param(stmt, bind)) {
			LOG << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
			delete[] bind;
			return false;
		}

		if (mysql_stmt_execute(stmt)) {
			LOG << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
		}


		/* Increment the selector */
		unsigned i = selector.size() - 1;
		selector[i] ++;

		while (i > 0 && field_size_at_pos(msg, selector, i) <= selector[i]) {
			selector[i] = 0;
			i--;
			selector[i] ++;
		}
		if (i == 0) break;
	}

	delete[] bind;

	return true;

}
