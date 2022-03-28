options indenting = 2
module server shared

require network
require strings


typedef
  Handler = lambda<(message: string): string>
  MatchResult = variant<success: int; invalid: void?>

class Matcher
  def abstract match(str: string): MatchResult


class StringMatch : Matcher
  matchStr : string
  def StringMatch(str: string)
    matchStr = str
  def override match(str: string): MatchResult
    if str == matchStr
      return <- [[MatchResult success = length(str) ]]
    return <- [[MatchResult invalid = null]]


class WebServer : Server
  port:int = 9000
  restore_session: string = "server-session"
  debugLogs : bool = false

  current_string : array<uint8>
  done : bool = false
  matchers: array<Matcher?>
  handlers: array<Handler>
  unhandledHandler: Handler

  def WebServer
    Server`Server(cast<Server> self)

  def onStr(str: string; var handler: Handler)
    self->on(new StringMatch(str), handler)

  def on(var match: Matcher?; var handler: Handler)
    matchers |> push(match)
    handlers |> emplace(handler)

  def unhandled(var handler: Handler)
    unhandledHandler <- handler

  def override onError(msg:string; code:int)
    print("[E] :{port} error code: {code} msg: {msg}\n")

  def override onConnect()
    if debugLogs
      print(":{port} client connected\n")

  def override onDisconnect()
    if debugLogs
      print(":{port} client disconnected\n")
    if !empty(current_string)
      print("[W] :{port} unhandled req `{string(current_string)}`")
      current_string |> clear()

  def finalize()
    if debugLogs
      print(":{port} destroy server\n")
    delete cast<Server> self

  def restart
    if _server != null
      var session : smart_ptr<NetworkServer>
      self->save(session)
      gc0_save_smart_ptr(restore_session, session)

  def sendStrLf(str: string)
    self->sendStr("{str}\n") // use '\n' to split multiple messages on client

  def sendStr(str: string)
    if debugLogs
      print(":{port} > `{str}`\n")
    unsafe
      self->send(reinterpret<uint8?> str, length(str))

  def override onData(msg:uint8?; size:int)
    for i in range(size)
      var ch: uint8
      unsafe
        ch = msg[i]
      if int(ch) == '\r'
        continue

      if int(ch) == '\n' && length(current_string) > 0
        var res = self->processCurrentString()
        if !res
          if unhandledHandler != null
            let unhandledRes = invoke(unhandledHandler, string(current_string))
            if length(unhandledRes) > 0
              self->sendStr(unhandledRes)
            current_string |> clear()
        continue

      current_string |> push(ch)
    self->processCurrentString()

  def processCurrentString(): bool
    var found = false
    if length(current_string) > 0
      let str = string(current_string)
      if debugLogs
        print(":{port} < `{str}`\n")
      for matcher, idx in matchers, iter_range(matchers)
        let match <- matcher->match(str)
        if match is invalid
          continue
        found = true
        let matchData & = match as success
        let res = invoke(handlers[idx], str)
        if length(res) > 0
          self->sendStr(res)
        let len = matchData
        if len >= length(str)
          current_string |> clear()
        else
          for i in range(length(str) - len)
            current_string[i] = current_string[len + i]
          current_string |> resize(length(str) - len)
        break
    return found


def start_server(var server: WebServer?): bool
  var session : smart_ptr<NetworkServer> <- gc0_restore_smart_ptr(server.restore_session)
  if session != null
    if server.debugLogs
      print("restoring :{server.port} after restart\n")
    server->restore(session)
  elif server->init(server.port)
    if server.debugLogs
      print("start :{server.port}\n")
  else
    if server.debugLogs
      print(":{server.port} failed to initialize\n")
    unsafe
      delete server
    return false
  return true
