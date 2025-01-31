
define dso_local i32 @main() {
    %1 = alloca float, align 4
    %2 = fptrunc double 5.555 to float
    store float %2, float* %1, align 4
    %3 = load float, float* %1
    %4 = fcmp uge float %3, 1.
    br i1 %4, label %success, label %fail
success:
    ret i32 233
fail:
    ret i32 0
}