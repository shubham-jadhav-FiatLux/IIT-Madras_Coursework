; ModuleID = 'testcases/testcase10.c'
source_filename = "testcases/testcase10.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(i32 noundef %0, i32 noundef %1) #0 !dbg !17 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca [20 x i32], align 16
  %6 = alloca [10 x i32], align 16
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
    #dbg_declare(ptr %3, !22, !DIExpression(), !23)
  store i32 %1, ptr %4, align 4
    #dbg_declare(ptr %4, !24, !DIExpression(), !25)
    #dbg_declare(ptr %5, !26, !DIExpression(), !30)
  call void @llvm.memset.p0.i64(ptr align 16 %5, i8 0, i64 80, i1 false), !dbg !30
    #dbg_declare(ptr %6, !31, !DIExpression(), !35)
  call void @llvm.memset.p0.i64(ptr align 16 %6, i8 0, i64 40, i1 false), !dbg !35
    #dbg_declare(ptr %7, !36, !DIExpression(), !37)
  store i32 5, ptr %7, align 4, !dbg !37
    #dbg_declare(ptr %8, !38, !DIExpression(), !39)
  store i32 5, ptr %8, align 4, !dbg !39
  %9 = load i32, ptr %3, align 4, !dbg !40
  %10 = icmp sgt i32 %9, 100, !dbg !42
  br i1 %10, label %11, label %15, !dbg !42

11:                                               ; preds = %2
  %12 = load i32, ptr %7, align 4, !dbg !43
  %13 = sext i32 %12 to i64, !dbg !45
  %14 = getelementptr inbounds [10 x i32], ptr %6, i64 0, i64 %13, !dbg !45
  store i32 100, ptr %14, align 4, !dbg !46
  br label %19, !dbg !47

15:                                               ; preds = %2
  %16 = load i32, ptr %7, align 4, !dbg !48
  %17 = sext i32 %16 to i64, !dbg !50
  %18 = getelementptr inbounds [10 x i32], ptr %6, i64 0, i64 %17, !dbg !50
  store i32 7, ptr %18, align 4, !dbg !51
  br label %19

19:                                               ; preds = %15, %11
  %20 = load i32, ptr %7, align 4, !dbg !52
  %21 = sext i32 %20 to i64, !dbg !53
  %22 = getelementptr inbounds [10 x i32], ptr %6, i64 0, i64 %21, !dbg !53
  %23 = load i32, ptr %22, align 4, !dbg !53
  store i32 %23, ptr %7, align 4, !dbg !54
  %24 = load i32, ptr %7, align 4, !dbg !55
  %25 = sext i32 %24 to i64, !dbg !56
  %26 = getelementptr inbounds [20 x i32], ptr %5, i64 0, i64 %25, !dbg !56
  %27 = load i32, ptr %26, align 4, !dbg !56
  store i32 %27, ptr %7, align 4, !dbg !57
  ret i32 0, !dbg !58
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 !dbg !59 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !64, !DIExpression(), !65)
  store ptr %1, ptr %5, align 8
    #dbg_declare(ptr %5, !66, !DIExpression(), !67)
  %6 = load ptr, ptr %5, align 8, !dbg !68
  %7 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !68
  %8 = load ptr, ptr %7, align 8, !dbg !68
  %9 = call i32 @atoi(ptr noundef %8) #4, !dbg !69
  %10 = load ptr, ptr %5, align 8, !dbg !70
  %11 = getelementptr inbounds ptr, ptr %10, i64 2, !dbg !70
  %12 = load ptr, ptr %11, align 8, !dbg !70
  %13 = call i32 @atoi(ptr noundef %12) #4, !dbg !71
  %14 = call i32 @test(i32 noundef %9, i32 noundef %13), !dbg !72
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %14), !dbg !73
  ret i32 0, !dbg !74
}

declare i32 @printf(ptr noundef, ...) #2

