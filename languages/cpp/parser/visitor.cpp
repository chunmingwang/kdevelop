/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "visitor.h"

Visitor::visitor_fun_ptr Visitor::_S_table[AST::NODE_KIND_COUNT] = {
  0,
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitAccessSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitAsmDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBaseClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBaseSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBinaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCastExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitClassMemberAccess),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitClassSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCompoundStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCondition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitConditionalExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCppCastExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCtorInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeclarationStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeleteExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDoStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitElaboratedTypeSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitEnumSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitEnumerator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExceptionSpecification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExpressionOrDeclarationStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExpressionStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitForStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitFunctionCall),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitFunctionDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitIfStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitIncrDecrExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitializerClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLabeledStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLinkageBody),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLinkageSpecification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitMemInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitName),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNamespace),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNamespaceAliasDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewTypeId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitOperator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitOperatorFunctionId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitParameterDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitParameterDeclarationClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPostfixExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPrimaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPtrOperator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPtrToMember),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitReturnStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSimpleDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSimpleTypeSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSizeofExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitStringLiteral),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSubscriptExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSwitchStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateArgument),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateParameter),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitThrowExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTranslationUnit),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTryBlockStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCatchStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeIdentification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeParameter),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypedef),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUnaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUnqualifiedName),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUsing),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUsingDirective),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitWhileStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitWinDeclSpec),
  0,
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitJumpStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSignalSlotExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitQPropertyDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitForRangeDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeIDOperator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitStaticAssert),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTrailingReturnType),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLambdaExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLambdaCapture),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLambdaDeclarator),
};

Visitor::Visitor()
{
}

Visitor::~Visitor()
{
}

void Visitor::visit(AST *node)
{
  if (node)
    (this->*_S_table[node->kind])(node);
}

