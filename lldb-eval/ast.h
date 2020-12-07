/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LLDB_EVAL_AST_H_
#define LLDB_EVAL_AST_H_

#include <memory>
#include <string>

#include "clang/Basic/TokenKinds.h"
#include "lldb-eval/value.h"
#include "lldb/API/SBType.h"

namespace lldb_eval {

class Visitor;

// TODO(werat): Save original token and the source position, so we can give
// better diagnostic messages during the evaluation.
class AstNode {
 public:
  virtual ~AstNode() {}

  virtual void Accept(Visitor* v) const = 0;
};

using ExprResult = std::unique_ptr<AstNode>;

class ErrorNode : public AstNode {
  void Accept(Visitor* v) const override;
};

class LiteralNode : public AstNode {
 public:
  LiteralNode(Value value) : value_(std::move(value)) {}

  void Accept(Visitor* v) const override;

  Value value() const { return value_; }

 private:
  Value value_;
};

class IdentifierNode : public AstNode {
 public:
  IdentifierNode(std::string name, Value value)
      : name_(std::move(name)), value_(std::move(value)) {}

  void Accept(Visitor* v) const override;

  std::string name() const { return name_; }
  Value value() const { return value_; }

 private:
  std::string name_;
  Value value_;
};

class CStyleCastNode : public AstNode {
 public:
  CStyleCastNode(lldb::SBType type, ExprResult rhs)
      : type_(std::move(type)), rhs_(std::move(rhs)) {}

  void Accept(Visitor* v) const override;

  lldb::SBType type() const { return type_; }
  AstNode* rhs() const { return rhs_.get(); }

 private:
  lldb::SBType type_;
  ExprResult rhs_;
};

class MemberOfNode : public AstNode {
 public:
  enum class Type {
    OF_OBJECT,
    OF_POINTER,
  };

 public:
  MemberOfNode(Type type, ExprResult lhs, std::string member_id)
      : type_(type), lhs_(std::move(lhs)), member_id_(std::move(member_id)) {}

  void Accept(Visitor* v) const override;

  Type type() const { return type_; }
  AstNode* lhs() const { return lhs_.get(); }
  std::string member_id() const { return member_id_; }

 private:
  Type type_;
  ExprResult lhs_;
  std::string member_id_;
};

class BinaryOpNode : public AstNode {
 public:
  BinaryOpNode(clang::tok::TokenKind op, ExprResult lhs, ExprResult rhs)
      : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  void Accept(Visitor* v) const override;

  clang::tok::TokenKind op() const { return op_; }
  std::string op_name() const { return clang::tok::getTokenName(op_); }
  AstNode* lhs() const { return lhs_.get(); }
  AstNode* rhs() const { return rhs_.get(); }

 private:
  // TODO(werat): Use custom enum with binary operators.
  clang::tok::TokenKind op_;
  ExprResult lhs_;
  ExprResult rhs_;
};

class UnaryOpNode : public AstNode {
 public:
  UnaryOpNode(clang::tok::TokenKind op, ExprResult rhs)
      : op_(op), rhs_(std::move(rhs)) {}

  void Accept(Visitor* v) const override;

  clang::tok::TokenKind op() const { return op_; }
  std::string op_name() const { return clang::tok::getTokenName(op_); }
  AstNode* rhs() const { return rhs_.get(); }

 private:
  // TODO(werat): Use custom enum with unary operators.
  clang::tok::TokenKind op_;
  ExprResult rhs_;
};

class TernaryOpNode : public AstNode {
 public:
  TernaryOpNode(ExprResult cond, ExprResult lhs, ExprResult rhs)
      : cond_(std::move(cond)), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  void Accept(Visitor* v) const override;

  AstNode* cond() const { return cond_.get(); }
  AstNode* lhs() const { return lhs_.get(); }
  AstNode* rhs() const { return rhs_.get(); }

 private:
  ExprResult cond_;
  ExprResult lhs_;
  ExprResult rhs_;
};

class Visitor {
 public:
  virtual ~Visitor() {}
  virtual void Visit(const ErrorNode* node) = 0;
  virtual void Visit(const LiteralNode* node) = 0;
  virtual void Visit(const IdentifierNode* node) = 0;
  virtual void Visit(const CStyleCastNode* node) = 0;
  virtual void Visit(const MemberOfNode* node) = 0;
  virtual void Visit(const BinaryOpNode* node) = 0;
  virtual void Visit(const UnaryOpNode* node) = 0;
  virtual void Visit(const TernaryOpNode* node) = 0;
};

}  // namespace lldb_eval

#endif  // LLDB_EVAL_AST_H_
