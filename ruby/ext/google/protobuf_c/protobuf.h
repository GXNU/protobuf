// Protocol Buffers - Google's data interchange format
// Copyright 2014 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__
#define __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__

#include <ruby/ruby.h>
#include <ruby/vm.h>
#include <ruby/encoding.h>

#include "upb.h"

// Forward decls.
struct DescriptorPool;
struct Descriptor;
struct FieldDescriptor;
struct EnumDescriptor;
struct MessageLayout;
struct MessageHeader;
struct MessageBuilderContext;
struct EnumBuilderContext;
struct Builder;

typedef struct DescriptorPool DescriptorPool;
typedef struct Descriptor Descriptor;
typedef struct FieldDescriptor FieldDescriptor;
typedef struct EnumDescriptor EnumDescriptor;
typedef struct MessageLayout MessageLayout;
typedef struct MessageHeader MessageHeader;
typedef struct MessageBuilderContext MessageBuilderContext;
typedef struct EnumBuilderContext EnumBuilderContext;
typedef struct Builder Builder;

/*
 It can be a bit confusing how the C structs defined below and the Ruby
 objects interact and hold references to each other. First, a few principles:

 - Ruby's "TypedData" abstraction lets a Ruby VALUE hold a pointer to a C
   struct (or arbitrary memory chunk), own it, and free it when collected.
   Thus, each struct below will have a corresponding Ruby object
   wrapping/owning it.

 - To get back from an underlying upb {msg,enum}def to the Ruby object, we
   keep a global hashmap, accessed by get_def_obj/add_def_obj below.

 The in-memory structure is then something like:

   Ruby                        |      upb
                               |
   DescriptorPool  ------------|-----------> upb_symtab____________________
                               |                | (message types)          \
                               |                v                           \
   Descriptor   ---------------|-----------> upb_msgdef         (enum types)|
    |--> msgclass              |                |   ^                       |
    |    (dynamically built)   |                |   | (submsg fields)       |
    |--> MessageLayout         |                |   |                       /
    |--------------------------|> decoder method|   |                      /
    \--------------------------|> serialize     |   |                     /
                               |  handlers      v   |                    /
   FieldDescriptor  -----------|-----------> upb_fielddef               /
                               |                    |                  /
                               |                    v (enum fields)   /
   EnumDescriptor  ------------|-----------> upb_enumdef  <----------'
                               |
                               |
               ^               |               \___/
               `---------------|-----------------'    (get_def_obj map)
 */

// -----------------------------------------------------------------------------
// Ruby class structure definitions.
// -----------------------------------------------------------------------------

struct DescriptorPool {
  upb_symtab* symtab;
};

struct Descriptor {
  const upb_msgdef* msgdef;
  MessageLayout* layout;
  VALUE klass;  // begins as nil
  const upb_handlers* fill_handlers;
  const upb_pbdecodermethod* fill_method;
  const upb_handlers* pb_serialize_handlers;
  const upb_handlers* json_serialize_handlers;
};

struct FieldDescriptor {
  const upb_fielddef* fielddef;
};

struct EnumDescriptor {
  const upb_enumdef* enumdef;
  VALUE module;  // begins as nil
};

struct MessageBuilderContext {
  VALUE descriptor;
};

struct EnumBuilderContext {
  VALUE enumdesc;
};

struct Builder {
  VALUE pending_list;
  upb_def** defs;  // used only while finalizing
};

extern VALUE cDescriptorPool;
extern VALUE cDescriptor;
extern VALUE cFieldDescriptor;
extern VALUE cEnumDescriptor;
extern VALUE cMessageBuilderContext;
extern VALUE cEnumBuilderContext;
extern VALUE cBuilder;

extern const char* kDescriptorInstanceVar;

// We forward-declare all of the Ruby method implementations here because we
// sometimes call the methods directly across .c files, rather than going
// through Ruby's method dispatching (e.g. during message parse). It's cleaner
// to keep the list of object methods together than to split them between
// static-in-file definitions and header declarations.

