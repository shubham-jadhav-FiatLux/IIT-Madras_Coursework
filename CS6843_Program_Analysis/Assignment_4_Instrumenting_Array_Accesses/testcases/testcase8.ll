; ModuleID = 'testcases/testcase8.c'
source_filename = "testcases/testcase8.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(i32 noundef %0, i32 noundef %1) #0 !dbg !17 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca [20 x i32], align 16
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
    #dbg_declare(ptr %3, !22, !DIExpression(), !23)
  store i32 %1, ptr %4, align 4
    #dbg_declare(ptr %4, !24, !DIExpression(), !25)
    #dbg_declare(ptr %5, !26, !DIExpression(), !30)
  call void @llvm.memset.p0.i64(ptr align 16 %5, i8 0, i64 80, i1 false), !dbg !30
    #dbg_declare(ptr %6, !31, !DIExpression(), !32)
  store i32 10, ptr %6, align 4, !dbg !32
    #dbg_declare(ptr %7, !33, !DIExpression(), !34)
  store i32 0, ptr %7, align 4, !dbg !34
  %8 = load i32, ptr %3, align 4, !dbg !35
  switch i32 %8, label %23 [
    i32 10, label %9
    i32 11, label %16
  ], !dbg !36

9:                                                ; preds = %2
  %10 = load i32, ptr %6, align 4, !dbg !37
  %11 = load i32, ptr %3, align 4, !dbg !39
  %12 = add nsw i32 %10, %11, !dbg !40
  %13 = sext i32 %12 to i64, !dbg !41
  %14 = getelementptr inbounds [20 x i32], ptr %5, i64 0, i64 %13, !dbg !41
  %15 = load i32, ptr %14, align 4, !dbg !41
  store i32 %15, ptr %7, align 4, !dbg !42
  br label %28, !dbg !43

16:                                               ; preds = %2
  %17 = load i32, ptr %6, align 4, !dbg !44
  %18 = load i32, ptr %3, align 4, !dbg !45
  %19 = add nsw i32 %17, %18, !dbg !46
  %20 = sext i32 %19 to i64, !dbg !47
  %21 = getelementptr inbounds [20 x i32], ptr %5, i64 0, i64 %20, !dbg !47
  %22 = load i32, ptr %21, align 4, !dbg !47
  store i32 %22, ptr %7, align 4, !dbg !48
  br label %28, !dbg !49

23:                                               ; preds = %2
  %24 = load i32, ptr %3, align 4, !dbg !50
  %25 = sext i32 %24 to i64, !dbg !51
  %26 = getelementptr inbounds [20 x i32], ptr %5, i64 0, i64 %25, !dbg !51
  %27 = load i32, ptr %26, align 4, !dbg !51
  store i32 %27, ptr %7, align 4, !dbg !52
  br label %28, !dbg !53

28:                                               ; preds = %23, %16, %9
  ret i32 0, !dbg !54
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 !dbg !55 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !60, !DIExpression(), !61)
  store ptr %1, ptr %5, align 8
    #dbg_declare(ptr %5, !62, !DIExpression(), !63)
  %6 = load ptr, ptr %5, align 8, !dbg !64
  %7 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !64
  %8 = load ptr, ptr %7, align 8, !dbg !64
  %9 = call i32 @atoi(ptr noundef %8) #4, !dbg !65
  %10 = load ptr, ptr %5, align 8, !dbg !66
  %11 = getelementptr inbounds ptr, ptr %10, i64 2, !dbg !66
  %12 = load ptr, ptr %11, align 8, !dbg !66
  %13 = call i32 @atoi(ptr noundef %12) #4, !dbg !67
  %14 = call i32 @test(i32 noundef %9, i32 noundef %13), !dbg !68
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %14), !dbg !69
  ret i32 0, !dbg !70
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
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 22, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase8.c", directory: "/root/data/MyAssignments/InstrumentingArrayAccesses/Solution", checksumkind: CSK_MD5, checksum: "dfbf837491259a643f8e8301bc136216")
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
!34 = !DILocation(line: 6, column: 32, scope: !17)
!35 = !DILocation(line: 7, column: 13, scope: !17)
!36 = !DILocation(line: 7, column: 5, scope: !17)
!37 = !DILocation(line: 9, column: 21, scope: !38)
!38 = distinct !DILexicalBlock(scope: !17, file: !2, line: 7, column: 19)
!39 = !DILocation(line: 9, column: 23, scope: !38)
!40 = !DILocation(line: 9, column: 22, scope: !38)
!41 = !DILocation(line: 9, column: 17, scope: !38)
!42 = !DILocation(line: 9, column: 15, scope: !38)
!43 = !DILocation(line: 10, column: 13, scope: !38)
!44 = !DILocation(line: 12, column: 21, scope: !38)
!45 = !DILocation(line: 12, column: 23, scope: !38)
!46 = !DILocation(line: 12, column: 22, scope: !38)
!47 = !DILocation(line: 12, column: 17, scope: !38)
!48 = !DILocation(line: 12, column: 15, scope: !38)
!49 = !DILocation(line: 13, column: 13, scope: !38)
!50 = !DILocation(line: 15, column: 21, scope: !38)
!51 = !DILocation(line: 15, column: 17, scope: !38)
!52 = !DILocation(line: 15, column: 15, scope: !38)
!53 = !DILocation(line: 16, column: 13, scope: !38)
!54 = !DILocation(line: 18, column: 5, scope: !17)
!55 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 21, type: !56, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!56 = !DISubroutineType(types: !57)
!57 = !{!20, !20, !58}
!58 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !59, size: 64)
!59 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!60 = !DILocalVariable(name: "argc", arg: 1, scope: !55, file: !2, line: 21, type: !20)
!61 = !DILocation(line: 21, column: 14, scope: !55)
!62 = !DILocalVariable(name: "argv", arg: 2, scope: !55, file: !2, line: 21, type: !58)
!63 = !DILocation(line: 21, column: 27, scope: !55)
!64 = !DILocation(line: 22, column: 30, scope: !55)
!65 = !DILocation(line: 22, column: 25, scope: !55)
!66 = !DILocation(line: 22, column: 45, scope: !55)
!67 = !DILocation(line: 22, column: 40, scope: !55)
!68 = !DILocation(line: 22, column: 20, scope: !55)
!69 = !DILocation(line: 22, column: 5, scope: !55)
!70 = !DILocation(line: 23, column: 5, scope: !55)
