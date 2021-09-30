//
// AUTO-GENERATED FILE - DO NOT EDIT!!
//

"options indenting = 4\n"
"options no_unused_block_arguments = false\n"
"options no_unused_function_arguments = false\n"
"module dap shared\n"
"\n"
"require daslib/json\n"
"require daslib/json_boost\n"
"require strings\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Ini"
"tialize\n"
"struct InitializeRequestArguments {}\n"
"\n"
"def InitializeRequestArguments(data: JsonValue?)\n"
"    return [[InitializeRequestArguments]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Dis"
"connect\n"
"struct DisconnectArguments\n"
"    restart: bool\n"
"    terminateDebuggee: bool\n"
"    suspendDebuggee: bool\n"
"\n"
"def DisconnectArguments(data: JsonValue?)\n"
"    return [[DisconnectArguments restart=job(data, \"restart\"),\n"
"                                 terminateDebuggee=job(data, \"terminateDebuggee\""
"),\n"
"                                 suspendDebuggee=job(data, \"suspendDebuggee\")\n"
"            ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Capabi"
"lities\n"
"struct Capabilities\n"
"    supportsConfigurationDoneRequest: bool\n"
"    supportsRestartRequest: bool\n"
"    supportTerminateDebuggee: bool\n"
"    supportsTerminateRequest: bool\n"
"    supportsExceptionOptions: bool\n"
"    supportsExceptionFilterOptions: bool\n"
"    supportsDelayedStackTraceLoading: bool\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Set"
"Breakpoints\n"
"struct SetBreakpointsArguments\n"
"    source: Source\n"
"    breakpoints: array<SourceBreakpoint>\n"
"    sourceModified: bool\n"
"\n"
"def SetBreakpointsArguments(data: JsonValue?)\n"
"    var res <- [[SetBreakpointsArguments source=Source(joj(data, \"source\")),\n"
"                                         sourceModified=job(data, \"sourceModifie"
"d\") ]]\n"
"    var breakpoints = joj(data, \"breakpoints\")\n"
"    if breakpoints != null\n"
"        for it in breakpoints.value as _array\n"
"            res.breakpoints |> emplace(SourceBreakpoint(it))\n"
"    return <- res\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Source"
"\n"
"struct Source\n"
"    name: string\n"
"    path: string\n"
"\n"
"def Source(data: JsonValue?)\n"
"    return [[Source name=jos(data, \"name\"), path=jos(data, \"path\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Source"
"Breakpoint\n"
"struct SourceBreakpoint\n"
"    line: double\n"
"\n"
"def SourceBreakpoint(data: JsonValue?)\n"
"    return [[SourceBreakpoint line=jon(data, \"line\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Set"
"Breakpoints\n"
"struct SetBreakpointsResponse\n"
"    breakpoints: array<Breakpoint>\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Breakp"
"oint\n"
"struct Breakpoint\n"
"    id: double\n"
"    verified: bool\n"
"    source: Source\n"
"    line: double\n"
"    message: string\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Thr"
"eads\n"
"struct ThreadsResponseBody\n"
"    threads: array<Thread>\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Thread"
"\n"
"struct Thread\n"
"    id: double\n"
"    name: string\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Sta"
"ckTrace\n"
"struct StackTraceArguments\n"
"    threadId: double\n"
"    startFrame: double\n"
"    levels: double\n"
"\n"
"def StackTraceArguments(data: JsonValue?)\n"
"    return <- [[StackTraceArguments threadId=jon(data, \"threadId\"), startFrame=j"
"on(data, \"startFrame\"), levels=jon(data, \"levels\")]]\n"
"\n"
"struct StackTraceResponseBody\n"
"    stackFrames: array<StackFrame>\n"
"    totalFrames: double\n"
"\n"
"struct StackFrame\n"
"    id: double\n"
"    name: string\n"
"    source: Source\n"
"    line: double\n"
"    column: double\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Sco"
"pes\n"
"struct ScopesArguments\n"
"    frameId: double\n"
"\n"
"def ScopesArguments(data: JsonValue?)\n"
"    return <- [[ScopesArguments frameId=jon(data, \"frameId\") ]]\n"
"\n"
"struct ScopesResponseBody\n"
"    scopes: array<Scope>\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Scope\n"
"struct Scope\n"
"    name: string\n"
"    variablesReference: double\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Var"
"iables\n"
"struct VariablesArguments\n"
"    variablesReference: double\n"
"    start: double\n"
"    count: double\n"
"\n"
"def VariablesArguments(data: JsonValue?)\n"
"    return <- [[VariablesArguments\n"
"        variablesReference=jon(data, \"variablesReference\"),\n"
"        start=jon(data, \"start\", -1lf),\n"
"        count=jon(data, \"count\", -1lf)\n"
"    ]]\n"
"\n"
"struct VariablesResponseBody\n"
"    variables: array<Variable>\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Types_Variab"
"le\n"
"struct Variable\n"
"    name: string\n"
"    value: string\n"
"    variablesReference: double\n"
"    _type: string\n"
"    indexedVariables: double\n"
"\n"
"def JV(data: Variable)\n"
"    return JV({{\n"
"        \"name\"=>JV(data.name);\n"
"        \"value\"=>JV(data.value);\n"
"        \"variablesReference\"=>JV(data.variablesReference);\n"
"        \"indexedVariables\"=>data.indexedVariables > 0lf ? JV(data.indexedVariabl"
"es) : JV(null);\n"
"        \"type\"=>JV(data._type)\n"
"    }})\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Events_Outpu"
"t\n"
"struct OutputEventBody\n"
"    category: string\n"
"    output: string\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Con"
"tinue\n"
"struct ContinueArguments\n"
"    threadId: double\n"
"\n"
"def ContinueArguments(data: JsonValue?)\n"
"    return <- [[ContinueArguments threadId=jon(data, \"threadId\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pau"
"se\n"
"struct PauseArguments\n"
"    threadId: double\n"
"\n"
"def PauseArguments(data: JsonValue?)\n"
"    return <- [[PauseArguments threadId=jon(data, \"threadId\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pau"
"se\n"
"struct StepInArguments\n"
"    threadId: double\n"
"\n"
"def StepInArguments(data: JsonValue?)\n"
"    return <- [[StepInArguments threadId=jon(data, \"threadId\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Nex"
"t\n"
"struct NextArguments\n"
"    threadId: double\n"
"\n"
"def NextArguments(data: JsonValue?)\n"
"    return <- [[NextArguments threadId=jon(data, \"threadId\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Ste"
"pOut\n"
"struct StepOutArguments\n"
"    threadId: double\n"
"\n"
"def StepOutArguments(data: JsonValue?)\n"
"    return <- [[StepOutArguments threadId=jon(data, \"threadId\") ]]\n"
"\n"
"\n"
"// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Eva"
"luate\n"
"struct EvaluateArguments\n"
"    expression: string\n"
"    frameId: double\n"
"    context: string\n"
"\n"
"def EvaluateArguments(data: JsonValue?)\n"
"    return <- [[EvaluateArguments frameId=jon(data, \"frameId\"), expression=jos(d"
"ata, \"expression\"), context=jos(data, \"context\")]]\n"
"\n"
"\n"
"struct EvaluateResponse\n"
"    result: string\n"
"    _type: string\n"
"    variablesReference: double\n"
"    indexedVariables: double\n"
"\n"
"def JV(data: EvaluateResponse)\n"
"    return JV({{\n"
"        \"result\"=>JV(data.result);\n"
"        \"variablesReference\"=>JV(data.variablesReference);\n"
"        \"indexedVariables\"=>data.indexedVariables > 0lf ? JV(data.indexedVariabl"
"es) : JV(null);\n"
"        \"type\"=>JV(data._type)\n"
"    }})\n"
"\n"
"\n"
"\n"
"def joj(val : JsonValue?; id : string) : JsonValue?\n"
"    var res : JsonValue? = null\n"
"    if val == null || !(val is _object)\n"
"        return res\n"
"    find_if_exists(val as _object, id) <| $(v)\n"
"        res = *v\n"
"    return res\n"
"\n"
"def jon(val : JsonValue?; id : string; defVal = 0lf) : double\n"
"    var res = defVal\n"
"    if val == null || !(val is _object)\n"
"        return res\n"
"    find_if_exists(val.value as _object, id) <| $(v)\n"
"        if v != null && (*v).value is _number\n"
"            res = (*v).value as _number\n"
"    return res\n"
"\n"
"def j_s(val : JsonValue?; defVal = \"\") : string\n"
"    return val?.value ?as _string ?? defVal\n"
"\n"
"def jos(val : JsonValue?; id : string; defVal = \"\") : string\n"
"    var res = defVal\n"
"    if val == null || !(val is _object)\n"
"        return res\n"
"    find_if_exists(val.value as _object, id) <| $(v)\n"
"        if v != null && (*v).value is _string\n"
"            res = (*v).value as _string\n"
"    return res\n"
"\n"
"def job(val : JsonValue?; id : string; defVal = false) : bool\n"
"    var res = defVal\n"
"    if val == null || !(val is _object)\n"
"        return res\n"
"    find_if_exists(val.value as _object, id) <| $(v)\n"
"        if v != null && (*v).value is _bool\n"
"            res = (*v).value as _bool\n"
"    return res\n"
