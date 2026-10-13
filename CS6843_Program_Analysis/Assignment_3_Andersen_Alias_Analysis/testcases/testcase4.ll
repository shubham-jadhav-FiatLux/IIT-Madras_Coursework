; ModuleID = 'testcases/testcase4.c'
source_filename = "testcases/testcase4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global ptr null, align 8, !dbg !0
@b = dso_local global ptr null, align 8, !dbg !5

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !17 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !21, !DIExpression(), !22)
    #dbg_declare(ptr %3, !23, !DIExpression(), !24)
  store ptr %2, ptr @a, align 8, !dbg !25
  %4 = load ptr, ptr @a, align 8, !dbg !26
  store ptr %4, ptr @b, align 8, !dbg !27
  ret i32 0, !dbg !28
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "testcases/testcase4.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "7ba88afe4d5829c75327023fbb9244ae")
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
!17 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 2, type: !18, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{!8}
!20 = !{}
!21 = !DILocalVariable(name: "x", scope: !17, file: !3, line: 4, type: !8)
!22 = !DILocation(line: 4, column: 9, scope: !17)
!23 = !DILocalVariable(name: "y", scope: !17, file: !3, line: 4, type: !8)
!24 = !DILocation(line: 4, column: 12, scope: !17)
!25 = !DILocation(line: 5, column: 7, scope: !17)
!26 = !DILocation(line: 6, column: 9, scope: !17)
!27 = !DILocation(line: 6, column: 7, scope: !17)
!28 = !DILocation(line: 7, column: 5, scope: !17)
