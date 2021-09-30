//
// AUTO-GENERATED FILE - DO NOT EDIT!!
//

"options indenting = 4\n"
"options no_unused_block_arguments = false\n"
"options no_unused_function_arguments = false\n"
"options no_aot = true\n"
"\n"
"module defer shared private\n"
"\n"
"require ast\n"
"require rtti\n"
"require daslib/ast_boost\n"
"require daslib/templates_boost\n"
"\n"
"[tag_function(defer_tag)]\n"
"def public defer ( blk : block<():void> ) {}\n"
"\n"
"def public nada() {}\n"
"\n"
"// covnert defer() <| block expression\n"
"// into {}, and move block to the finally section of the current block\n"
"[tag_function_macro(tag=\"defer_tag\")]\n"
"class DeferMacro : AstFunctionAnnotation\n"
"    def override transform ( var call : smart_ptr<ExprCallFunc>; var errors : da"
"s_string ) : ExpressionPtr\n"
"        var success = true\n"
"        compiling_program() |> get_ast_context(call) <| $(valid, astc)\n"
"            if !valid\n"
"                compiling_program() |>macro_error(call.at,\"can't get valid progr"
"am context\")\n"
"                success = false\n"
"            elif astc.scopes.length == 0\n"
"                compiling_program() |>macro_error(call.at,\"defer needs to be in "
"the scope\")\n"
"                success = false\n"
"            else\n"
"                var scope = astc.scopes[astc.scopes.length-1] as ExprBlock\n"
"                var c_arg <- clone_expression(call.arguments[0])\n"
"                var from_block <- move_unquote_block(c_arg)\n"
"                from_block.blockFlags ^= ExprBlockFlags isClosure\n"
"                scope.finalList |> emplace(from_block,0)\n"
"        return !success ? [[ExpressionPtr]] : quote() <| nada()\n"
"\n"
"// convert defer_delete() expression\n"
"// into {}, and add delete expression to the finally section of the current bloc"
"k\n"
"[call_macro(name=\"defer_delete\")]\n"
"class DeferDeleteMacro : AstCallMacro\n"
"    def override visit ( prog:ProgramPtr; mod:Module?; var call:smart_ptr<ExprCa"
"llMacro> ) : ExpressionPtr\n"
"        if call.arguments.length!=1\n"
"            prog |> macro_error(call.at,\"expecting defer_delete(expr)\")\n"
"            return [[ExpressionPtr]]\n"
"        var success = true\n"
"        prog |> get_ast_context(call) <| $(valid, astc)\n"
"            if !valid\n"
"                compiling_program() |>macro_error(call.at,\"can't get valid progr"
"am context\")\n"
"                success = false\n"
"            elif astc.scopes.length == 0\n"
"                compiling_program() |>macro_error(call.at,\"defer_delete needs to"
" be in the scope\")\n"
"                success = false\n"
"            else\n"
"                var scope = astc.scopes[astc.scopes.length-1] as ExprBlock\n"
"                var e_del <- new [[ExprDelete() at=call.at,\n"
"                    subexpr <- clone_expression(call.arguments[0])\n"
"                ]]\n"
"                scope.finalList |> emplace(e_del,0)\n"
"        return !success ? [[ExpressionPtr]] : quote() <| nada()\n"
