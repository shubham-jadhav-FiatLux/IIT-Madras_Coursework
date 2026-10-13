; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: arg : [0,0]
; CHECK: j : [0,0]
; ModuleID = 'testcases/testcase6.c'
source_filename = "testcases/testcase6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(i32 noundef %0) #0 !dbg !10 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
  store i32 -20, ptr %3, align 4, !dbg !18
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
  store i32 -10, ptr %4, align 4, !dbg !20
  %5 = load i32, ptr %2, align 4, !dbg !21
  %6 = icmp sgt i32 %5, 0, !dbg !23
  br i1 %6, label %7, label %10, !dbg !23

7:                                                ; preds = %1
  %8 = load i32, ptr %3, align 4, !dbg !24
  %9 = add nsw i32 %8, 70, !dbg !24
  store i32 %9, ptr %3, align 4, !dbg !24
  br label %10, !dbg !26

10:                                               ; preds = %7, %1
  %11 = load i32, ptr %4, align 4, !dbg !27
  %12 = add nsw i32 %11, 10, !dbg !27
  store i32 %12, ptr %4, align 4, !dbg !27
  %13 = load i32, ptr %3, align 4, !dbg !28
  %14 = load i32, ptr %4, align 4, !dbg !29
  %15 = mul nsw i32 %13, %14, !dbg !30
  store i32 %15, ptr %3, align 4, !dbg !31
  %16 = load i32, ptr %2, align 4, !dbg !32
  %17 = load i32, ptr %4, align 4, !dbg !33
  %18 = mul nsw i32 %16, %17, !dbg !34
  store i32 %18, ptr %2, align 4, !dbg !35
  ret void, !dbg !36
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !37 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !40
  ret i32 0, !dbg !41
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase6.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "7b6606ef96ef1a8e90f73c74d9d108c8")
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
!17 = !DILocalVariable(name: "i", scope: !10, file: !1, line: 2, type: !13)
!18 = !DILocation(line: 2, column: 9, scope: !10)
!19 = !DILocalVariable(name: "j", scope: !10, file: !1, line: 2, type: !13)
!20 = !DILocation(line: 2, column: 18, scope: !10)
!21 = !DILocation(line: 3, column: 9, scope: !22)
!22 = distinct !DILexicalBlock(scope: !10, file: !1, line: 3, column: 9)
!23 = !DILocation(line: 3, column: 13, scope: !22)
!24 = !DILocation(line: 4, column: 11, scope: !25)
!25 = distinct !DILexicalBlock(scope: !22, file: !1, line: 3, column: 18)
!26 = !DILocation(line: 5, column: 5, scope: !25)
!27 = !DILocation(line: 6, column: 7, scope: !10)
!28 = !DILocation(line: 7, column: 9, scope: !10)
!29 = !DILocation(line: 7, column: 13, scope: !10)
!30 = !DILocation(line: 7, column: 11, scope: !10)
!31 = !DILocation(line: 7, column: 7, scope: !10)
!32 = !DILocation(line: 8, column: 11, scope: !10)
!33 = !DILocation(line: 8, column: 17, scope: !10)
!34 = !DILocation(line: 8, column: 15, scope: !10)
!35 = !DILocation(line: 8, column: 9, scope: !10)
!36 = !DILocation(line: 9, column: 1, scope: !10)
!37 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !38, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !0)
!38 = !DISubroutineType(types: !39)
!39 = !{!13}
!40 = !DILocation(line: 12, column: 5, scope: !37)
!41 = !DILocation(line: 13, column: 5, scope: !37)
