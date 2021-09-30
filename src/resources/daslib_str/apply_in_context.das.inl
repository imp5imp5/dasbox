//
// AUTO-GENERATED FILE - DO NOT EDIT!!
//

"options indenting = 2\n"
"options no_unused_block_arguments = false\n"
"options no_unused_function_arguments = false\n"
"options no_aot = true\n"
"\n"
"module apply_in_context shared private\n"
"\n"
"require ast\n"
"require daslib/ast_boost\n"
"require daslib/templates_boost\n"
"require daslib/defer\n"
"\n"
"\n"
"[function_macro(name=\"apply_in_context\")]\n"
"class AppendCondAnnotation : AstFunctionAnnotation\n"
"  def override apply ( var func:FunctionPtr; var group:ModuleGroup; args:Annotat"
"ionArgumentList; var errors : das_string ) : bool\n"
"    if args.length != 1\n"
"      errors := \"expecting one argument\"\n"
"      return false\n"
"\n"
"    var contextName = \"\"\n"
"    for argv in args\n"
"      let val = get_annotation_argument_value(argv)\n"
"      if val is tBool\n"
"        contextName = \"{argv.name}\"\n"
"      else\n"
"        errors := \"invalid argument type {argv.name}\"\n"
"        return false\n"
"\n"
"    for arg in func.arguments\n"
"      if is_temp_type(arg._type,true) && !(arg._type.flags.temporary || arg._typ"
"e.flags._implicit)\n"
"        errors := \"argument {arg.name} needs to be temporary or implicit, i.e. {"
"describe(arg._type)}# or {describe(arg._type)} implicit\"\n"
"        return false\n"
"\n"
"    let resName = \"__res__\"\n"
"    let ctxCloneFnName = \"CONTEXT_CLONE`{func.name}\"\n"
"    let at = func.at\n"
"    let ctxFnName = \"CONTEXT`{func.name}\"\n"
"    var ctxFn <- new [[Function() at = at, atDecl = at, name := ctxFnName]]\n"
"    ctxFn.flags |= FunctionFlags generated\n"
"    ctxFn.flags |= FunctionFlags exports\n"
"    ctxFn.result <- new [[TypeDecl() baseType=Type tVoid, at=at]]\n"
"\n"
"    var ctxQblock <- quote() <|\n"
"      unsafe\n"
"        verify(has_debug_agent_context(CONTEXT_NAME))\n"
"        verify(addr(get_debug_agent_context(CONTEXT_NAME)) == addr(this_context("
")))\n"
"\n"
"    apply_template(at, ctxQblock) <| $ ( rules )\n"
"      rules |> replaceVariable(\"CONTEXT_NAME\") <| new [[ExprConstString() at = a"
"t, value := contextName]]\n"
"    unsafe\n"
"      ctxFn.body <- move_unquote_block(ctxQblock)\n"
"\n"
"    var ctxFnBlock = ctxFn.body as ExprBlock\n"
"    ctxFnBlock.blockFlags ^= ExprBlockFlags isClosure\n"
"\n"
"    var callInCtx <- new [[ExprCall() at=at, name:=ctxCloneFnName]]\n"
"\n"
"    for arg in func.arguments\n"
"      ctxFn.arguments |> emplace_new <| clone_variable(arg)\n"
"      callInCtx.arguments |> emplace_new <| new [[ExprVar() at=at, name:=arg.nam"
"e]]\n"
"\n"
"    var resType <- new [[TypeDecl() baseType=Type tPointer, firstType<-clone_typ"
"e(func.result)]]\n"
"    ctxFn.arguments |> emplace_new <| new [[Variable() at=at, _type<-resType, na"
"me:=resName]]\n"
"\n"
"    var ctxExprDeref <- new [[ExprPtr2Ref() at=at, subexpr <- new [[ExprVar() at"
"=at, name:=resName]] ]]\n"
"    var ctxMovOp <- new [[ExprCopy() at=at, left<-ctxExprDeref, right<-callInCtx"
"]]\n"
"    ctxFnBlock.list |> emplace(ctxMovOp)\n"
"\n"
"    compiling_module() |> add_function(ctxFn)\n"
"\n"
"\n"
"    var fn <- clone_function(func)\n"
"    fn.name := ctxCloneFnName\n"
"    fn.flags |= FunctionFlags generated\n"
"    fn.flags |= FunctionFlags privateFunction\n"
"    fn.flags |= FunctionFlags exports\n"
"    compiling_module() |> add_function(fn)\n"
"\n"
"    func.body := null\n"
"\n"
"    var qblock <- quote() <|\n"
"      unsafe\n"
"        verify(has_debug_agent_context(CONTEXT_NAME))\n"
"        verify(addr(get_debug_agent_context(CONTEXT_NAME)) != addr(this_context("
")))\n"
"\n"
"    apply_template(at, qblock) <| $ ( rules2 )\n"
"      rules2 |> replaceVariable(\"CONTEXT_NAME\") <| new [[ExprConstString() at = "
"at, value := contextName]]\n"
"\n"
"    unsafe\n"
"      func.body <- move_unquote_block(qblock)\n"
"\n"
"    var funcBlock: ExprBlock?\n"
"    unsafe\n"
"      funcBlock = reinterpret<ExprBlock?> func.body\n"
"    funcBlock.blockFlags ^= ExprBlockFlags isClosure\n"
"\n"
"    var vlet <- new [[ExprLet() at=at, atInit=at ]]\n"
"    vlet.variables |> emplace_new() <| new [[Variable() at = at,\n"
"      name := resName,\n"
"      _type <- clone_type(func.result)\n"
"    ]]\n"
"\n"
"    funcBlock.list |> emplace(vlet)\n"
"\n"
"    var pinvoke <- new [[ExprCall() at=at, name:=\"invoke_in_context\"]]\n"
"    pinvoke.genFlags |= ExprGenFlags alwaysSafe\n"
"    var getCtx <- new [[ExprCall() at=at, name:=\"get_debug_agent_context\"]]\n"
"    getCtx.arguments |> emplace_new <| new [[ExprConstString() at=at, value:=con"
"textName]]\n"
"    pinvoke.arguments |> emplace(getCtx)\n"
"    pinvoke.arguments |> emplace_new <| new [[ExprConstString() at=at, value:=ct"
"xFnName]]\n"
"    for arg in func.arguments\n"
"      pinvoke.arguments |> emplace_new <| new [[ExprVar() at=at, name:=arg.name]"
"]\n"
"    var resAddr <- new [[ExprRef2Ptr() at=at, subexpr<-new [[ExprVar() at=at, na"
"me:=resName]] ]]\n"
"    resAddr.genFlags |= ExprGenFlags alwaysSafe\n"
"    pinvoke.arguments |> emplace_new <| resAddr\n"
"\n"
"    funcBlock.list |> emplace(pinvoke)\n"
"\n"
"    funcBlock.list |> emplace_new() <| new [[ExprReturn() at=at, subexpr<-new [["
"ExprVar() at=at, name:=resName]] ]]\n"
"\n"
"    return true\n"
