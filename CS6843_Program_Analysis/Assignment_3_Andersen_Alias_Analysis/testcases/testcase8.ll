; ModuleID = 'testcases/testcase8.c'
source_filename = "testcases/testcase8.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(ptr noundef %0, ptr noundef %1) #0 !dbg !10 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
    #dbg_declare(ptr %3, !16, !DIExpression(), !17)
  store ptr %1, ptr %4, align 8
    #dbg_declare(ptr %4, !18, !DIExpression(), !19)
    #dbg_declare(ptr %5, !20, !DIExpression(), !21)
    #dbg_declare(ptr %6, !22, !DIExpression(), !23)
  store ptr %6, ptr %3, align 8, !dbg !24
  %7 = load ptr, ptr %3, align 8, !dbg !25
  store ptr %7, ptr %4, align 8, !dbg !26
  br label %8, !dbg !27

8:                                                ; preds = %9, %2
  store i32 0, ptr %6, align 4, !dbg !28
  br i1 false, label %9, label %11, !dbg !27

9:                                                ; preds = %8
  %10 = load ptr, ptr %4, align 8, !dbg !29
  store ptr %10, ptr %5, align 8, !dbg !30
  br label %8, !dbg !27, !llvm.loop !31

11:                                               ; preds = %8
  ret i32 0, !dbg !33
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase8.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "0d3c449449b27033842a9e4781200f4f")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!10 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !11, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !14, !14}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!15 = !{}
!16 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 1, type: !14)
!17 = !DILocation(line: 1, column: 15, scope: !10)
!18 = !DILocalVariable(name: "b", arg: 2, scope: !10, file: !1, line: 1, type: !14)
!19 = !DILocation(line: 1, column: 23, scope: !10)
!20 = !DILocalVariable(name: "c", scope: !10, file: !1, line: 3, type: !14)
!21 = !DILocation(line: 3, column: 10, scope: !10)
!22 = !DILocalVariable(name: "x", scope: !10, file: !1, line: 4, type: !13)
!23 = !DILocation(line: 4, column: 9, scope: !10)
!24 = !DILocation(line: 7, column: 7, scope: !10)
!25 = !DILocation(line: 8, column: 9, scope: !10)
!26 = !DILocation(line: 8, column: 7, scope: !10)
!27 = !DILocation(line: 9, column: 5, scope: !10)
!28 = !DILocation(line: 9, column: 12, scope: !10)
!29 = !DILocation(line: 10, column: 13, scope: !10)
!30 = !DILocation(line: 10, column: 11, scope: !10)
!31 = distinct !{!31, !27, !29, !32}
!32 = !{!"llvm.loop.mustprogress"}
!33 = !DILocation(line: 12, column: 5, scope: !10)
