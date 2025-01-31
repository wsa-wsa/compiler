
define dso_local i32 @main(){
    %1 = alloca i32, align 4
    %2 = alloca i32, align 4
    store i32 10, i32* %1
    store i32 0,  i32* %2
    br label %loop
loop:
    %3 = load i32, i32* %2
    %4 = icmp slt i32 %3, 10
    br i1 %4, label %body, label %next
body:
    %5 = add i32 %3, 1
    store i32 %5, i32* %2
    %6 = load i32, i32* %1
    %7 = add i32 %6, %5
    store i32 %7, i32* %1
    br label %loop
next:
    %8 = load i32, i32* %1
    ret i32 %8
}