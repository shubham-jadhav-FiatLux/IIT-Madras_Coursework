; ModuleID = 'testcases/testcase6.c'
source_filename = "testcases/testcase6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(i32 noundef %0, i32 noundef %1) #0 !dbg !17 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca [20 x i32], align 16
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !22, !DIExpression(), !23)
  store i32 %1, ptr %5, align 4
    #dbg_declare(ptr %5, !24, !DIExpression(), !25)
    #dbg_declare(ptr %6, !26, !DIExpression(), !30)
  call void @llvm.memset.p0.i64(ptr align 16 %6, i8 0, i64 80, i1 false), !dbg !30
    #dbg_declare(ptr %7, !31, !DIExpression(), !32)
  store i32 0, ptr %7, align 4, !dbg !32
    #dbg_declare(ptr %8, !33, !DIExpression(), !34)
  store i32 100, ptr %8, align 4, !dbg !34
  br label %9, !dbg !35

9:                                                ; preds = %23, %2
  %10 = load i32, ptr %7, align 4, !dbg !36
  %11 = icmp slt i32 %10, 10, !dbg !37
  br i1 %11, label %12, label %24, !dbg !35

12:                                               ; preds = %9
  %13 = load i32, ptr %8, align 4, !dbg !38
  %14 = icmp sgt i32 %13, 20, !dbg !41
  br i1 %14, label %15, label %23, !dbg !41

15:                                               ; preds = %12
  %16 = load i32, ptr %8, align 4, !dbg !42
  %17 = load i32, ptr %4, align 4, !dbg !44
  %18 = add nsw i32 %16, %17, !dbg !45
  store i32 %18, ptr %8, align 4, !dbg !46
  %19 = load i32, ptr %8, align 4, !dbg !47
  %20 = sext i32 %19 to i64, !dbg !48
  %21 = getelementptr inbounds [20 x i32], ptr %6, i64 0, i64 %20, !dbg !48
  %22 = load i32, ptr %21, align 4, !dbg !48
  store i32 %22, ptr %8, align 4, !dbg !49
  store i32 0, ptr %3, align 4, !dbg !50
  br label %25, !dbg !50

23:                                               ; preds = %12
  br label %9, !dbg !35, !llvm.loop !51

24:                                               ; preds = %9
  store i32 0, ptr %3, align 4, !dbg !54
  br label %25, !dbg !54

25:                                               ; preds = %24, %15
  %26 = load i32, ptr %3, align 4, !dbg !55
  ret i32 %26, !dbg !55
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 !dbg !56 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !61, !DIExpression(), !62)
  store ptr %1, ptr %5, align 8
    #dbg_declare(ptr %5, !63, !DIExpression(), !64)
  %6 = load ptr, ptr %5, align 8, !dbg !65
  %7 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !65
  %8 = load ptr, ptr %7, align 8, !dbg !65
  %9 = call i32 @atoi(ptr noundef %8) #4, !dbg !66
  %10 = load ptr, ptr %5, align 8, !dbg !67
  %11 = getelementptr inbounds ptr, ptr %10, i64 2, !dbg !67
  %12 = load ptr, ptr %11, align 8, !dbg !67
  %13 = call i32 @atoi(ptr noundef %12) #4, !dbg !68
  %14 = call i32 @test(i32 noundef %9, i32 noundef %13), !dbg !69
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %14), !dbg !70
  ret i32 0, !dbg !71
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
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 18, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase6.c", directory: "/root/data/MyAssignments/InstrumentingArrayAccesses/Solution", checksumkind: CSK_MD5, checksum: "f232c94158beb69f92c9648be0c5091f")
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
!31 = !DILocalVariable(name: "i", scope: !17, file: !2, line: 6, type: !20)
!32 = !DILocation(line: 6, column: 24, scope: !17)
!33 = !DILocalVariable(name: "j", scope: !17, file: !2, line: 6, type: !20)
!34 = !DILocation(line: 6, column: 31, scope: !17)
!35 = !DILocation(line: 7, column: 5, scope: !17)
!36 = !DILocation(line: 7, column: 12, scope: !17)
!37 = !DILocation(line: 7, column: 14, scope: !17)
!38 = !DILocation(line: 8, column: 13, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !2, line: 8, column: 13)
!40 = distinct !DILexicalBlock(scope: !17, file: !2, line: 7, column: 20)
!41 = !DILocation(line: 8, column: 15, scope: !39)
!42 = !DILocation(line: 9, column: 17, scope: !43)
!43 = distinct !DILexicalBlock(scope: !39, file: !2, line: 8, column: 21)
!44 = !DILocation(line: 9, column: 21, scope: !43)
!45 = !DILocation(line: 9, column: 19, scope: !43)
!46 = !DILocation(line: 9, column: 15, scope: !43)
!47 = !DILocation(line: 10, column: 21, scope: !43)
!48 = !DILocation(line: 10, column: 17, scope: !43)
!49 = !DILocation(line: 10, column: 15, scope: !43)
!50 = !DILocation(line: 11, column: 13, scope: !43)
!51 = distinct !{!51, !35, !52, !53}
!52 = !DILocation(line: 13, column: 5, scope: !17)
!53 = !{!"llvm.loop.mustprogress"}
!54 = !DILocation(line: 14, column: 5, scope: !17)
!55 = !DILocation(line: 15, column: 1, scope: !17)
!56 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 17, type: !57, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!57 = !DISubroutineType(types: !58)
!58 = !{!20, !20, !59}
!59 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !60, size: 64)
!60 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!61 = !DILocalVariable(name: "argc", arg: 1, scope: !56, file: !2, line: 17, type: !20)
!62 = !DILocation(line: 17, column: 14, scope: !56)
!63 = !DILocalVariable(name: "argv", arg: 2, scope: !56, file: !2, line: 17, type: !59)
!64 = !DILocation(line: 17, column: 27, scope: !56)
!65 = !DILocation(line: 18, column: 30, scope: !56)
!66 = !DILocation(line: 18, column: 25, scope: !56)
!67 = !DILocation(line: 18, column: 45, scope: !56)
!68 = !DILocation(line: 18, column: 40, scope: !56)
!69 = !DILocation(line: 18, column: 20, scope: !56)
!70 = !DILocation(line: 18, column: 5, scope: !56)
!71 = !DILocation(line: 19, column: 5, scope: !56)
