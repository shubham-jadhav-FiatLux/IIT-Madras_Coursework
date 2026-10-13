; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: i : [0,20]
; CHECK: n : [-2147483648,2147483647]
; ModuleID = 'testcases/testcase12.c'
source_filename = "testcases/testcase12.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [23 x i8] c"Enter size of square:\0A\00", align 1, !dbg !0
@.str.1 = private unnamed_addr constant [3 x i8] c"%d\00", align 1, !dbg !7
@.str.2 = private unnamed_addr constant [33 x i8] c"Cannot handle n greater than 10\0A\00", align 1, !dbg !12
@.str.3 = private unnamed_addr constant [26 x i8] c"Cannot handle negative n\0A\00", align 1, !dbg !17

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(i32 noundef %0) #0 !dbg !32 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
    #dbg_declare(ptr %2, !37, !DIExpression(), !38)
    #dbg_declare(ptr %3, !39, !DIExpression(), !40)
  store i32 10, ptr %3, align 4, !dbg !40
    #dbg_declare(ptr %4, !41, !DIExpression(), !42)
  store i32 0, ptr %4, align 4, !dbg !42
  %5 = call i32 (ptr, ...) @printf(ptr noundef @.str), !dbg !43
  %6 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str.1, ptr noundef %3), !dbg !44
  %7 = load i32, ptr %3, align 4, !dbg !45
  %8 = icmp sgt i32 %7, 10, !dbg !47
  br i1 %8, label %9, label %11, !dbg !47

9:                                                ; preds = %1
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.2), !dbg !48
  br label %24, !dbg !50

11:                                               ; preds = %1
  %12 = load i32, ptr %3, align 4, !dbg !51
  %13 = icmp slt i32 %12, 0, !dbg !53
  br i1 %13, label %14, label %16, !dbg !53

14:                                               ; preds = %11
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str.3), !dbg !54
  br label %24, !dbg !56

16:                                               ; preds = %11
  br label %17

17:                                               ; preds = %16
  %18 = load i32, ptr %3, align 4, !dbg !57
  store i32 %18, ptr %4, align 4, !dbg !58
  %19 = load i32, ptr %2, align 4, !dbg !59
  %20 = icmp sgt i32 %19, 3, !dbg !61
  br i1 %20, label %21, label %24, !dbg !61

21:                                               ; preds = %17
  %22 = load i32, ptr %4, align 4, !dbg !62
  %23 = add nsw i32 %22, 10, !dbg !63
  store i32 %23, ptr %4, align 4, !dbg !64
  br label %24, !dbg !65

24:                                               ; preds = %9, %14, %21, %17
  ret void, !dbg !66
}

declare i32 @printf(ptr noundef, ...) #1

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !67 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !70
  ret i32 0, !dbg !71
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!22}
!llvm.module.flags = !{!24, !25, !26, !27, !28, !29, !30}
!llvm.ident = !{!31}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 5, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase12.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "3b145830411daa712c69d421ebd923cd")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 184, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 23)
!7 = !DIGlobalVariableExpression(var: !8, expr: !DIExpression())
!8 = distinct !DIGlobalVariable(scope: null, file: !2, line: 6, type: !9, isLocal: true, isDefinition: true)
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 24, elements: !10)
!10 = !{!11}
!11 = !DISubrange(count: 3)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(scope: null, file: !2, line: 8, type: !14, isLocal: true, isDefinition: true)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 264, elements: !15)
!15 = !{!16}
!16 = !DISubrange(count: 33)
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(scope: null, file: !2, line: 11, type: !19, isLocal: true, isDefinition: true)
!19 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 208, elements: !20)
!20 = !{!21}
!21 = !DISubrange(count: 26)
!22 = distinct !DICompileUnit(language: DW_LANG_C11, file: !2, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !23, splitDebugInlining: false, nameTableKind: None)
!23 = !{!0, !7, !12, !17}
!24 = !{i32 7, !"Dwarf Version", i32 5}
!25 = !{i32 2, !"Debug Info Version", i32 3}
!26 = !{i32 1, !"wchar_size", i32 4}
!27 = !{i32 8, !"PIC Level", i32 2}
!28 = !{i32 7, !"PIE Level", i32 2}
!29 = !{i32 7, !"uwtable", i32 2}
!30 = !{i32 7, !"frame-pointer", i32 2}
!31 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!32 = distinct !DISubprogram(name: "test", scope: !2, file: !2, line: 3, type: !33, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !22, retainedNodes: !36)
!33 = !DISubroutineType(types: !34)
!34 = !{null, !35}
!35 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!36 = !{}
!37 = !DILocalVariable(name: "arg", arg: 1, scope: !32, file: !2, line: 3, type: !35)
!38 = !DILocation(line: 3, column: 15, scope: !32)
!39 = !DILocalVariable(name: "n", scope: !32, file: !2, line: 4, type: !35)
!40 = !DILocation(line: 4, column: 9, scope: !32)
!41 = !DILocalVariable(name: "i", scope: !32, file: !2, line: 4, type: !35)
!42 = !DILocation(line: 4, column: 17, scope: !32)
!43 = !DILocation(line: 5, column: 5, scope: !32)
!44 = !DILocation(line: 6, column: 5, scope: !32)
!45 = !DILocation(line: 7, column: 9, scope: !46)
!46 = distinct !DILexicalBlock(scope: !32, file: !2, line: 7, column: 9)
!47 = !DILocation(line: 7, column: 11, scope: !46)
!48 = !DILocation(line: 8, column: 9, scope: !49)
!49 = distinct !DILexicalBlock(scope: !46, file: !2, line: 7, column: 17)
!50 = !DILocation(line: 9, column: 9, scope: !49)
!51 = !DILocation(line: 10, column: 16, scope: !52)
!52 = distinct !DILexicalBlock(scope: !46, file: !2, line: 10, column: 16)
!53 = !DILocation(line: 10, column: 18, scope: !52)
!54 = !DILocation(line: 11, column: 9, scope: !55)
!55 = distinct !DILexicalBlock(scope: !52, file: !2, line: 10, column: 23)
!56 = !DILocation(line: 12, column: 9, scope: !55)
!57 = !DILocation(line: 14, column: 9, scope: !32)
!58 = !DILocation(line: 14, column: 7, scope: !32)
!59 = !DILocation(line: 15, column: 9, scope: !60)
!60 = distinct !DILexicalBlock(scope: !32, file: !2, line: 15, column: 9)
!61 = !DILocation(line: 15, column: 13, scope: !60)
!62 = !DILocation(line: 16, column: 13, scope: !60)
!63 = !DILocation(line: 16, column: 15, scope: !60)
!64 = !DILocation(line: 16, column: 11, scope: !60)
!65 = !DILocation(line: 16, column: 9, scope: !60)
!66 = !DILocation(line: 17, column: 1, scope: !32)
!67 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 19, type: !68, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !22)
!68 = !DISubroutineType(types: !69)
!69 = !{!35}
!70 = !DILocation(line: 20, column: 5, scope: !67)
!71 = !DILocation(line: 21, column: 5, scope: !67)
