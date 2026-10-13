; ModuleID = 'testcases/testcase3.c'
source_filename = "testcases/testcase3.c"
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
  br i1 %9, label %10, label %19, !dbg !37

10:                                               ; preds = %2
  %11 = load i32, ptr %3, align 4, !dbg !38
  %12 = icmp slt i32 %11, 10, !dbg !41
  br i1 %12, label %13, label %18, !dbg !41

13:                                               ; preds = %10
  %14 = load i32, ptr %3, align 4, !dbg !42
  %15 = sext i32 %14 to i64, !dbg !44
  %16 = getelementptr inbounds [10 x i32], ptr %5, i64 0, i64 %15, !dbg !44
  %17 = load i32, ptr %16, align 4, !dbg !44
  store i32 %17, ptr %7, align 4, !dbg !45
  br label %18, !dbg !46

18:                                               ; preds = %13, %10
  br label %19, !dbg !47

19:                                               ; preds = %18, %2
  ret i32 0, !dbg !48
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 !dbg !49 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
    #dbg_declare(ptr %4, !54, !DIExpression(), !55)
  store ptr %1, ptr %5, align 8
    #dbg_declare(ptr %5, !56, !DIExpression(), !57)
  %6 = load ptr, ptr %5, align 8, !dbg !58
  %7 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !58
  %8 = load ptr, ptr %7, align 8, !dbg !58
  %9 = call i32 @atoi(ptr noundef %8) #4, !dbg !59
  %10 = load ptr, ptr %5, align 8, !dbg !60
  %11 = getelementptr inbounds ptr, ptr %10, i64 2, !dbg !60
  %12 = load ptr, ptr %11, align 8, !dbg !60
  %13 = call i32 @atoi(ptr noundef %12) #4, !dbg !61
  %14 = call i32 @test(i32 noundef %9, i32 noundef %13), !dbg !62
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %14), !dbg !63
  ret i32 0, !dbg !64
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
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 16, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "testcases/testcase3.c", directory: "/root/data/MyAssignments/InstrumentingArrayAccesses/Solution", checksumkind: CSK_MD5, checksum: "3bc46296f51d0d12e4a873377cee13b8")
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
!38 = !DILocation(line: 8, column: 13, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !2, line: 8, column: 13)
!40 = distinct !DILexicalBlock(scope: !36, file: !2, line: 7, column: 20)
!41 = !DILocation(line: 8, column: 18, scope: !39)
!42 = !DILocation(line: 9, column: 21, scope: !43)
!43 = distinct !DILexicalBlock(scope: !39, file: !2, line: 8, column: 24)
!44 = !DILocation(line: 9, column: 17, scope: !43)
!45 = !DILocation(line: 9, column: 15, scope: !43)
!46 = !DILocation(line: 10, column: 9, scope: !43)
!47 = !DILocation(line: 11, column: 5, scope: !40)
!48 = !DILocation(line: 12, column: 5, scope: !17)
!49 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 15, type: !50, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !21)
!50 = !DISubroutineType(types: !51)
!51 = !{!20, !20, !52}
!52 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !53, size: 64)
!53 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!54 = !DILocalVariable(name: "argc", arg: 1, scope: !49, file: !2, line: 15, type: !20)
!55 = !DILocation(line: 15, column: 14, scope: !49)
!56 = !DILocalVariable(name: "argv", arg: 2, scope: !49, file: !2, line: 15, type: !52)
!57 = !DILocation(line: 15, column: 27, scope: !49)
!58 = !DILocation(line: 16, column: 30, scope: !49)
!59 = !DILocation(line: 16, column: 25, scope: !49)
!60 = !DILocation(line: 16, column: 45, scope: !49)
!61 = !DILocation(line: 16, column: 40, scope: !49)
!62 = !DILocation(line: 16, column: 20, scope: !49)
!63 = !DILocation(line: 16, column: 5, scope: !49)
!64 = !DILocation(line: 17, column: 5, scope: !49)