void DescriptorPool_mark(void* _self);
void DescriptorPool_free(void* _self);
VALUE DescriptorPool_alloc(VALUE klass);
void DescriptorPool_register(VALUE module);
DescriptorPool* ruby_to_DescriptorPool(VALUE value);
VALUE DescriptorPool_add(VALUE _self, VALUE def);
VALUE DescriptorPool_build(VALUE _self);
VALUE DescriptorPool_lookup(VALUE _self, VALUE name);
VALUE DescriptorPool_generated_pool(VALUE _self);

void Descriptor_mark(void* _self);
void Descriptor_free(void* _self);
VALUE Descriptor_alloc(VALUE klass);
void Descriptor_register(VALUE module);
Descriptor* ruby_to_Descriptor(VALUE value);
VALUE Descriptor_name(VALUE _self);
VALUE Descriptor_name_set(VALUE _self, VALUE str);
VALUE Descriptor_each(VALUE _self);
VALUE Descriptor_lookup(VALUE _self, VALUE name);
VALUE Descriptor_add_field(VALUE _self, VALUE obj);
VALUE Descriptor_msgclass(VALUE _self);
extern const rb_data_type_t _Descriptor_type;

void FieldDescriptor_mark(void* _self);
void FieldDescriptor_free(void* _self);
VALUE FieldDescriptor_alloc(VALUE klass);
void FieldDescriptor_register(VALUE module);
FieldDescriptor* ruby_to_FieldDescriptor(VALUE value);
VALUE FieldDescriptor_name(VALUE _self);
VALUE FieldDescriptor_name_set(VALUE _self, VALUE str);
VALUE FieldDescriptor_type(VALUE _self);
VALUE FieldDescriptor_type_set(VALUE _self, VALUE type);
VALUE FieldDescriptor_label(VALUE _self);
VALUE FieldDescriptor_label_set(VALUE _self, VALUE label);
VALUE FieldDescriptor_number(VALUE _self);
VALUE FieldDescriptor_number_set(VALUE _self, VALUE number);
VALUE FieldDescriptor_submsg_name(VALUE _self);
VALUE FieldDescriptor_submsg_name_set(VALUE _self, VALUE value);
VALUE FieldDescriptor_subtype(VALUE _self);
VALUE FieldDescriptor_get(VALUE _self, VALUE msg_rb);
VALUE FieldDescriptor_set(VALUE _self, VALUE msg_rb, VALUE value);
upb_fieldtype_t ruby_to_fieldtype(VALUE type);
VALUE fieldtype_to_ruby(upb_fieldtype_t type);

void EnumDescriptor_mark(void* _self);
void EnumDescriptor_free(void* _self);
VALUE EnumDescriptor_alloc(VALUE klass);
void EnumDescriptor_register(VALUE module);
EnumDescriptor* ruby_to_EnumDescriptor(VALUE value);
VALUE EnumDescriptor_name(VALUE _self);
VALUE EnumDescriptor_name_set(VALUE _self, VALUE str);
VALUE EnumDescriptor_add_value(VALUE _self, VALUE name, VALUE number);
VALUE EnumDescriptor_lookup_name(VALUE _self, VALUE name);
VALUE EnumDescriptor_lookup_value(VALUE _self, VALUE number);
VALUE EnumDescriptor_each(VALUE _self);
VALUE EnumDescriptor_enummodule(VALUE _self);
extern const rb_data_type_t _EnumDescriptor_type;

