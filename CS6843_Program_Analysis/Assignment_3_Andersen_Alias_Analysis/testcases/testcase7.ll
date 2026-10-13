; ModuleID = 'testcases/testcase7.c'
source_filename = "testcases/testcase7.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global ptr null, align 8, !dbg !0
@b = dso_local global ptr null, align 8, !dbg !5

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !17 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !21, !DIExpression(), !22)
    #dbg_declare(ptr %3, !23, !DIExpression(), !24)
  store ptr %2, ptr @a, align 8, !dbg !25
    #dbg_declare(ptr %4, !26, !DIExpression(), !28)
  store i32 0, ptr %4, align 4, !dbg !28
  br label %5, !dbg !29

5:                                                ; preds = %10, %0
  %6 = load i32, ptr %4, align 4, !dbg !30
  %7 = icmp slt i32 %6, 10, !dbg !32
  br i1 %7, label %8, label %13, !dbg !33

8:                                                ; preds = %5
  %9 = load ptr, ptr @a, align 8, !dbg !34
  store ptr %9, ptr @b, align 8, !dbg !35
  br label %10, !dbg !36

10:                                               ; preds = %8
  %11 = load i32, ptr %4, align 4, !dbg !37
  %12 = add nsw i32 %11, 1, !dbg !37
  store i32 %12, ptr %4, align 4, !dbg !37
  br label %5, !dbg !38, !llvm.loop !39

13:                                               ; preds = %5
  ret i32 0, !dbg !42
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "testcases/testcase7.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "4cd3ce1d04c2823a0951399927bea1ee")
!4 = !{!0, !5}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 1, type: !7, isLocal: false, isDefinition: true)
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !{i32 7, !"Dwarf Version", i32 5}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 8, !"PIC Level", i32 2}
!13 = !{i32 7, !"PIE Level", i32 2}
!14 = !{i32 7, !"uwtable", i32 2}
!15 = !{i32 7, !"frame-pointer", i32 2}
!16 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!17 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 4, type: !18, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{!8}
!20 = !{}
!21 = !DILocalVariable(name: "x", scope: !17, file: !3, line: 6, type: !8)
!22 = !DILocation(line: 6, column: 9, scope: !17)
!23 = !DILocalVariable(name: "y", scope: !17, file: !3, line: 6, type: !8)
!24 = !DILocation(line: 6, column: 12, scope: !17)
!25 = !DILocation(line: 7, column: 7, scope: !17)
!26 = !DILocalVariable(name: "i", scope: !27, file: !3, line: 9, type: !8)
!27 = distinct !DILexicalBlock(scope: !17, file: !3, line: 9, column: 5)
!28 = !DILocation(line: 9, column: 13, scope: !27)
!29 = !DILocation(line: 9, column: 9, scope: !27)
!30 = !DILocation(line: 9, column: 18, scope: !31)
!31 = distinct !DILexicalBlock(scope: !27, file: !3, line: 9, column: 5)
!32 = !DILocation(line: 9, column: 19, scope: !31)
!33 = !DILocation(line: 9, column: 5, scope: !27)
!34 = !DILocation(line: 10, column: 13, scope: !31)
!35 = !DILocation(line: 10, column: 11, scope: !31)
!36 = !DILocation(line: 10, column: 9, scope: !31)
!37 = !DILocation(line: 9, column: 24, scope: !31)
!38 = !DILocation(line: 9, column: 5, scope: !31)
!39 = distinct !{!39, !33, !40, !41}
!40 = !DILocation(line: 10, column: 13, scope: !27)
!41 = !{!"llvm.loop.mustprogress"}
!42 = !DILocation(line: 12, column: 5, scope: !17)
