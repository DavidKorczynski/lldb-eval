// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tools/fuzzer/constraints.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tools/fuzzer/ast.h"

using namespace fuzzer;
using namespace testing;

TEST(Constraints, ScalarValues) {
  SpecificTypes float_only = FLOAT_TYPES;
  SpecificTypes int_only = INT_TYPES;

  TypeConstraints float_constraints = float_only;
  TypeConstraints int_constraints = int_only;

  TypeConstraints any = AnyType();
  TypeConstraints none;

  PointerType void_ptr_type{QualifiedType(ScalarType::Void)};

  EXPECT_THAT(float_only.allows_any_of(ScalarType::Float), IsTrue());
  EXPECT_THAT(float_only.allows_any_of(ScalarType::Double), IsTrue());
  EXPECT_THAT(float_only.allows_any_of(ScalarType::LongDouble), IsTrue());

  EXPECT_THAT(float_only.allows_any_of(ScalarType::Void), IsFalse());
  EXPECT_THAT(float_only.allows_any_of(ScalarType::Bool), IsFalse());
  EXPECT_THAT(float_only.allows_any_of(ScalarType::SignedInt), IsFalse());
  EXPECT_THAT(float_only.allows_any_of(ScalarType::UnsignedLong), IsFalse());

  EXPECT_THAT(int_only.allowed_tagged_types(), IsEmpty());
  EXPECT_THAT(float_only.allowed_tagged_types(), IsEmpty());

  EXPECT_THAT(int_only.allows_non_void_pointer(), IsFalse());
  EXPECT_THAT(float_only.allows_non_void_pointer(), IsFalse());

  EXPECT_THAT(int_only.allows_void_pointer(), IsFalse());
  EXPECT_THAT(float_only.allows_void_pointer(), IsFalse());

  EXPECT_THAT(int_only.allows_any_of(ScalarType::Float), IsFalse());
  EXPECT_THAT(int_only.allows_any_of(ScalarType::Double), IsFalse());
  EXPECT_THAT(int_only.allows_any_of(ScalarType::LongDouble), IsFalse());

  EXPECT_THAT(int_only.allows_any_of(ScalarType::Void), IsFalse());
  EXPECT_THAT(int_only.allows_any_of(ScalarType::Bool), IsTrue());
  EXPECT_THAT(int_only.allows_any_of(ScalarType::SignedInt), IsTrue());
  EXPECT_THAT(int_only.allows_any_of(ScalarType::UnsignedLong), IsTrue());

  EXPECT_THAT(float_constraints.allows_type(ScalarType::Float), IsTrue());
  EXPECT_THAT(float_constraints.allows_type(ScalarType::SignedInt), IsFalse());
  EXPECT_THAT(float_constraints.allows_type(TaggedType("Test")), IsFalse());
  EXPECT_THAT(float_constraints.allows_type(void_ptr_type), IsFalse());

  EXPECT_THAT(int_constraints.allows_type(ScalarType::Float), IsFalse());
  EXPECT_THAT(int_constraints.allows_type(ScalarType::SignedInt), IsTrue());
  EXPECT_THAT(int_constraints.allows_type(TaggedType("Test")), IsFalse());
  EXPECT_THAT(int_constraints.allows_type(void_ptr_type), IsFalse());

  EXPECT_THAT(any.allows_type(ScalarType::Float), IsTrue());
  EXPECT_THAT(any.allows_type(ScalarType::SignedInt), IsTrue());

  EXPECT_THAT(none.allows_type(ScalarType::Float), IsFalse());
  EXPECT_THAT(none.allows_type(ScalarType::SignedInt), IsFalse());
}

TEST(Constraints, TaggedTypes) {
  SpecificTypes test_struct =
      std::unordered_set<TaggedType>{{TaggedType("TestStruct")}};

  EXPECT_THAT(test_struct.allows_any_of(ScalarMask::all_set()), IsFalse());
  EXPECT_THAT(test_struct.allowed_tagged_types(),
              UnorderedElementsAre(TaggedType("TestStruct")));
  EXPECT_THAT(test_struct.allows_non_void_pointer(), IsFalse());
  EXPECT_THAT(test_struct.allows_void_pointer(), IsFalse());

  TypeConstraints any = AnyType();
  TypeConstraints none;

  EXPECT_THAT(any.allows_tagged_types(), IsTrue());
  EXPECT_THAT(any.allowed_tagged_types(), IsNull());
  EXPECT_THAT(any.allows_type(TaggedType("TestStruct")), IsTrue());

  EXPECT_THAT(none.allows_tagged_types(), IsFalse());
  EXPECT_THAT(none.allowed_tagged_types(), IsNull());
  EXPECT_THAT(none.allows_type(TaggedType("TestStruct")), IsFalse());
}

