; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: i : [-2147483648,-20]
; CHECK: j : [-2147483648,2147483647]
; ModuleID = 'testcases/testcase13.c'
source_filename = "testcases/testcase13.c"
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
  store i32 0, ptr %3, align 4, !dbg !18
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
  store i32 0, ptr %4, align 4, !dbg !20
  store i32 20, ptr %3, align 4, !dbg !21
  br label %5, !dbg !23

5:                                                ; preds = %15, %1
  %6 = load i32, ptr %3, align 4, !dbg !24
  %7 = icmp sgt i32 %6, -20, !dbg !26
  br i1 %7, label %8, label %18, !dbg !27

8:                                                ; preds = %5
  %9 = load i32, ptr %2, align 4, !dbg !28
  %10 = icmp slt i32 %9, 10, !dbg !31
  br i1 %10, label %11, label %14, !dbg !31

11:                                               ; preds = %8
  %12 = load i32, ptr %2, align 4, !dbg !32
  %13 = add nsw i32 %12, 1, !dbg !34
  store i32 %13, ptr %4, align 4, !dbg !35
  br label %14, !dbg !36

14:                                               ; preds = %11, %8
  br label %15, !dbg !37

15:                                               ; preds = %14
  %16 = load i32, ptr %3, align 4, !dbg !38
  %17 = add nsw i32 %16, -1, !dbg !38
  store i32 %17, ptr %3, align 4, !dbg !38
  br label %5, !dbg !39, !llvm.loop !40

18:                                               ; preds = %5
  ret void, !dbg !43
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !44 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !47
  ret i32 0, !dbg !48
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase13.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "3c3f4f5e662d5d9b10c4a497fb57fa2b")
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
!20 = !DILocation(line: 2, column: 16, scope: !10)
!21 = !DILocation(line: 3, column: 12, scope: !22)
!22 = distinct !DILexicalBlock(scope: !10, file: !1, line: 3, column: 5)
!23 = !DILocation(line: 3, column: 10, scope: !22)
!24 = !DILocation(line: 3, column: 18, scope: !25)
!25 = distinct !DILexicalBlock(scope: !22, file: !1, line: 3, column: 5)
!26 = !DILocation(line: 3, column: 20, scope: !25)
!27 = !DILocation(line: 3, column: 5, scope: !22)
!28 = !DILocation(line: 4, column: 13, scope: !29)
!29 = distinct !DILexicalBlock(scope: !30, file: !1, line: 4, column: 13)
!30 = distinct !DILexicalBlock(scope: !25, file: !1, line: 3, column: 32)
!31 = !DILocation(line: 4, column: 17, scope: !29)
!32 = !DILocation(line: 5, column: 17, scope: !33)
!33 = distinct !DILexicalBlock(scope: !29, file: !1, line: 4, column: 23)
!34 = !DILocation(line: 5, column: 21, scope: !33)
!35 = !DILocation(line: 5, column: 15, scope: !33)
!36 = !DILocation(line: 6, column: 9, scope: !33)
!37 = !DILocation(line: 7, column: 5, scope: !30)
!38 = !DILocation(line: 3, column: 28, scope: !25)
!39 = !DILocation(line: 3, column: 5, scope: !25)
!40 = distinct !{!40, !27, !41, !42}
!41 = !DILocation(line: 7, column: 5, scope: !22)
!42 = !{!"llvm.loop.mustprogress"}
!43 = !DILocation(line: 8, column: 1, scope: !10)
!44 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !45, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !0)
!45 = !DISubroutineType(types: !46)
!46 = !{!13}
!47 = !DILocation(line: 11, column: 5, scope: !44)
!48 = !DILocation(line: 12, column: 5, scope: !44)