void MessageBuilderContext_mark(void* _self);
void MessageBuilderContext_free(void* _self);
VALUE MessageBuilderContext_alloc(VALUE klass);
void MessageBuilderContext_register(VALUE module);
MessageBuilderContext* ruby_to_MessageBuilderContext(VALUE value);
VALUE MessageBuilderContext_initialize(VALUE _self, VALUE descriptor);
VALUE MessageBuilderContext_optional(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_required(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_repeated(int argc, VALUE* argv, VALUE _self);

void EnumBuilderContext_mark(void* _self);
void EnumBuilderContext_free(void* _self);
VALUE EnumBuilderContext_alloc(VALUE klass);
void EnumBuilderContext_register(VALUE module);
EnumBuilderContext* ruby_to_EnumBuilderContext(VALUE value);
VALUE EnumBuilderContext_initialize(VALUE _self, VALUE enumdesc);
VALUE EnumBuilderContext_value(VALUE _self, VALUE name, VALUE number);

void Builder_mark(void* _self);
void Builder_free(void* _self);
VALUE Builder_alloc(VALUE klass);
void Builder_register(VALUE module);
Builder* ruby_to_Builder(VALUE value);
VALUE Builder_add_message(VALUE _self, VALUE name);
VALUE Builder_add_enum(VALUE _self, VALUE name);
VALUE Builder_finalize_to_pool(VALUE _self, VALUE pool_rb);

// -----------------------------------------------------------------------------
// Native slot storage abstraction.
// -----------------------------------------------------------------------------

size_t native_slot_size(upb_fieldtype_t type);
void native_slot_set(upb_fieldtype_t type,
                     VALUE type_class,
                     void* memory,
                     VALUE value);
VALUE native_slot_get(upb_fieldtype_t type,
                      VALUE type_class,
                      void* memory);
void native_slot_init(upb_fieldtype_t type, void* memory);
void native_slot_mark(upb_fieldtype_t type, void* memory);
void native_slot_dup(upb_fieldtype_t type, void* to, void* from);
void native_slot_deep_copy(upb_fieldtype_t type, void* to, void* from);
bool native_slot_eq(upb_fieldtype_t type, void* mem1, void* mem2);

void native_slot_validate_string_encoding(upb_fieldtype_t type, VALUE value);

extern rb_encoding* kRubyStringUtf8Encoding;
extern rb_encoding* kRubyStringASCIIEncoding;
extern rb_encoding* kRubyString8bitEncoding;

// -----------------------------------------------------------------------------
// Repeated field container type.
// -----------------------------------------------------------------------------

typedef struct {
  upb_fieldtype_t field_type;
  VALUE field_type_class;
  void* elements;
  int size;
  int capacity;
} RepeatedField;

void RepeatedField_mark(void* self);
void RepeatedField_free(void* self);
VALUE RepeatedField_alloc(VALUE klass);
VALUE RepeatedField_init(int argc, VALUE* argv, VALUE self);
void RepeatedField_register(VALUE module);

extern const rb_data_type_t RepeatedField_type;
extern VALUE cRepeatedField;

RepeatedField* ruby_to_RepeatedField(VALUE value);

void RepeatedField_register(VALUE module);
VALUE RepeatedField_each(VALUE _self);
VALUE RepeatedField_index(VALUE _self, VALUE _index);
void* RepeatedField_index_native(VALUE _self, int index);
VALUE RepeatedField_index_set(VALUE _self, VALUE _index, VALUE val);
void RepeatedField_reserve(RepeatedField* self, int new_size);
VALUE RepeatedField_push(VALUE _self, VALUE val);
void RepeatedField_push_native(VALUE _self, void* data);
VALUE RepeatedField_pop(VALUE _self);
VALUE RepeatedField_insert(int argc, VALUE* argv, VALUE _self);
VALUE RepeatedField_replace(VALUE _self, VALUE list);
VALUE RepeatedField_clear(VALUE _self);
VALUE RepeatedField_length(VALUE _self);
VALUE RepeatedField_dup(VALUE _self);
VALUE RepeatedField_deep_copy(VALUE _self);
VALUE RepeatedField_eq(VALUE _self, VALUE _other);
VALUE RepeatedField_hash(VALUE _self);
VALUE RepeatedField_inspect(VALUE _self);
VALUE RepeatedField_plus(VALUE _self, VALUE list);

// -----------------------------------------------------------------------------
// Message layout / storage.
// -----------------------------------------------------------------------------

struct MessageLayout {
  const upb_msgdef* msgdef;
  size_t* offsets;
  size_t size;
};

MessageLayout* create_layout(const upb_msgdef* msgdef);
void free_layout(MessageLayout* layout);
VALUE layout_get(MessageLayout* layout,
                 void* storage,
                 const upb_fielddef* field);
void layout_set(MessageLayout* layout,
                void* storage,
                const upb_fielddef* field,
                VALUE val);
void layout_init(MessageLayout* layout, void* storage);
void layout_mark(MessageLayout* layout, void* storage);
void layout_dup(MessageLayout* layout, void* to, void* from);
void layout_deep_copy(MessageLayout* layout, void* to, void* from);
VALUE layout_eq(MessageLayout* layout, void* msg1, void* msg2);
VALUE layout_hash(MessageLayout* layout, void* storage);
VALUE layout_inspect(MessageLayout* layout, void* storage);

// -----------------------------------------------------------------------------
// Message class creation.
// -----------------------------------------------------------------------------

struct MessageHeader {
  Descriptor* descriptor;  // kept alive by self.class.descriptor reference.
  // Data comes after this.
};

extern rb_data_type_t Message_type;

VALUE build_class_from_descriptor(Descriptor* descriptor);
void* Message_data(void* msg);
void Message_mark(void* self);
void Message_free(void* self);
VALUE Message_alloc(VALUE klass);
VALUE Message_method_missing(int argc, VALUE* argv, VALUE _self);
VALUE Message_initialize(int argc, VALUE* argv, VALUE _self);
VALUE Message_dup(VALUE _self);
VALUE Message_deep_copy(VALUE _self);
VALUE Message_eq(VALUE _self, VALUE _other);
VALUE Message_hash(VALUE _self);
VALUE Message_inspect(VALUE _self);
VALUE Message_index(VALUE _self, VALUE field_name);
VALUE Message_index_set(VALUE _self, VALUE field_name, VALUE value);
VALUE Message_descriptor(VALUE klass);
VALUE Message_decode(VALUE klass, VALUE data);
VALUE Message_encode(VALUE klass, VALUE msg_rb);
VALUE Message_decode_json(VALUE klass, VALUE data);
VALUE Message_encode_json(VALUE klass, VALUE msg_rb);

VALUE Google_Protobuf_encode(VALUE self, VALUE msg_rb);
VALUE Google_Protobuf_decode(VALUE self, VALUE klass, VALUE msg_rb);
VALUE Google_Protobuf_encode_json(VALUE self, VALUE msg_rb);
VALUE Google_Protobuf_decode_json(VALUE self, VALUE klass, VALUE msg_rb);

VALUE Google_Protobuf_deep_copy(VALUE self, VALUE obj);

VALUE build_module_from_enumdesc(EnumDescriptor* enumdef);
VALUE enum_lookup(VALUE self, VALUE number);
VALUE enum_resolve(VALUE self, VALUE sym);

const upb_pbdecodermethod *new_fillmsg_decodermethod(
    Descriptor* descriptor, const void *owner);

// -----------------------------------------------------------------------------
// Global map from upb {msg,enum}defs to wrapper Descriptor/EnumDescriptor
// instances.
// -----------------------------------------------------------------------------
void add_def_obj(const void* def, VALUE value);
VALUE get_def_obj(const void* def);

// -----------------------------------------------------------------------------
// Utilities.
// -----------------------------------------------------------------------------

void check_upb_status(const upb_status* status, const char* msg);

#define CHECK_UPB(code, msg) do {                                             \
    upb_status status = UPB_STATUS_INIT;                                      \
    code;                                                                     \
    check_upb_status(&status, msg);                                           \
} while (0)

#endif  // __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__
