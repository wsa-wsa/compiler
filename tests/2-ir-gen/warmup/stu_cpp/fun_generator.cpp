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
    // 返回值
    auto Int32Type = module->get_int32_type();
    // 参数列表
    std::vector<Type*>Ints(1, Int32Type);
    // 函数类型创建
    auto calleeTy = FunctionType::get(Int32Type, Ints);
    // 函数定义
    auto calleeFun = Function::create(calleeTy, "callee", module);
    // 创建函数block
    auto bb = BasicBlock::create(module, "entry", calleeFun);
    builder->set_insert_point(bb);
    /*
        //获取参数
        auto aAlloca = builder->create_alloca(Int32Type);
        std::vector<Value*>args;
        for(auto& arg: calleeFun->get_args()){
            args.push_back(&arg);
        }
        //加载参数
        builder->create_store(args[0], aAlloca);
        auto aValue = builder->create_load(aAlloca);
    */
    auto aValue = &*calleeFun->get_args().begin(); // 直接获取第一个参数
    auto ret = builder->create_imul(aValue, CONST_INT(2));
    builder->create_ret(ret);

    std::vector<Type*>mainARG(0, {}); 
    auto mainTy = FunctionType::get(Int32Type, mainARG);
    auto mainFun = Function::create(mainTy, "main", module);
    auto mainBB = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(mainBB);
    auto cValue = CONST_INT(110);
    auto call = builder->create_call(calleeFun, {cValue});
    builder->create_ret(call);
    std::cout<<module->print();
    delete module;
}