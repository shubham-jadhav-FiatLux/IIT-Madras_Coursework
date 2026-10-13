; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: a : [400,400]
; CHECK: c : [-400,-400]
; CHECK: e : [400,400]
; CHECK: g : [-2147483648,-2147483648]
; CHECK: h : [2147483647,2147483647]
; ModuleID = 'testcases/testcase5.c'
source_filename = "testcases/testcase5.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(i32 noundef %0) #0 !dbg !10 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
  store i32 10, ptr %3, align 4, !dbg !18
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
  store i32 40, ptr %4, align 4, !dbg !20
    #dbg_declare(ptr %5, !21, !DIExpression(), !22)
  store i32 -10, ptr %5, align 4, !dbg !22
    #dbg_declare(ptr %6, !23, !DIExpression(), !24)
  store i32 40, ptr %6, align 4, !dbg !24
    #dbg_declare(ptr %7, !25, !DIExpression(), !26)
  store i32 -10, ptr %7, align 4, !dbg !26
    #dbg_declare(ptr %8, !27, !DIExpression(), !28)
  store i32 -40, ptr %8, align 4, !dbg !28
    #dbg_declare(ptr %9, !29, !DIExpression(), !30)
  store i32 -214748369, ptr %9, align 4, !dbg !30
    #dbg_declare(ptr %10, !31, !DIExpression(), !32)
  store i32 214748369, ptr %10, align 4, !dbg !32
  %11 = load i32, ptr %3, align 4, !dbg !33
  %12 = load i32, ptr %4, align 4, !dbg !34
  %13 = mul nsw i32 %11, %12, !dbg !35
  store i32 %13, ptr %3, align 4, !dbg !36
  %14 = load i32, ptr %5, align 4, !dbg !37
  %15 = load i32, ptr %6, align 4, !dbg !38
  %16 = mul nsw i32 %14, %15, !dbg !39
  store i32 %16, ptr %5, align 4, !dbg !40
  %17 = load i32, ptr %7, align 4, !dbg !41
  %18 = load i32, ptr %8, align 4, !dbg !42
  %19 = mul nsw i32 %17, %18, !dbg !43
  store i32 %19, ptr %7, align 4, !dbg !44
  %20 = load i32, ptr %9, align 4, !dbg !45
  %21 = mul nsw i32 %20, 10, !dbg !46
  store i32 %21, ptr %9, align 4, !dbg !47
  %22 = load i32, ptr %10, align 4, !dbg !48
  %23 = mul nsw i32 %22, 10, !dbg !49
  store i32 %23, ptr %10, align 4, !dbg !50
  ret void, !dbg !51
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !52 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !55
  ret i32 0, !dbg !56
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase5.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "863385b0c5d86e5937d269e30bf27ba9")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!10 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "arg", arg: 1, scope: !10, file: !1, line: 1, type: !13)
!16 = !DILocation(line: 1, column: 15, scope: !10)
!17 = !DILocalVariable(name: "a", scope: !10, file: !1, line: 2, type: !13)
!18 = !DILocation(line: 2, column: 9, scope: !10)
!19 = !DILocalVariable(name: "b", scope: !10, file: !1, line: 2, type: !13)
!20 = !DILocation(line: 2, column: 17, scope: !10)
!21 = !DILocalVariable(name: "c", scope: !10, file: !1, line: 2, type: !13)
!22 = !DILocation(line: 2, column: 25, scope: !10)
!23 = !DILocalVariable(name: "d", scope: !10, file: !1, line: 2, type: !13)
!24 = !DILocation(line: 2, column: 34, scope: !10)
!25 = !DILocalVariable(name: "e", scope: !10, file: !1, line: 2, type: !13)
!26 = !DILocation(line: 2, column: 42, scope: !10)
!27 = !DILocalVariable(name: "f", scope: !10, file: !1, line: 2, type: !13)
!28 = !DILocation(line: 2, column: 51, scope: !10)
!29 = !DILocalVariable(name: "g", scope: !10, file: !1, line: 2, type: !13)
!30 = !DILocation(line: 2, column: 60, scope: !10)
!31 = !DILocalVariable(name: "h", scope: !10, file: !1, line: 2, type: !13)
!32 = !DILocation(line: 2, column: 76, scope: !10)
!33 = !DILocation(line: 3, column: 9, scope: !10)
!34 = !DILocation(line: 3, column: 13, scope: !10)
!35 = !DILocation(line: 3, column: 11, scope: !10)
!36 = !DILocation(line: 3, column: 7, scope: !10)
!37 = !DILocation(line: 4, column: 9, scope: !10)
!38 = !DILocation(line: 4, column: 13, scope: !10)
!39 = !DILocation(line: 4, column: 11, scope: !10)
!40 = !DILocation(line: 4, column: 7, scope: !10)
!41 = !DILocation(line: 5, column: 9, scope: !10)
!42 = !DILocation(line: 5, column: 13, scope: !10)
!43 = !DILocation(line: 5, column: 11, scope: !10)
!44 = !DILocation(line: 5, column: 7, scope: !10)
!45 = !DILocation(line: 6, column: 9, scope: !10)
!46 = !DILocation(line: 6, column: 11, scope: !10)
!47 = !DILocation(line: 6, column: 7, scope: !10)
!48 = !DILocation(line: 7, column: 9, scope: !10)
!49 = !DILocation(line: 7, column: 11, scope: !10)
!50 = !DILocation(line: 7, column: 7, scope: !10)
!51 = !DILocation(line: 8, column: 1, scope: !10)
!52 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !53, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !0)
!53 = !DISubroutineType(types: !54)
!54 = !{!13}
!55 = !DILocation(line: 11, column: 5, scope: !52)
!56 = !DILocation(line: 12, column: 5, scope: !52)
