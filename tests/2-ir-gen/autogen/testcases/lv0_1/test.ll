; ModuleID = 'cminus'
source_filename = "/jhe/2024ustc-jianmu-compiler_impl/tests/2-ir-gen/autogen/testcases/lv0_1/decl_float_array.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op0 = alloca [10 x float]
  ret void
}
