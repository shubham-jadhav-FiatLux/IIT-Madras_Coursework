; RUN: opt -load-pass-plugin ../build/analysis/a2Pass.so -passes='range-analysis' -disable-output %s 2>&1 | FileCheck %s
; CHECK: Function test
; CHECK: j : [10,10]
; ModuleID = 'testcases/testcase14.c'
source_filename = "testcases/testcase14.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(i32 noundef %0) #0 !dbg !17 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
    #dbg_declare(ptr %2, !22, !DIExpression(), !23)
    #dbg_declare(ptr %3, !24, !DIExpression(), !25)
  store i32 0, ptr %3, align 4, !dbg !25
    #dbg_declare(ptr %4, !26, !DIExpression(), !27)
  store i32 10, ptr %4, align 4, !dbg !27
  store i32 0, ptr %3, align 4, !dbg !28
  br label %5, !dbg !30

5:                                                ; preds = %17, %1
  %6 = load i32, ptr %3, align 4, !dbg !31
  %7 = icmp slt i32 %6, 10, !dbg !33
  br i1 %7, label %8, label %20, !dbg !34

8:                                                ; preds = %5
  %9 = load i32, ptr %4, align 4, !dbg !35
  %10 = add nsw i32 %9, 1, !dbg !35
  store i32 %10, ptr %4, align 4, !dbg !35
  %11 = load i32, ptr %4, align 4, !dbg !37
  %12 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %11), !dbg !38
  %13 = load i32, ptr %4, align 4, !dbg !39
  %14 = sub nsw i32 %13, 1, !dbg !39
  store i32 %14, ptr %4, align 4, !dbg !39
  %15 = load i32, ptr %4, align 4, !dbg !40
  %16 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %15), !dbg !41
  br label %17, !dbg !42

17:                                               ; preds = %8
  %18 = load i32, ptr %3, align 4, !dbg !43
  %19 = add nsw i32 %18, 1, !dbg !43
  store i32 %19, ptr %3, align 4, !dbg !43
  br label %5, !dbg !44, !llvm.loop !45

20:                                               ; preds = %5
  ret void, !dbg !48
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !49 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @test(i32 noundef 10), !dbg !52
  ret i32 0, !dbg !53
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 7, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase14.c", directory: "/root/data/MyAssignments/RangeAnalysis/Solution/A2Pass", checksumkind: CSK_MD5, checksum: "f90cf2c8e536f9a5c98b3d0c3e5f4411")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 32, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 4)
!7 = distinct !DICompileUnit(language: DW_LANG_C11, file: !2, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !8, splitDebugInlining: false, nameTableKind: None)
!8 = !{!0}
!9 = !{i32 7, !"Dwarf Version", i32 5}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 8, !"PIC Level", i32 2}
!13 = !{i32 7, !"PIE Level", i32 2}
!14 = !{i32 7, !"uwtable", i32 2}
!15 = !{i32 7, !"frame-pointer", i32 2}
!16 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!17 = distinct !DISubprogram(name: "test", scope: !2, file: !2, line: 3, type: !18, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!18 = !DISubroutineType(types: !19)
!19 = !{null, !20}
!20 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!21 = !{}
!22 = !DILocalVariable(name: "arg", arg: 1, scope: !17, file: !2, line: 3, type: !20)
!23 = !DILocation(line: 3, column: 15, scope: !17)
!24 = !DILocalVariable(name: "i", scope: !17, file: !2, line: 4, type: !20)
!25 = !DILocation(line: 4, column: 9, scope: !17)
!26 = !DILocalVariable(name: "j", scope: !17, file: !2, line: 4, type: !20)
!27 = !DILocation(line: 4, column: 16, scope: !17)
!28 = !DILocation(line: 5, column: 12, scope: !29)
!29 = distinct !DILexicalBlock(scope: !17, file: !2, line: 5, column: 5)
!30 = !DILocation(line: 5, column: 10, scope: !29)
!31 = !DILocation(line: 5, column: 17, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !2, line: 5, column: 5)
!33 = !DILocation(line: 5, column: 19, scope: !32)
!34 = !DILocation(line: 5, column: 5, scope: !29)
!35 = !DILocation(line: 6, column: 11, scope: !36)
!36 = distinct !DILexicalBlock(scope: !32, file: !2, line: 5, column: 30)
!37 = !DILocation(line: 7, column: 24, scope: !36)
!38 = !DILocation(line: 7, column: 9, scope: !36)
!39 = !DILocation(line: 8, column: 11, scope: !36)
!40 = !DILocation(line: 9, column: 24, scope: !36)
!41 = !DILocation(line: 9, column: 9, scope: !36)
!42 = !DILocation(line: 10, column: 5, scope: !36)
!43 = !DILocation(line: 5, column: 26, scope: !32)
!44 = !DILocation(line: 5, column: 5, scope: !32)
!45 = distinct !{!45, !34, !46, !47}
!46 = !DILocation(line: 10, column: 5, scope: !29)
!47 = !{!"llvm.loop.mustprogress"}
!48 = !DILocation(line: 11, column: 1, scope: !17)
!49 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 13, type: !50, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !7)
!50 = !DISubroutineType(types: !51)
!51 = !{!20}
!52 = !DILocation(line: 14, column: 5, scope: !49)
!53 = !DILocation(line: 15, column: 5, scope: !49)
