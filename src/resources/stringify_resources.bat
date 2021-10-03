@echo off
set "L=call :add_daslib_file"
set init_file=daslib_str\daslib_init.cpp.inl
del %init_file%


rem call python stringify.py --array JetBrainsMonoNL-Medium.ttf font.JetBrainsMonoNL-Medium.ttf.inl
call python stringify.py --array OpenSans-Regular.ttf font.OpenSans-Regular.ttf.inl
call python stringify.py --array JetBrainsMonoNL-Medium.ttf font.JetBrainsMonoNL-Medium.ttf.inl

%L% algorithm.das
%L% apply.das
%L% apply_in_context.das
%L% assert_once.das
%L% ast_boost.das
%L% ast_used.das
%L% constexpr.das
%L% contracts.das
%L% dap.das
%L% debug.das
%L% defer.das
%L% enum_trait.das
%L% functional.das
%L% if_not_null.das
%L% instance_function.das
%L% is_local.das
%L% jobque_boost.das
%L% json.das
%L% json_boost.das
%L% lpipe.das
%L% random.das
%L% regex.das
%L% regex_boost.das
%L% safe_addr.das
%L% sort_boost.das
%L% static_let.das
%L% strings_boost.das
%L% templates.das
%L% templates_boost.das
%L% unroll.das
%L% decs.das
%L% decs_boost.das

exit/b

:add_daslib_file
call python stringify.py ../../3rdParty/daScript/daslib/%1 daslib_str/%1.inl
set name=%1
set identifier=%name:.=_%
echo static const char %identifier%[] =>> %init_file%
echo #include "resources/daslib_str/%name%.inl">> %init_file%
echo ;>> %init_file%
echo daslib_inc_files[string("%name%")] = das::FileInfo(%identifier%, sizeof(%identifier%));>> %init_file%
echo.>> %init_file%


exit/b
