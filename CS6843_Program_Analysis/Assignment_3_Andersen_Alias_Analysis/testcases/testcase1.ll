; ModuleID = 'testcases/testcase1.c'
source_filename = "testcases/testcase1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
    #dbg_declare(ptr %4, !19, !DIExpression(), !21)
    #dbg_declare(ptr %5, !22, !DIExpression(), !23)
    #dbg_declare(ptr %6, !24, !DIExpression(), !26)
  store ptr %2, ptr %5, align 8, !dbg !27
  %7 = load ptr, ptr %5, align 8, !dbg !28
  store ptr %7, ptr %4, align 8, !dbg !29
  %8 = load ptr, ptr %6, align 8, !dbg !30
  %9 = load ptr, ptr %8, align 8, !dbg !31
  store ptr %9, ptr %5, align 8, !dbg !32
  %10 = load ptr, ptr %4, align 8, !dbg !33
  store ptr %10, ptr %6, align 8, !dbg !34
  ret i32 0, !dbg !35
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase1.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "88f7675267cb408fb2bc52899c6bcb58")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !11, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "x", scope: !10, file: !1, line: 3, type: !13)
!16 = !DILocation(line: 3, column: 9, scope: !10)
!17 = !DILocalVariable(name: "y", scope: !10, file: !1, line: 3, type: !13)
!18 = !DILocation(line: 3, column: 11, scope: !10)
!19 = !DILocalVariable(name: "a", scope: !10, file: !1, line: 4, type: !20)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!21 = !DILocation(line: 4, column: 10, scope: !10)
!22 = !DILocalVariable(name: "b", scope: !10, file: !1, line: 5, type: !20)
!23 = !DILocation(line: 5, column: 10, scope: !10)
!24 = !DILocalVariable(name: "h", scope: !10, file: !1, line: 6, type: !25)
!25 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!26 = !DILocation(line: 6, column: 11, scope: !10)
!27 = !DILocation(line: 8, column: 7, scope: !10)
!28 = !DILocation(line: 9, column: 9, scope: !10)
!29 = !DILocation(line: 9, column: 7, scope: !10)
!30 = !DILocation(line: 10, column: 10, scope: !10)
!31 = !DILocation(line: 10, column: 9, scope: !10)
!32 = !DILocation(line: 10, column: 7, scope: !10)
!33 = !DILocation(line: 11, column: 9, scope: !10)
!34 = !DILocation(line: 11, column: 7, scope: !10)
!35 = !DILocation(line: 13, column: 5, scope: !10)
