; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: a : [-10,-10]
; CHECK: b : [-2147483648,2147482647]
; CHECK: c : [-2147483648,-2147483648]
; ModuleID = 'testcases/testcase4.c'
source_filename = "testcases/testcase4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(i32 noundef %0) #0 !dbg !10 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
  store i32 10, ptr %3, align 4, !dbg !18
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
  store i32 1000, ptr %4, align 4, !dbg !20
    #dbg_declare(ptr %5, !21, !DIExpression(), !22)
  store i32 -2147483640, ptr %5, align 4, !dbg !22
  %6 = load i32, ptr %3, align 4, !dbg !23
  %7 = sub nsw i32 %6, 20, !dbg !24
  store i32 %7, ptr %3, align 4, !dbg !25
  %8 = load i32, ptr %2, align 4, !dbg !26
  %9 = load i32, ptr %4, align 4, !dbg !27
  %10 = sub nsw i32 %8, %9, !dbg !28
  store i32 %10, ptr %4, align 4, !dbg !29
  %11 = load i32, ptr %5, align 4, !dbg !30
  %12 = sub nsw i32 %11, 10, !dbg !31
  store i32 %12, ptr %5, align 4, !dbg !32
  ret void, !dbg !33
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !34 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !37
  ret i32 0, !dbg !38
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase4.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "eb50d62ec3c51a4f64384160314dbbdd")
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
!22 = !DILocation(line: 2, column: 27, scope: !10)
!23 = !DILocation(line: 3, column: 9, scope: !10)
!24 = !DILocation(line: 3, column: 11, scope: !10)
!25 = !DILocation(line: 3, column: 7, scope: !10)
!26 = !DILocation(line: 4, column: 9, scope: !10)
!27 = !DILocation(line: 4, column: 15, scope: !10)
!28 = !DILocation(line: 4, column: 13, scope: !10)
!29 = !DILocation(line: 4, column: 7, scope: !10)
!30 = !DILocation(line: 5, column: 9, scope: !10)
!31 = !DILocation(line: 5, column: 11, scope: !10)
!32 = !DILocation(line: 5, column: 7, scope: !10)
!33 = !DILocation(line: 6, column: 1, scope: !10)
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !35, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !0)
!35 = !DISubroutineType(types: !36)
!36 = !{!13}
!37 = !DILocation(line: 9, column: 5, scope: !34)
!38 = !DILocation(line: 10, column: 5, scope: !34)