; Function Attrs: nounwind willreturn memory(read)
declare i32 @atoi(ptr noundef) #3

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind willreturn memory(read) }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 19, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase10.c", directory: "/root/llvm/Assignments_3_docker/template/Template", checksumkind: CSK_MD5, checksum: "3f1d4b2fdc7fe11e822978e303f0bf3a")
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
!17 = distinct !DISubprogram(name: "test", scope: !2, file: !2, line: 5, type: !18, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!18 = !DISubroutineType(types: !19)
!19 = !{!20, !20, !20}
!20 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!21 = !{}
!22 = !DILocalVariable(name: "arg1", arg: 1, scope: !17, file: !2, line: 5, type: !20)
!23 = !DILocation(line: 5, column: 14, scope: !17)
!24 = !DILocalVariable(name: "arg2", arg: 2, scope: !17, file: !2, line: 5, type: !20)
!25 = !DILocation(line: 5, column: 24, scope: !17)
!26 = !DILocalVariable(name: "arr", scope: !17, file: !2, line: 6, type: !27)
!27 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 640, elements: !28)
!28 = !{!29}
!29 = !DISubrange(count: 20)
!30 = !DILocation(line: 6, column: 9, scope: !17)
!31 = !DILocalVariable(name: "indexArr", scope: !17, file: !2, line: 6, type: !32)
!32 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 320, elements: !33)
!33 = !{!34}
!34 = !DISubrange(count: 10)
!35 = !DILocation(line: 6, column: 24, scope: !17)
!36 = !DILocalVariable(name: "i", scope: !17, file: !2, line: 6, type: !20)
!37 = !DILocation(line: 6, column: 44, scope: !17)
!38 = !DILocalVariable(name: "j", scope: !17, file: !2, line: 6, type: !20)
!39 = !DILocation(line: 6, column: 51, scope: !17)
!40 = !DILocation(line: 8, column: 9, scope: !41)
!41 = distinct !DILexicalBlock(scope: !17, file: !2, line: 8, column: 9)
!42 = !DILocation(line: 8, column: 14, scope: !41)
!43 = !DILocation(line: 9, column: 18, scope: !44)
!44 = distinct !DILexicalBlock(scope: !41, file: !2, line: 8, column: 21)
!45 = !DILocation(line: 9, column: 9, scope: !44)
!46 = !DILocation(line: 9, column: 21, scope: !44)
!47 = !DILocation(line: 10, column: 5, scope: !44)
!48 = !DILocation(line: 11, column: 18, scope: !49)
!49 = distinct !DILexicalBlock(scope: !41, file: !2, line: 10, column: 12)
!50 = !DILocation(line: 11, column: 9, scope: !49)
!51 = !DILocation(line: 11, column: 21, scope: !49)
!52 = !DILocation(line: 13, column: 18, scope: !17)
!53 = !DILocation(line: 13, column: 9, scope: !17)
!54 = !DILocation(line: 13, column: 7, scope: !17)
!55 = !DILocation(line: 14, column: 13, scope: !17)
!56 = !DILocation(line: 14, column: 9, scope: !17)
!57 = !DILocation(line: 14, column: 7, scope: !17)
!58 = !DILocation(line: 15, column: 5, scope: !17)
!59 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 18, type: !60, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!60 = !DISubroutineType(types: !61)
!61 = !{!20, !20, !62}
!62 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !63, size: 64)
!63 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!64 = !DILocalVariable(name: "argc", arg: 1, scope: !59, file: !2, line: 18, type: !20)
!65 = !DILocation(line: 18, column: 14, scope: !59)
!66 = !DILocalVariable(name: "argv", arg: 2, scope: !59, file: !2, line: 18, type: !62)
!67 = !DILocation(line: 18, column: 27, scope: !59)
!68 = !DILocation(line: 19, column: 30, scope: !59)
!69 = !DILocation(line: 19, column: 25, scope: !59)
!70 = !DILocation(line: 19, column: 45, scope: !59)
!71 = !DILocation(line: 19, column: 40, scope: !59)
!72 = !DILocation(line: 19, column: 20, scope: !59)
!73 = !DILocation(line: 19, column: 5, scope: !59)
!74 = !DILocation(line: 20, column: 5, scope: !59)
