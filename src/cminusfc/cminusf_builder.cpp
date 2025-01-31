#include "cminusf_builder.hpp"
#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Type.hpp"
#include "Value.hpp"
#include "ast.hpp"
#include <cstddef>
#include <vector>

#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope  进入一个新的scope
 * scope.exit: exit current scope  退出当前的scope
 * scope.push: add a new binding to current scope 添加新的绑定
 * scope.find: find and return the value bound to the name 查找并返回绑定的值
 */

Value* CminusfBuilder::visit(ASTProgram &node) {
    VOID_T = module->get_void_type();
    INT1_T = module->get_int1_type();
    INT32_T = module->get_int32_type();
    INT32PTR_T = module->get_int32_ptr_type();
    FLOAT_T = module->get_float_type();
    FLOATPTR_T = module->get_float_ptr_type();

    Value *ret_val = nullptr;
    for (auto &decl : node.declarations) {
        ret_val = decl->accept(*this);
    }
    return ret_val;
}

Value* CminusfBuilder::visit(ASTNum &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.type==TYPE_INT){
        return  ConstantInt::get(node.i_val, module.get());
    }
    else if(node.type==TYPE_FLOAT){
        return ConstantFP::get(node.f_val, module.get());
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTVarDeclaration &node) {
    // TODO: This function is empty now.
    // Add some code here.
    // Value * result = nullptr;
    if(node.type==TYPE_INT){
        if(node.num){
            auto* arrayType = ArrayType::get(INT32_T, node.num->i_val);
            if(scope.in_global()){
                // 分配数组并初始化
                // 创建 0 常数类实例化，用于全局变量的初始化（目前全局变量仅支持初始化为 0）
                auto initializer = ConstantZero::get(INT32_T, module.get());
                // 为全局数组变量分配空间。参数解释：名字 name，所属 module，待分配空间类型
                // type，标志全局变量是否有 const 限定（cminus 中全是 false），初始化值
                auto array = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);
                scope.push(node.id, array);
            }else {
                auto aAlloca = builder->create_alloca(arrayType);
                scope.push(node.id, aAlloca);
            }
        }
        else {
            if(scope.in_global()){
                // 创建 0 常数类实例化，用于全局变量的初始化（目前全局变量仅支持初始化为 0）
                auto initializer = ConstantZero::get(INT32_T, module.get());
                // 为全局数组变量分配空间。参数解释：名字 name，所属 module，待分配空间类型
                // type，标志全局变量是否有 const 限定（cminus 中全是 false），初始化值
                auto array = GlobalVariable::create(node.id, module.get(), INT32_T, false, initializer);
                scope.push(node.id, array);
            }else {
                auto aAlloca = builder->create_alloca(INT32_T);
                scope.push(node.id, aAlloca);
            }
        }
    }else {
        if(node.num){
            auto* arrayType = ArrayType::get(FLOAT_T, node.num->i_val);
            if(scope.in_global()){
                auto initializer = ConstantZero::get(INT32_T, module.get());
                auto array = GlobalVariable::create("x", module.get(), arrayType, false, initializer);
                scope.push(node.id, array);
            }else {
                auto aAlloca = builder->create_alloca(arrayType);
                scope.push(node.id, aAlloca);
            }
        }
        else {
            auto aAlloca = builder->create_alloca(FLOAT_T);
            aAlloca->set_name(node.id);
            scope.push(node.id, aAlloca);
        }
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTFunDeclaration &node) {
    FunctionType *fun_type;
    Type *ret_type;
    std::vector<Type *> param_types;
    if (node.type == TYPE_INT)
        ret_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        ret_type = FLOAT_T;
    else
        ret_type = VOID_T;

    for (auto &param : node.params) {
        // TODO: Please accomplish param_types.
        if(param->type==TYPE_INT){
            if(param->isarray)param_types.push_back(INT32PTR_T);
            else param_types.push_back(INT32_T);
        }else if(param->type==TYPE_FLOAT){
            if(param->isarray)param_types.push_back(FLOATPTR_T);
            else param_types.push_back(FLOAT_T);
        }
    }

    fun_type = FunctionType::get(ret_type, param_types);
    auto func = Function::create(fun_type, node.id, module.get());
    scope.push(node.id, func);
    context.func = func;
    context.func_map[node.id] = func;
    auto funBB = BasicBlock::create(module.get(), "entry", func);
    builder->set_insert_point(funBB);
    scope.enter();
    std::vector<Value *> args;
    for (auto &arg : func->get_args()) {
        args.push_back(&arg);
    }
    for (unsigned int i = 0; i < node.params.size(); ++i) {
        // TODO: You need to deal with params and store them in the scope.
        node.params[i]->accept(*this);
        builder->create_store(args[i], scope.find(node.params[i]->id));
    }
    node.compound_stmt->accept(*this);
    if (not builder->get_insert_block()->is_terminated()) 
    {
        if (context.func->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (context.func->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    scope.exit();
    return nullptr;
}

Value* CminusfBuilder::visit(ASTParam &node) {
    // TODO: This function is empty now.
    // Add some code here.
    Value *address = nullptr;
    if(node.type==TYPE_INT){
        if(node.isarray){
            address = builder->create_alloca(INT32PTR_T);
        }else {
            address = builder->create_alloca(INT32_T);
        }
    }else {
        if(node.isarray){
            address = builder->create_alloca(FLOATPTR_T);
        }else {
            address = builder->create_alloca(FLOAT_T);
        }
    }

    scope.push(node.id, address);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTCompoundStmt &node) {
    // TODO: This function is not complete.
    // You may need to add some code here
    // to deal with complex statements. 
    
    for (auto &decl : node.local_declarations) {
        auto dValue = decl->accept(*this);
        scope.push(decl->id, dValue);
    }

    for (auto &stmt : node.statement_list) {
        stmt->accept(*this);
        if (builder->get_insert_block()->is_terminated())
            break;
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTExpressionStmt &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.expression){
        return node.expression->accept(*this);
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTSelectionStmt &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto loopBB = BasicBlock::create(module.get(), "loop", context.func);
    auto trueBB = BasicBlock::create(module.get(), "true", context.func);
    auto falseBB = BasicBlock::create(module.get(), "falseBB", context.func);
    auto otherBB = BasicBlock::create(module.get(), "other", context.func);
    builder->create_br(loopBB);
    builder->set_insert_point(loopBB);
    auto eValue = node.expression->accept(*this);
    builder->create_cond_br(eValue, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    scope.enter();
    node.if_statement->accept(*this);
    scope.exit();
    if (not builder->get_insert_block()->is_terminated()){
        builder->create_br(otherBB);
    }
    builder->set_insert_point(falseBB);
    if(node.else_statement){
        scope.enter();
        node.else_statement->accept(*this);
        scope.exit();
    }
    if (not builder->get_insert_block()->is_terminated()){
        builder->create_br(otherBB);
    }
    builder->set_insert_point(otherBB);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTIterationStmt &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto whileBB = BasicBlock::create(module.get(), "while", context.func);
    auto trueBB  = BasicBlock::create(module.get(), "true", context.func);
    auto falseBB = BasicBlock::create(module.get(), "false", context.func);
    builder->create_br(whileBB);
    builder->set_insert_point(whileBB);
    scope.enter();
    auto eValue = node.expression->accept(*this);
    scope.exit();
    builder->create_cond_br(eValue, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    node.statement->accept(*this);
    builder->create_br(whileBB);
    builder->set_insert_point(falseBB);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        builder->create_void_ret();
        return nullptr;
    } else {
        // TODO: The given code is incomplete.
        // You need to solve other return cases (e.g. return an integer).
        auto ret = node.expression->accept(*this);
        builder->create_ret(ret);
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTVar &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto aAlloca = scope.find(node.id);
    bool isarray = false;
    if(aAlloca->is<GlobalVariable>()){
        auto ptr = aAlloca->as<GlobalVariable>();
        if(ptr->get_type()->is_pointer_type()){
            isarray = true;
        }
    }else if (aAlloca->is<AllocaInst>()) {
        auto ptr = aAlloca->as<AllocaInst>();
        if(ptr->get_alloca_type()->is_array_type()){
            isarray = true;
        }
    }
    if(node.expression){
        if(!isarray){
            auto eValue = node.expression->accept(*this);
            auto aPointer = builder->create_load(aAlloca);
            auto aGEP = builder->create_gep(aPointer, {eValue});
            return builder->create_load(aGEP);
        }else{
            auto eValue = node.expression->accept(*this);
            auto aGEP = builder->create_gep(aAlloca, {CONST_INT(0), eValue});
            return builder->create_load(aGEP);
        }
    }
    if(isarray){
        auto arrToPtr = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
        return arrToPtr;
    }
    return builder->create_load(aAlloca);
}

Value* CminusfBuilder::visit(ASTAssignExpression &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto eValue  = node.expression->accept(*this);
    auto aAlloca = scope.find(node.var->id);
    Value* address = nullptr;
    if(node.var->expression){
        bool isarray = false;
        if(aAlloca->is<GlobalVariable>()){
            auto ptr = aAlloca->as<GlobalVariable>();
            if(ptr->get_type()->is_pointer_type()){
                isarray = true;
            }
        }else if(aAlloca->is<AllocaInst>()){
            auto ptr = aAlloca->as<AllocaInst>();
            // 确保ptr正确转型
            assert(ptr);
            if(ptr->get_alloca_type()->is_array_type()){
                isarray = true;
            }
        }

        if(!isarray){
            auto e = node.var->expression->accept(*this);
            auto aPointer = builder->create_load(aAlloca);
            address = builder->create_gep(aPointer, {e});
        }else{
            auto e = node.var->expression->accept(*this);
            address = builder->create_gep(aAlloca, {CONST_INT(0), e});
        }

    }else {
        address= aAlloca;
    }
    builder->create_store(eValue, address);
    return eValue;
}

Value* CminusfBuilder::visit(ASTSimpleExpression &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.additive_expression_r){
        Value *lValue = nullptr, *rValue = nullptr;
        Type  *highest = nullptr;
        {
            auto ltValue = node.additive_expression_l->accept(*this);
            auto rtValue = node.additive_expression_r->accept(*this);
            
            if(ltValue->get_type()->is_integer_type()&&rtValue->get_type()->is_integer_type()){
                lValue = ltValue;
                rValue = rtValue;
                highest = INT32_T;
            }else{
                if(ltValue->get_type()->is_integer_type()){
                    lValue = builder->create_sitofp(ltValue, FLOAT_T);
                }else {
                    lValue = ltValue;
                }
                if(rtValue->get_type()->is_integer_type()){
                    rValue = builder->create_sitofp(rtValue, FLOAT_T);
                }else {
                    rValue = rtValue;
                }
                highest = FLOAT_T;
            }
        }

        Value * result = nullptr;
        if(node.op==OP_LE){
            if(highest==INT32_T)
                result = builder->create_icmp_le(lValue, rValue);
            else
                result = builder->create_fcmp_le(lValue, rValue);
        }
        // <
        else if(node.op==OP_LT){
            if(highest==INT32_T)
                result = builder->create_icmp_lt(lValue, rValue);
            else
                result = builder->create_fcmp_lt(lValue, rValue);
        }
        // >
        else if(node.op==OP_GT){
            if(highest==INT32_T)
                result = builder->create_icmp_gt(lValue, rValue);
            else
                result = builder->create_fcmp_gt(lValue, rValue);
        }
        // >=
        else if(node.op==OP_GE){
            if(highest==INT32_T)
                result = builder->create_icmp_ge(lValue, rValue);
            else
                result = builder->create_fcmp_ge(lValue, rValue);
        }
        // ==
        else if(node.op==OP_EQ){
            if(highest==INT32_T)
                result = builder->create_icmp_eq(lValue, rValue);
            else
                result = builder->create_fcmp_eq(lValue, rValue);
        }
        // !=
        else if(node.op==OP_NEQ){
            if(highest==INT32_T)
                result = builder->create_icmp_ne(lValue, rValue);
            else
                result = builder->create_fcmp_ne(lValue, rValue);
        }
        else{
        }
        return result;
    }else {
        return node.additive_expression_l->accept(*this);
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTAdditiveExpression &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.additive_expression){
        auto eValue    = node.additive_expression->accept(*this);
        auto termValue = node.term->accept(*this);
        Value* retValue = nullptr;
        // 使用is和as进行优化
        if(node.op==OP_PLUS){
            if(eValue->get_type()==INT32_T&&termValue->get_type()==INT32_T)
                retValue = builder->create_iadd(eValue,termValue);
            else if (eValue->get_type()==INT32_T&&termValue->get_type()==FLOAT_T){
                auto ftValue = builder->create_sitofp(eValue, FLOAT_T);
                retValue = builder->create_fadd(ftValue, termValue);
            }else if(eValue->get_type()==FLOAT_T&&termValue->get_type()==INT32_T){
                auto ftValue = builder->create_sitofp(termValue, FLOAT_T);
                retValue = builder->create_fadd(eValue, ftValue);
            }else {
                retValue = builder->create_fadd(eValue, termValue);
            }
        }else {
            if(eValue->get_type()==INT32_T&&termValue->get_type()==INT32_T)
                retValue = builder->create_isub(eValue,termValue);
            else if (eValue->get_type()==INT32_T&&termValue->get_type()==FLOAT_T){
                auto ftValue = builder->create_sitofp(eValue, FLOAT_T);
                retValue = builder->create_fsub(ftValue, termValue);
            }else if(eValue->get_type()==FLOAT_T&&termValue->get_type()==INT32_T){
                auto ftValue = builder->create_sitofp(termValue, FLOAT_T);
                retValue = builder->create_fsub(eValue, ftValue);
            }else {
                retValue = builder->create_fsub(eValue, termValue);
            }
        }
        return retValue;
        
    }else {
        auto termValue = node.term->accept(*this);
        return termValue;
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTTerm &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.term){
        auto termValue = node.term->accept(*this);
        auto fValue = node.factor->accept(*this);
        Value * retValue = nullptr;
        if(node.op==OP_MUL){
            if(termValue->get_type()==INT32_T&&fValue->get_type()==INT32_T)
                retValue = builder->create_imul(termValue,fValue);
            else if (termValue->get_type()==INT32_T&&fValue->get_type()==FLOAT_T){
                auto ftValue = builder->create_sitofp(termValue, FLOAT_T);
                retValue = builder->create_fmul(ftValue, fValue);
            }else if(termValue->get_type()==FLOAT_T&&fValue->get_type()==INT32_T){
                auto ftValue = builder->create_sitofp(fValue, FLOAT_T);
                retValue = builder->create_fmul(termValue, ftValue);
            }else {
                retValue = builder->create_fmul(termValue, fValue);
            }
        }else{
            if(termValue->get_type()==INT32_T&&fValue->get_type()==INT32_T)
                retValue = builder->create_isdiv(termValue,fValue);
            else if (termValue->get_type()==INT32_T&&fValue->get_type()==FLOAT_T){
                auto ftValue = builder->create_sitofp(termValue, FLOAT_T);
                retValue = builder->create_fdiv(ftValue, fValue);
            }else if(termValue->get_type()==FLOAT_T&&fValue->get_type()==INT32_T){
                auto ftValue = builder->create_sitofp(fValue, FLOAT_T);
                retValue = builder->create_fdiv(termValue, ftValue);
            }else {
                retValue = builder->create_fdiv(termValue, fValue);
            }
        }
        return retValue;
    }else {
        auto fValue = node.factor->accept(*this);
        return fValue;
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTCall &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto call = scope.find(node.id);
    scope.enter();
    Function* func = context.func_map[node.id];
    //获得参数
    // for(auto& it: func->get_args()){

    // }
    assert(call->get_type()->is_function_type());
    std::vector<Value*>aVector;
    func->get_args();
    for(auto& arg: node.args){
        auto aValue = arg->accept(*this);
        aVector.push_back(aValue);
    }

    auto retValue  = builder->create_call(call, aVector);
    scope.exit();
    return retValue;
}
