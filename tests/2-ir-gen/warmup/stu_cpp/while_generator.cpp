#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "IRBuilder.hpp"
#include "Instruction.hpp"
#include "Module.hpp"
#include "Type.hpp"
#include "Value.hpp"
#include "syntax_analyzer.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

// 定义一个从常数值获取/创建 ConstantInt 类实例化的宏，方便多次调用
#define CONST_INT(num) ConstantInt::get(num, module)

// 定义一个从常数值获取/创建 ConstantFP 类实例化的宏，方便多次调用
#define CONST_FP(num) ConstantFP::get(num, module)

// 必须注明跳转的标签，否则不会顺序执行而是会发生错误
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr ,module);
    // int32类型
    auto Int32Type = module->get_int32_type();
    std::vector<Type*>mainARG(0, {}); 
    auto mainTy = FunctionType::get(Int32Type, mainARG);
    auto mainFun = Function::create(mainTy, "main", module);
    auto mainBB = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(mainBB);
    auto aAlloca = builder->create_alloca(Int32Type);
    auto iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), aAlloca);
    builder->create_store(CONST_INT(0), iAlloca);
    auto judgeBB = BasicBlock::create(module, "judgeBB", mainFun);
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
    auto retBB = BasicBlock::create(module, "retBB", mainFun);
    builder->create_br(judgeBB);
    builder->set_insert_point(judgeBB);
    auto iValue  = builder->create_load(iAlloca);
    auto icmp = builder->create_icmp_lt(iValue,  CONST_INT(10));
    auto br = builder->create_cond_br(icmp, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    auto itValue = builder->create_iadd(iValue, CONST_INT(1));
    builder->create_store(itValue, iAlloca);
    auto aValue  = builder->create_load(aAlloca);
    auto atValue = builder->create_iadd(aValue, itValue);
    builder->create_store(atValue, aAlloca);
    builder->create_br(judgeBB);  // 条件为假跳转到合并块
    builder->set_insert_point(falseBB);
    builder->create_br(retBB);
    builder->set_insert_point(retBB);
    auto retValue = builder->create_load(aAlloca);
    builder->create_ret(retValue);
    std::cout<<module->print();
    delete module;
}