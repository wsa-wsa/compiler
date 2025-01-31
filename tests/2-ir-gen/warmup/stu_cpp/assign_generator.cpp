#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "IRBuilder.hpp"
#include "Instruction.hpp"
#include "Module.hpp"
#include "Type.hpp"
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

    auto builder = new IRBuilder(nullptr, module);
    auto *Int32Type = module->get_int32_type();
    auto *arrayType = ArrayType::get(Int32Type, 10);
    auto initializer = ConstantZero::get(Int32Type, module);
    // 创建main函数
    // 1、声明函数的参数类型
    std::vector<Type *>Ints(1, Int32Type);
    // 2、创建函数类型，第一个参数数返回值类型，第二个参数是参数类型列表
    auto mainFunTy  = FunctionType::get(Int32Type, {});
    // 3、创建函数，参数分别是 函数类型，函数名 name，函数所属
    auto mainFun = Function::create(mainFunTy, "main", module);
    // 4、创建函数block
    auto bb = BasicBlock::create(module, "entry", mainFun);

    builder->set_insert_point(bb);
    /*
        // 创建alloca指令，为函数返回值分配空间
        auto retAlloca = builder->create_alloca(Int32Type);
        // main 函数的返回值默认为 0，可以提前设置为 0
        builder->create_store(CONST_INT(0), retAlloca);
    */
    // 为数组a[10]分配空间
    auto aAlloca  = builder->create_alloca(arrayType);
    // 用 builder 创建 getelementptr 指令，
    // 参数解释：第一个参数传入getelementptr 的指针参数，
    //          第二个参数传入取偏移的数组。
    // 在此条语句中，x 为 [1 x i32]*类型的指针，取两次偏移，两次偏移值都为 0，
    // 因此返回的即为 x[0]的地址。
    auto x0GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), x0GEP);
    auto x0Value = builder->create_load(x0GEP);
    auto x1Value = builder->create_imul(x0Value, CONST_INT(2));
    auto x1GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(x1Value, x1GEP);
    auto ret = builder->create_load(x1GEP);
    builder->create_ret(ret);
    std::cout << module->print();
    delete module;
    return 0;
}