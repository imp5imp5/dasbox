options indenting = 2
options no_unused_function_arguments = false
module server_agent_boost shared

require strings

require daslib/ast_boost
require daslib/json

let
  genFnPrefix = "`gen`"


[export,tag_function(send_request_fn)]
def send_request(method: string; var params: JsonValue?; var fn: function; var ctx: Context): void {}


[tag_function(send_request_fn)]
def send_request(method: string; var params: JsonValue?; var fn: function): void {}


[tag_function_macro(tag="send_request_fn")]
class SendRequestFn : AstFunctionAnnotation
  def override transform(var expr: smart_ptr<ExprCallFunc>; var errors: das_string): ExpressionPtr
    var exprClone <- clone_expression(expr)
    var newCall : smart_ptr<ExprCallFunc>
    unsafe
      newCall <- reinterpret<smart_ptr<ExprCallFunc>> exprClone
    newCall.name := "{newCall.name}_typed"
    var success = false
    for arg in newCall.arguments
      if arg is ExprAddr
        unsafe
          var exprAddr = reinterpret<ExprAddr?> arg
          if !string(exprAddr.target) |> starts_with(genFnPrefix)
            let newName = "{genFnPrefix}{exprAddr.target}"
            var fn <- compiling_module() |> find_unique_function(newName)
            if fn == null
              compiling_program() |> macro_error(newCall.at, "can't find <{newName}> function, annotate function\n[rpc_handler]\ndef {exprAddr.target}...")
              return [[ExpressionPtr]]
            else
              exprAddr.target := newName
              success = true
    if success
      return newCall
    return [[ExpressionPtr]]
