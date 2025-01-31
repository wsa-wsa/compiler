; ModuleID = 'main_module'
; source_filename = "main_module"

; target triple = "x86_64-pc-linux-gnu"

define dso_local i32 @main() #0 {
  ; 此处 main function 仅有 1 个 basicblock
  ; basicblock 由一系列 instruction 组成
; entry:
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 1, i32* %2, align 4
  store i32 2, i32* %3, align 4
  %4 = load i32, i32* %2, align 4
  %5 = load i32, i32* %3, align 4
  %6 = add nsw i32 %4, %5
  ret i32 %6
}

; attributes #0 = { noinline nounwind optnone uwtable }