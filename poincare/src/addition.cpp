#include <poincare/addition.h>
#include <poincare/expression.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
}
#include "layout/horizontal_layout.h"
#include "layout/string_layout.h"
#include "layout/parenthesis_layout.h"

Addition::Addition(Expression ** operands,
    int numberOfOperands,
    bool cloneOperands) : m_numberOfOperands(numberOfOperands) {
  assert(operands != nullptr);
  assert(numberOfOperands >= 2);
  m_operands = (Expression **)malloc(numberOfOperands*sizeof(Expression *));
  for (int i=0; i<numberOfOperands; i++) {
    assert(operands[i] != nullptr);
    if (cloneOperands) {
      m_operands[i] = operands[i]->clone();
    } else {
      m_operands[i] = operands[i];
    }
  }
}

Addition::~Addition() {
  for (int i=0; i<m_numberOfOperands; i++) {
    delete m_operands[i];
  }
  free(m_operands);
}

int Addition::numberOfOperands() const {
  return m_numberOfOperands;
}

const Expression * Addition::operand(int i) const {
  assert(i >= 0);
  assert(i < m_numberOfOperands);
  return m_operands[i];
}

Expression::Type Addition::type() const {
  return Type::Addition;
}

float Addition::approximate(Context& context) const {
  float result = m_operands[0]->approximate(context);
  for (int i=1; i<m_numberOfOperands; i++) {
    float next = m_operands[i]->approximate(context);
    result = this->operateApproximatevelyOn(result, next);
  }
  return result;
}

Expression * Addition::evaluate(Context& context) const {
  Expression * result = m_operands[0]->evaluate(context);
  for (int i=1; i<m_numberOfOperands; i++) {
    Expression * next = m_operands[i]->evaluate(context);
    Expression * newResult = this->evaluateOn(result, next, context);
    delete result;
    result = newResult;
    delete next;
  }
  assert(result == nullptr || result->type() == Type::Float || result->type() == Type::Matrix);
  return result;
}

Expression * Addition::clone() const {
  return this->cloneWithDifferentOperands(m_operands, m_numberOfOperands, true);
}

Expression * Addition::cloneWithDifferentOperands(Expression** newOperands,
        int numberOfOperands, bool cloneOperands) const {
  return new Addition(newOperands, numberOfOperands, cloneOperands);
}

ExpressionLayout * Addition::createLayout(DisplayMode displayMode) const {
  int number_of_children = 2*m_numberOfOperands-1;
  ExpressionLayout** children_layouts = (ExpressionLayout **)malloc(number_of_children*sizeof(ExpressionLayout *));
  children_layouts[0] = m_operands[0]->createLayout(displayMode);
  for (int i=1; i<m_numberOfOperands; i++) {
    children_layouts[2*i-1] = new StringLayout("+", 1);
    children_layouts[2*i] = m_operands[i]->type() == Type::Opposite ? new ParenthesisLayout(m_operands[i]->createLayout(displayMode)) : m_operands[i]->createLayout(displayMode);
  }
  return new HorizontalLayout(children_layouts, number_of_children);
}

bool Addition::isCommutative() const {
  return true;
}

float Addition::operateApproximatevelyOn(float a, float b) const {
  return a + b;
}

Expression * Addition::evaluateOn(Expression * a, Expression * b, Context& context) const {
  if (a == nullptr || b == nullptr) {
    return nullptr;
  }
  assert(a->type() == Type::Float || a->type() == Type::Matrix);
  assert(b->type() == Type::Float || b->type() == Type::Matrix);
  Expression * result = nullptr;
  if (a->type() == Type::Float && b->type() == Type::Float) {
    result = new Float(a->approximate(context) + b->approximate(context));
  }
  if (a->type() == Type::Float && b->type() == Type::Matrix) {
    result = evaluateOnFloatAndMatrix((Float *)a, (Matrix *)b, context);
  }
  if (b->type() == Type::Float && a->type() == Type::Matrix) {
    result = evaluateOnFloatAndMatrix((Float *)b, (Matrix *)a, context);
  }
  if (b->type() == Type::Matrix && a->type() == Type::Matrix) {
    result = evaluateOnMatrices((Matrix *)a, (Matrix *)b, context);
  }
  return result;
}

Expression * Addition::evaluateOnFloatAndMatrix(Float * a, Matrix * m, Context& context) const {
  Expression * operands[m->numberOfRows() * m->numberOfColumns()];
  for (int i = 0; i < m->numberOfRows() * m->numberOfColumns(); i++) {
    operands[i] = new Float(a->approximate(context) + m->operand(i)->approximate(context));
  }
  return new Matrix(operands, m->numberOfRows() * m->numberOfColumns(), m->numberOfColumns(), m->numberOfRows(), false);
}

Expression * Addition::evaluateOnMatrices(Matrix * m, Matrix * n, Context& context) const {
  if (m->numberOfColumns() != n->numberOfColumns() || m->numberOfRows() != n->numberOfRows()) {
    return nullptr;
  }
  Expression * operands[m->numberOfRows() * m->numberOfColumns()];
  for (int i = 0; i < m->numberOfRows() * m->numberOfColumns(); i++) {
    operands[i] = new Float(m->operand(i)->approximate(context) + n->operand(i)->approximate(context));
  }
  return new Matrix(operands, m->numberOfRows() * m->numberOfColumns(), m->numberOfColumns(), m->numberOfRows(), false);
}