TEST(Constraints, PointerTypes) {
  SpecificTypes int_ptr = SpecificTypes::make_pointer_constraints(
      SpecificTypes(ScalarMask{ScalarType::SignedInt}));
  SpecificTypes void_ptr = SpecificTypes::make_pointer_constraints(
      SpecificTypes(), VoidPointerConstraint::Allow);
  SpecificTypes null_ptr = SpecificTypes(NullptrType{});

  EXPECT_THAT(int_ptr.allows_any_of(ScalarMask::all_set()), IsFalse());
  EXPECT_THAT(int_ptr.allowed_tagged_types(), IsEmpty());
  EXPECT_THAT(int_ptr.allows_non_void_pointer(), IsTrue());
  EXPECT_THAT(int_ptr.allows_void_pointer(), IsFalse());
  EXPECT_THAT(int_ptr.allows_nullptr(), IsFalse());

  EXPECT_THAT(void_ptr.allows_any_of(ScalarMask::all_set()), IsFalse());
  EXPECT_THAT(void_ptr.allowed_tagged_types(), IsEmpty());
  EXPECT_THAT(void_ptr.allows_non_void_pointer(), IsFalse());
  EXPECT_THAT(void_ptr.allows_void_pointer(), IsTrue());
  EXPECT_THAT(void_ptr.allows_nullptr(), IsFalse());

  EXPECT_THAT(null_ptr.allows_any_of(ScalarMask::all_set()), IsFalse());
  EXPECT_THAT(null_ptr.allowed_tagged_types(), IsEmpty());
  EXPECT_THAT(null_ptr.allows_non_void_pointer(), IsFalse());
  EXPECT_THAT(null_ptr.allows_void_pointer(), IsFalse());
  EXPECT_THAT(null_ptr.allows_nullptr(), IsTrue());

  PointerType const_int_ptr{
      QualifiedType(ScalarType::SignedInt, CvQualifier::Const)};
  PointerType volatile_void_ptr{
      QualifiedType(ScalarType::Void, CvQualifier::Volatile)};

  TypeConstraints int_ptr_constraints = int_ptr;
  TypeConstraints void_ptr_constraints = void_ptr;
  TypeConstraints null_ptr_constraints = null_ptr;

  TypeConstraints int_constraints = int_ptr.allowed_to_point_to();
  TypeConstraints void_constraints = void_ptr.allowed_to_point_to();

  EXPECT_THAT(int_ptr_constraints.allows_type(const_int_ptr), IsTrue());
  EXPECT_THAT(int_ptr_constraints.allows_type(volatile_void_ptr), IsFalse());
  EXPECT_THAT(int_ptr_constraints.allows_type(NullptrType{}), IsFalse());
  EXPECT_THAT(int_constraints.allows_any_of(ScalarType::SignedInt), IsTrue());
  EXPECT_THAT(int_constraints.allows_any_of(ScalarType::Void), IsFalse());

  EXPECT_THAT(void_ptr_constraints.allows_type(const_int_ptr), IsFalse());
  EXPECT_THAT(void_ptr_constraints.allows_type(volatile_void_ptr), IsTrue());
  EXPECT_THAT(void_ptr_constraints.allows_type(NullptrType{}), IsFalse());
  EXPECT_THAT(void_constraints.allows_any_of(ScalarType::SignedInt), IsFalse());

  EXPECT_THAT(null_ptr_constraints.allows_type(NullptrType{}), IsTrue());
  EXPECT_THAT(null_ptr_constraints.allows_type(const_int_ptr), IsFalse());
  EXPECT_THAT(null_ptr_constraints.allows_type(volatile_void_ptr), IsFalse());
  EXPECT_THAT(null_ptr_constraints.allowed_to_point_to().satisfiable(),
              IsFalse());

  // Due to the way we represent constraints, we cannot state that we support
  // void types :(
  EXPECT_THAT(void_constraints.allows_any_of(ScalarType::Void), IsFalse());

  TypeConstraints any = AnyType();
  TypeConstraints none;

  EXPECT_THAT(any.allows_type(const_int_ptr), IsTrue());
  EXPECT_THAT(any.allows_type(volatile_void_ptr), IsTrue());
  EXPECT_THAT(any.allows_type(NullptrType{}), IsTrue());

  EXPECT_THAT(none.allows_type(const_int_ptr), IsFalse());
  EXPECT_THAT(none.allows_type(volatile_void_ptr), IsFalse());
  EXPECT_THAT(none.allows_type(NullptrType{}), IsFalse());
}

