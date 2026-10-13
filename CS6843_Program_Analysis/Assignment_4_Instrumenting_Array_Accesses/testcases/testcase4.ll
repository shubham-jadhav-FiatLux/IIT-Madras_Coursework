; ModuleID = 'testcases/testcase4.c'
source_filename = "testcases/testcase4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(i32 noundef %0, i32 noundef %1) #0 !dbg !17 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca [10 x i32], align 16
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
    #dbg_declare(ptr %3, !22, !DIExpression(), !23)
  store i32 %1, ptr %4, align 4
    #dbg_declare(ptr %4, !24, !DIExpression(), !25)
    #dbg_declare(ptr %5, !26, !DIExpression(), !30)
  call void @llvm.memset.p0.i64(ptr align 16 %5, i8 0, i64 40, i1 false), !dbg !30
    #dbg_declare(ptr %6, !31, !DIExpression(), !32)
  store i32 0, ptr %6, align 4, !dbg !32
    #dbg_declare(ptr %7, !33, !DIExpression(), !34)
  store i32 0, ptr %7, align 4, !dbg !34
  %8 = load i32, ptr %3, align 4, !dbg !35
  %9 = icmp sge i32 %8, 0, !dbg !37
  br i1 %9, label %10, label %15, !dbg !37

10:                                               ; preds = %2
  %11 = load i32, ptr %3, align 4, !dbg !38
  %12 = sext i32 %11 to i64, !dbg !40
  %13 = getelementptr inbounds [10 x i32], ptr %5, i64 0, i64 %12, !dbg !40
  %14 = load i32, ptr %13, align 4, !dbg !40
  store i32 %14, ptr %7, align 4, !dbg !41
  br label %15, !dbg !42

15:                                               ; preds = %10, %2
  ret i32 0, !dbg !43
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 !dbg !44 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !49, !DIExpression(), !50)
  store ptr %1, ptr %5, align 8
    #dbg_declare(ptr %5, !51, !DIExpression(), !52)
  %6 = load ptr, ptr %5, align 8, !dbg !53
  %7 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !53
  %8 = load ptr, ptr %7, align 8, !dbg !53
  %9 = call i32 @atoi(ptr noundef %8) #4, !dbg !54
  %10 = load ptr, ptr %5, align 8, !dbg !55
  %11 = getelementptr inbounds ptr, ptr %10, i64 2, !dbg !55
  %12 = load ptr, ptr %11, align 8, !dbg !55
  %13 = call i32 @atoi(ptr noundef %12) #4, !dbg !56
  %14 = call i32 @test(i32 noundef %9, i32 noundef %13), !dbg !57
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %14), !dbg !58
  ret i32 0, !dbg !59
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
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 14, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase4.c", directory: "/root/data/MyAssignments/InstrumentingArrayAccesses/Solution", checksumkind: CSK_MD5, checksum: "6c14ab1d0497b7ad259e07c3a005fa64")
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
!27 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 320, elements: !28)
!28 = !{!29}
!29 = !DISubrange(count: 10)
!30 = !DILocation(line: 6, column: 9, scope: !17)
!31 = !DILocalVariable(name: "i", scope: !17, file: !2, line: 6, type: !20)
!32 = !DILocation(line: 6, column: 24, scope: !17)
!33 = !DILocalVariable(name: "j", scope: !17, file: !2, line: 6, type: !20)
!34 = !DILocation(line: 6, column: 31, scope: !17)
!35 = !DILocation(line: 7, column: 9, scope: !36)
!36 = distinct !DILexicalBlock(scope: !17, file: !2, line: 7, column: 9)
!37 = !DILocation(line: 7, column: 14, scope: !36)
!38 = !DILocation(line: 8, column: 17, scope: !39)
!39 = distinct !DILexicalBlock(scope: !36, file: !2, line: 7, column: 20)
!40 = !DILocation(line: 8, column: 13, scope: !39)
!41 = !DILocation(line: 8, column: 11, scope: !39)
!42 = !DILocation(line: 9, column: 5, scope: !39)
!43 = !DILocation(line: 10, column: 5, scope: !17)
!44 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 13, type: !45, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!45 = !DISubroutineType(types: !46)
!46 = !{!20, !20, !47}
!47 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !48, size: 64)
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!49 = !DILocalVariable(name: "argc", arg: 1, scope: !44, file: !2, line: 13, type: !20)
!50 = !DILocation(line: 13, column: 14, scope: !44)
!51 = !DILocalVariable(name: "argv", arg: 2, scope: !44, file: !2, line: 13, type: !47)
!52 = !DILocation(line: 13, column: 27, scope: !44)
!53 = !DILocation(line: 14, column: 30, scope: !44)
!54 = !DILocation(line: 14, column: 25, scope: !44)
!55 = !DILocation(line: 14, column: 45, scope: !44)
!56 = !DILocation(line: 14, column: 40, scope: !44)
!57 = !DILocation(line: 14, column: 20, scope: !44)
!58 = !DILocation(line: 14, column: 5, scope: !44)
!59 = !DILocation(line: 15, column: 5, scope: !44)
