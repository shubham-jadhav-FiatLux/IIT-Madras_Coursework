; ModuleID = 'testcases/testcase6.c'
source_filename = "testcases/testcase6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
    #dbg_declare(ptr %5, !21, !DIExpression(), !23)
    #dbg_declare(ptr %6, !24, !DIExpression(), !25)
    #dbg_declare(ptr %7, !26, !DIExpression(), !27)
    #dbg_declare(ptr %8, !28, !DIExpression(), !30)
  %9 = load ptr, ptr %7, align 8, !dbg !31
  %10 = load ptr, ptr %8, align 8, !dbg !32
  store ptr %9, ptr %10, align 8, !dbg !33
  store ptr %3, ptr %6, align 8, !dbg !34
  %11 = load ptr, ptr %8, align 8, !dbg !35
  %12 = load ptr, ptr %11, align 8, !dbg !36
  store ptr %12, ptr %6, align 8, !dbg !37
  %13 = load ptr, ptr %7, align 8, !dbg !38
  %14 = icmp eq ptr %13, null, !dbg !40
  br i1 %14, label %15, label %16, !dbg !40

15:                                               ; preds = %0
  store ptr %5, ptr %8, align 8, !dbg !41
  br label %17, !dbg !42

16:                                               ; preds = %0
  store ptr %3, ptr %8, align 8, !dbg !43
  br label %17

17:                                               ; preds = %16, %15
  store ptr %2, ptr %5, align 8, !dbg !44
  %18 = load ptr, ptr %7, align 8, !dbg !45
  %19 = load ptr, ptr %8, align 8, !dbg !46
  store ptr %18, ptr %19, align 8, !dbg !47
  %20 = load ptr, ptr %8, align 8, !dbg !48
  store ptr %20, ptr %7, align 8, !dbg !49
  store ptr %4, ptr %7, align 8, !dbg !50
  ret i32 0, !dbg !51
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase6.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "75e96c105d201dc7480c0ebb92be9f7d")
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
!19 = !DILocalVariable(name: "z", scope: !10, file: !1, line: 3, type: !13)
!20 = !DILocation(line: 3, column: 13, scope: !10)
!21 = !DILocalVariable(name: "a", scope: !10, file: !1, line: 4, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!23 = !DILocation(line: 4, column: 10, scope: !10)
!24 = !DILocalVariable(name: "b", scope: !10, file: !1, line: 4, type: !22)
!25 = !DILocation(line: 4, column: 14, scope: !10)
!26 = !DILocalVariable(name: "c", scope: !10, file: !1, line: 4, type: !22)
!27 = !DILocation(line: 4, column: 18, scope: !10)
!28 = !DILocalVariable(name: "p", scope: !10, file: !1, line: 5, type: !29)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!30 = !DILocation(line: 5, column: 11, scope: !10)
!31 = !DILocation(line: 7, column: 10, scope: !10)
!32 = !DILocation(line: 7, column: 6, scope: !10)
!33 = !DILocation(line: 7, column: 8, scope: !10)
!34 = !DILocation(line: 8, column: 7, scope: !10)
!35 = !DILocation(line: 9, column: 10, scope: !10)
!36 = !DILocation(line: 9, column: 9, scope: !10)
!37 = !DILocation(line: 9, column: 7, scope: !10)
!38 = !DILocation(line: 11, column: 8, scope: !39)
!39 = distinct !DILexicalBlock(scope: !10, file: !1, line: 11, column: 8)
!40 = !DILocation(line: 11, column: 9, scope: !39)
!41 = !DILocation(line: 12, column: 11, scope: !39)
!42 = !DILocation(line: 12, column: 9, scope: !39)
!43 = !DILocation(line: 14, column: 11, scope: !39)
!44 = !DILocation(line: 16, column: 7, scope: !10)
!45 = !DILocation(line: 17, column: 10, scope: !10)
!46 = !DILocation(line: 17, column: 6, scope: !10)
!47 = !DILocation(line: 17, column: 8, scope: !10)
!48 = !DILocation(line: 18, column: 9, scope: !10)
!49 = !DILocation(line: 18, column: 7, scope: !10)
!50 = !DILocation(line: 19, column: 7, scope: !10)
!51 = !DILocation(line: 21, column: 5, scope: !10)