TEST(Constraints, EnumTypes) {
  EnumType scoped_enum("ScopedEnum", true);
  EnumType unscoped_enum("UnscopedEnum", false);
  EnumType specific_enum("SpecificEnum", true);

  TypeConstraints any = AnyType();
  TypeConstraints none;
  TypeConstraints bool_ctx = TypeConstraints::all_in_bool_ctx();
  TypeConstraints only_specific = SpecificTypes(specific_enum);

  SpecificTypes only_scoped_types, only_unscoped_types;
  only_scoped_types.allow_scoped_enums();
  only_unscoped_types.allow_unscoped_enums();
  TypeConstraints only_scoped = std::move(only_scoped_types);
  TypeConstraints only_unscoped = std::move(only_unscoped_types);

  EXPECT_THAT(any.allows_type(scoped_enum), IsTrue());
  EXPECT_THAT(any.allows_type(unscoped_enum), IsTrue());

  EXPECT_THAT(none.allows_type(scoped_enum), IsFalse());
  EXPECT_THAT(none.allows_type(unscoped_enum), IsFalse());

  EXPECT_THAT(bool_ctx.allows_type(scoped_enum), IsFalse());
  EXPECT_THAT(bool_ctx.allows_type(unscoped_enum), IsTrue());

  EXPECT_THAT(only_scoped.allows_type(scoped_enum), IsTrue());
  EXPECT_THAT(only_scoped.allows_type(unscoped_enum), IsFalse());
  EXPECT_THAT(only_scoped.allows_type(specific_enum), IsTrue());

  EXPECT_THAT(only_unscoped.allows_type(scoped_enum), IsFalse());
  EXPECT_THAT(only_unscoped.allows_type(unscoped_enum), IsTrue());
  EXPECT_THAT(only_unscoped.allows_type(specific_enum), IsFalse());

  EXPECT_THAT(only_specific.allows_type(scoped_enum), IsFalse());
  EXPECT_THAT(only_specific.allows_type(unscoped_enum), IsFalse());
  EXPECT_THAT(only_specific.allows_type(specific_enum), IsTrue());
}

TEST(Constraints, Unsatisfiability) {
  TypeConstraints default_ctor;
  TypeConstraints default_specific_types = SpecificTypes();

  EXPECT_THAT(default_ctor.satisfiable(), IsFalse());
  EXPECT_THAT(default_specific_types.satisfiable(), IsFalse());
}

namespace fuzzer {
bool operator==(const MemoryConstraints& lhs, const MemoryConstraints& rhs) {
  return lhs.must_be_valid() == rhs.must_be_valid() &&
         lhs.required_freedom_index() == rhs.required_freedom_index();
}
}  // namespace fuzzer

TEST(Constraints, MemoryConstraints) {
  MemoryConstraints default_ctor;
  MemoryConstraints freedom_ctor(0);
  MemoryConstraints invalid(false, 5);
  MemoryConstraints valid(true, 5);

  EXPECT_EQ(default_ctor.must_be_valid(), false);
  EXPECT_EQ(default_ctor.required_freedom_index(), 0);
  EXPECT_EQ(freedom_ctor.must_be_valid(), true);
  EXPECT_EQ(freedom_ctor.required_freedom_index(), 0);
  EXPECT_EQ(invalid.must_be_valid(), false);
  EXPECT_EQ(invalid.required_freedom_index(), 0);
  EXPECT_EQ(valid.must_be_valid(), true);
  EXPECT_EQ(valid.required_freedom_index(), 5);

  EXPECT_EQ(invalid.from_address_of(), MemoryConstraints(false, 0));
  EXPECT_EQ(invalid.from_dereference_of(false), MemoryConstraints(true, 1));
  EXPECT_EQ(invalid.from_dereference_of(true), MemoryConstraints(false, 0));
  EXPECT_EQ(invalid.from_member_of(false, 1), MemoryConstraints(true, 1));
  EXPECT_EQ(invalid.from_member_of(true, 1), MemoryConstraints(false, 0));

  EXPECT_EQ(valid.from_address_of(), MemoryConstraints(true, 4));
  EXPECT_EQ(valid.from_dereference_of(false), MemoryConstraints(true, 6));
  EXPECT_EQ(valid.from_dereference_of(true), MemoryConstraints(true, 6));
  EXPECT_EQ(valid.from_member_of(false, 1), MemoryConstraints(true, 1));
  EXPECT_EQ(valid.from_member_of(true, 1), MemoryConstraints(true, 1));
}
