; ModuleID = 'testcases/testcase10.c'
source_filename = "testcases/testcase10.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !15, !DIExpression(), !16)
    #dbg_declare(ptr %3, !17, !DIExpression(), !18)
    #dbg_declare(ptr %4, !19, !DIExpression(), !20)
    #dbg_declare(ptr %5, !21, !DIExpression(), !22)
    #dbg_declare(ptr %6, !23, !DIExpression(), !24)
    #dbg_declare(ptr %7, !25, !DIExpression(), !26)
    #dbg_declare(ptr %8, !27, !DIExpression(), !29)
    #dbg_declare(ptr %9, !30, !DIExpression(), !31)
    #dbg_declare(ptr %10, !32, !DIExpression(), !33)
    #dbg_declare(ptr %11, !34, !DIExpression(), !35)
    #dbg_declare(ptr %12, !36, !DIExpression(), !37)
    #dbg_declare(ptr %13, !38, !DIExpression(), !39)
    #dbg_declare(ptr %14, !40, !DIExpression(), !42)
    #dbg_declare(ptr %15, !43, !DIExpression(), !44)
    #dbg_declare(ptr %16, !45, !DIExpression(), !46)
    #dbg_declare(ptr %17, !47, !DIExpression(), !48)
  store ptr %2, ptr %8, align 8, !dbg !49
  store ptr %3, ptr %9, align 8, !dbg !50
  store ptr %4, ptr %10, align 8, !dbg !51
  store ptr %5, ptr %11, align 8, !dbg !52
  store ptr %6, ptr %12, align 8, !dbg !53
  store ptr %7, ptr %13, align 8, !dbg !54
  store ptr %8, ptr %14, align 8, !dbg !55
  store ptr %9, ptr %15, align 8, !dbg !56
  store ptr %10, ptr %16, align 8, !dbg !57
  store ptr %11, ptr %17, align 8, !dbg !58
  %18 = load ptr, ptr %9, align 8, !dbg !59
  %19 = load ptr, ptr %14, align 8, !dbg !60
  store ptr %18, ptr %19, align 8, !dbg !61
  %20 = load ptr, ptr %10, align 8, !dbg !62
  %21 = load ptr, ptr %15, align 8, !dbg !63
  store ptr %20, ptr %21, align 8, !dbg !64
  %22 = load ptr, ptr %11, align 8, !dbg !65
  %23 = load ptr, ptr %16, align 8, !dbg !66
  store ptr %22, ptr %23, align 8, !dbg !67
  %24 = load ptr, ptr %12, align 8, !dbg !68
  %25 = load ptr, ptr %17, align 8, !dbg !69
  store ptr %24, ptr %25, align 8, !dbg !70
  %26 = load ptr, ptr %12, align 8, !dbg !71
  %27 = load ptr, ptr %14, align 8, !dbg !72
  store ptr %26, ptr %27, align 8, !dbg !73
  %28 = load ptr, ptr %13, align 8, !dbg !74
  %29 = load ptr, ptr %15, align 8, !dbg !75
  store ptr %28, ptr %29, align 8, !dbg !76
  %30 = load ptr, ptr %8, align 8, !dbg !77
  %31 = load ptr, ptr %16, align 8, !dbg !78
  store ptr %30, ptr %31, align 8, !dbg !79
  %32 = load ptr, ptr %10, align 8, !dbg !80
  %33 = load ptr, ptr %17, align 8, !dbg !81
  store ptr %32, ptr %33, align 8, !dbg !82
  store ptr %10, ptr %14, align 8, !dbg !83
  store ptr %11, ptr %15, align 8, !dbg !84
  %34 = load ptr, ptr %13, align 8, !dbg !85
  %35 = load ptr, ptr %14, align 8, !dbg !86
  store ptr %34, ptr %35, align 8, !dbg !87
  %36 = load ptr, ptr %8, align 8, !dbg !88
  %37 = load ptr, ptr %15, align 8, !dbg !89
  store ptr %36, ptr %37, align 8, !dbg !90
  store ptr %8, ptr %16, align 8, !dbg !91
  store ptr %9, ptr %17, align 8, !dbg !92
  store ptr %12, ptr %14, align 8, !dbg !93
  store ptr %13, ptr %15, align 8, !dbg !94
  %38 = load ptr, ptr %11, align 8, !dbg !95
  %39 = load ptr, ptr %14, align 8, !dbg !96
  store ptr %38, ptr %39, align 8, !dbg !97
  %40 = load ptr, ptr %9, align 8, !dbg !98
  %41 = load ptr, ptr %15, align 8, !dbg !99
  store ptr %40, ptr %41, align 8, !dbg !100
  store ptr %12, ptr %16, align 8, !dbg !101
  store ptr %13, ptr %17, align 8, !dbg !102
  %42 = load ptr, ptr %10, align 8, !dbg !103
  store ptr %42, ptr %8, align 8, !dbg !104
  %43 = load ptr, ptr %11, align 8, !dbg !105
  store ptr %43, ptr %9, align 8, !dbg !106
  %44 = load ptr, ptr %12, align 8, !dbg !107
  store ptr %44, ptr %10, align 8, !dbg !108
  %45 = load ptr, ptr %13, align 8, !dbg !109
  store ptr %45, ptr %11, align 8, !dbg !110
  %46 = load ptr, ptr %8, align 8, !dbg !111
  %47 = load ptr, ptr %14, align 8, !dbg !112
  store ptr %46, ptr %47, align 8, !dbg !113
  %48 = load ptr, ptr %9, align 8, !dbg !114
  %49 = load ptr, ptr %15, align 8, !dbg !115
  store ptr %48, ptr %49, align 8, !dbg !116
  ret i32 0, !dbg !117
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "testcases/testcase10.c", directory: "/root/llvm/Assignments_3_docker/template/assignment3", checksumkind: CSK_MD5, checksum: "6311808f2d83eb00b49c684bba57d4f3")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 20.1.7 (https://github.com/llvm/llvm-project.git 6146a88f60492b520a36f8f8f3231e15f3cc6082)"}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !11, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "x", scope: !10, file: !1, line: 2, type: !13)
!16 = !DILocation(line: 2, column: 9, scope: !10)
!17 = !DILocalVariable(name: "y", scope: !10, file: !1, line: 2, type: !13)
!18 = !DILocation(line: 2, column: 12, scope: !10)
!19 = !DILocalVariable(name: "z", scope: !10, file: !1, line: 2, type: !13)
!20 = !DILocation(line: 2, column: 15, scope: !10)
!21 = !DILocalVariable(name: "w", scope: !10, file: !1, line: 2, type: !13)
!22 = !DILocation(line: 2, column: 18, scope: !10)
!23 = !DILocalVariable(name: "u", scope: !10, file: !1, line: 2, type: !13)
!24 = !DILocation(line: 2, column: 21, scope: !10)
!25 = !DILocalVariable(name: "v", scope: !10, file: !1, line: 2, type: !13)
!26 = !DILocation(line: 2, column: 24, scope: !10)
!27 = !DILocalVariable(name: "a", scope: !10, file: !1, line: 3, type: !28)
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!29 = !DILocation(line: 3, column: 10, scope: !10)
!30 = !DILocalVariable(name: "b", scope: !10, file: !1, line: 3, type: !28)
!31 = !DILocation(line: 3, column: 14, scope: !10)
!32 = !DILocalVariable(name: "c", scope: !10, file: !1, line: 3, type: !28)
!33 = !DILocation(line: 3, column: 18, scope: !10)
!34 = !DILocalVariable(name: "d", scope: !10, file: !1, line: 3, type: !28)
!35 = !DILocation(line: 3, column: 22, scope: !10)
!36 = !DILocalVariable(name: "e", scope: !10, file: !1, line: 3, type: !28)
!37 = !DILocation(line: 3, column: 26, scope: !10)
!38 = !DILocalVariable(name: "f", scope: !10, file: !1, line: 3, type: !28)
!39 = !DILocation(line: 3, column: 30, scope: !10)
!40 = !DILocalVariable(name: "p", scope: !10, file: !1, line: 4, type: !41)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !28, size: 64)
!42 = !DILocation(line: 4, column: 11, scope: !10)
!43 = !DILocalVariable(name: "q", scope: !10, file: !1, line: 4, type: !41)
!44 = !DILocation(line: 4, column: 16, scope: !10)
!45 = !DILocalVariable(name: "r", scope: !10, file: !1, line: 4, type: !41)
!46 = !DILocation(line: 4, column: 21, scope: !10)
!47 = !DILocalVariable(name: "s", scope: !10, file: !1, line: 4, type: !41)
!48 = !DILocation(line: 4, column: 26, scope: !10)
!49 = !DILocation(line: 6, column: 7, scope: !10)
!50 = !DILocation(line: 7, column: 7, scope: !10)
!51 = !DILocation(line: 8, column: 7, scope: !10)
!52 = !DILocation(line: 9, column: 7, scope: !10)
!53 = !DILocation(line: 10, column: 7, scope: !10)
!54 = !DILocation(line: 11, column: 7, scope: !10)
!55 = !DILocation(line: 13, column: 7, scope: !10)
!56 = !DILocation(line: 14, column: 7, scope: !10)
!57 = !DILocation(line: 15, column: 7, scope: !10)
!58 = !DILocation(line: 16, column: 7, scope: !10)
!59 = !DILocation(line: 18, column: 10, scope: !10)
!60 = !DILocation(line: 18, column: 6, scope: !10)
!61 = !DILocation(line: 18, column: 8, scope: !10)
!62 = !DILocation(line: 19, column: 10, scope: !10)
!63 = !DILocation(line: 19, column: 6, scope: !10)
!64 = !DILocation(line: 19, column: 8, scope: !10)
!65 = !DILocation(line: 20, column: 10, scope: !10)
!66 = !DILocation(line: 20, column: 6, scope: !10)
!67 = !DILocation(line: 20, column: 8, scope: !10)
!68 = !DILocation(line: 21, column: 10, scope: !10)
!69 = !DILocation(line: 21, column: 6, scope: !10)
!70 = !DILocation(line: 21, column: 8, scope: !10)
!71 = !DILocation(line: 23, column: 10, scope: !10)
!72 = !DILocation(line: 23, column: 6, scope: !10)
!73 = !DILocation(line: 23, column: 8, scope: !10)
!74 = !DILocation(line: 24, column: 10, scope: !10)
!75 = !DILocation(line: 24, column: 6, scope: !10)
!76 = !DILocation(line: 24, column: 8, scope: !10)
!77 = !DILocation(line: 25, column: 10, scope: !10)
!78 = !DILocation(line: 25, column: 6, scope: !10)
!79 = !DILocation(line: 25, column: 8, scope: !10)
!80 = !DILocation(line: 26, column: 10, scope: !10)
!81 = !DILocation(line: 26, column: 6, scope: !10)
!82 = !DILocation(line: 26, column: 8, scope: !10)
!83 = !DILocation(line: 28, column: 7, scope: !10)
!84 = !DILocation(line: 29, column: 7, scope: !10)
!85 = !DILocation(line: 30, column: 10, scope: !10)
!86 = !DILocation(line: 30, column: 6, scope: !10)
!87 = !DILocation(line: 30, column: 8, scope: !10)
!88 = !DILocation(line: 31, column: 10, scope: !10)
!89 = !DILocation(line: 31, column: 6, scope: !10)
!90 = !DILocation(line: 31, column: 8, scope: !10)
!91 = !DILocation(line: 33, column: 7, scope: !10)
!92 = !DILocation(line: 35, column: 7, scope: !10)
!93 = !DILocation(line: 37, column: 7, scope: !10)
!94 = !DILocation(line: 38, column: 7, scope: !10)
!95 = !DILocation(line: 39, column: 10, scope: !10)
!96 = !DILocation(line: 39, column: 6, scope: !10)
!97 = !DILocation(line: 39, column: 8, scope: !10)
!98 = !DILocation(line: 40, column: 10, scope: !10)
!99 = !DILocation(line: 40, column: 6, scope: !10)
!100 = !DILocation(line: 40, column: 8, scope: !10)
!101 = !DILocation(line: 42, column: 7, scope: !10)
!102 = !DILocation(line: 43, column: 7, scope: !10)
!103 = !DILocation(line: 45, column: 9, scope: !10)
!104 = !DILocation(line: 45, column: 7, scope: !10)
!105 = !DILocation(line: 46, column: 9, scope: !10)
!106 = !DILocation(line: 46, column: 7, scope: !10)
!107 = !DILocation(line: 47, column: 9, scope: !10)
!108 = !DILocation(line: 47, column: 7, scope: !10)
!109 = !DILocation(line: 48, column: 9, scope: !10)
!110 = !DILocation(line: 48, column: 7, scope: !10)
!111 = !DILocation(line: 50, column: 10, scope: !10)
!112 = !DILocation(line: 50, column: 6, scope: !10)
!113 = !DILocation(line: 50, column: 8, scope: !10)
!114 = !DILocation(line: 51, column: 10, scope: !10)
!115 = !DILocation(line: 51, column: 6, scope: !10)
!116 = !DILocation(line: 51, column: 8, scope: !10)
!117 = !DILocation(line: 53, column: 5, scope: !10)
