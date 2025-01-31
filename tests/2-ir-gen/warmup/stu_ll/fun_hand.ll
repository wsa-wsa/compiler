define dso_local i32 @callee(i32 %0) {
    ; 函数需要多留一个寄存器
    %2 = mul i32 %0, 2
    ret i32 %2
}
define dso_local i32 @main() {
    %1 = call i32 @callee(i32 110)
    ret i32 %1
}