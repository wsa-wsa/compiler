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
    auto aValue = ConstantFP::get(5.55, module);
    auto intValue = ConstantInt::get(1, module);
    auto fpValue = builder->create_sitofp(intValue, module->get_float_type());
    auto fcmp = builder->create_fcmp_gt(aValue,  fpValue);
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
    auto retBB = BasicBlock::create(module, "", mainFun);
    auto br = builder->create_cond_br(fcmp,trueBB, falseBB);
    builder->set_insert_point(trueBB);
    builder->create_ret(CONST_INT(255));
    builder->set_insert_point(falseBB);
    builder->create_br(retBB);  // 条件为假跳转到合并块
    builder->set_insert_point(retBB);
    builder->create_ret(CONST_INT(0));
    std::cout<<module->print();
    delete module;
}