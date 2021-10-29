module decs_rpc
require dasweb/jsonrpc_boost
require fio
require math

require daslib/decs

[init,export]
def first_start()
    if !is_in_debug_agent_creation() && !has_web_agent()
        start_agent()



[rpc_call(name="getArchetypes")]
def get_archetypes_rpc(str: string; id: uint64)
    var res: array<JsonValue?>
    for arch in decsState.allArchetypes
        var comps: array<JsonValue?>
        for comp in arch.components
            comps |> emplace <| JV({{ "name" => JV(comp.name); "info" => JV(comp.info) }})
        res |> emplace <| JV({{
            "hash" => JV(arch.hash);
            "size" => JV(arch.size);
            "components" => JV(comps)
        }})
    send_response(JV(res), id)
    delete res


[rpc_call(name="getEntities")]
def get_entities_rpc(str: string; id: uint64)
    var res: array<JsonValue?>
    for arch in decsState.allArchetypes
        for idx in range(arch.size)
            var comps: array<JsonValue?>
            var ent <- {{ "idx" => JV(idx); "archetype" => JV(arch.hash) }}
            for comp in arch.components
                if comp.name == "eid"
                    unsafe
                        let eid = *(reinterpret<EntityId?> addr(comp.data[idx*comp.stride]))
                        ent["eid"] = JV(eid.id)
                        ent["eid_gen"] = JV(eid.generation)

                elif comp.info.basicType == Type tBool
                    unsafe
                        let val = *(reinterpret<bool?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV(val) }})

                elif comp.info.basicType == Type tInt
                    unsafe
                        let val = *(reinterpret<int?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV(val) }})

                elif comp.info.basicType == Type tInt2
                    unsafe
                        let val = *(reinterpret<int2?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tInt3
                    unsafe
                        let val = *(reinterpret<int3?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tInt4
                    unsafe
                        let val = *(reinterpret<int4?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tUInt
                    unsafe
                        let val = *(reinterpret<uint?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV(val) }})

                elif comp.info.basicType == Type tUInt2
                    unsafe
                        let val = *(reinterpret<uint2?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tUInt3
                    unsafe
                        let val = *(reinterpret<uint3?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tUInt4
                    unsafe
                        let val = *(reinterpret<uint4?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})


                elif comp.info.basicType == Type tFloat
                    unsafe
                        let val = *(reinterpret<float?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV(val) }})

                elif comp.info.basicType == Type tFloat2
                    unsafe
                        let val = *(reinterpret<float2?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tFloat3
                    unsafe
                        let val = *(reinterpret<float3?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tFloat4
                    unsafe
                        let val = *(reinterpret<float4?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.basicType == Type tString
                    unsafe
                        let val = *(reinterpret<string?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV(val) }})

                elif comp.info.mangledName == "H<math::float3x3>"
                    unsafe
                        let val = *(reinterpret<float3x3?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.mangledName == "H<math::float3x4>"
                    unsafe
                        let val = *(reinterpret<float3x4?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                elif comp.info.mangledName == "H<math::float4x4>"
                    unsafe
                        let val = *(reinterpret<float4x4?> addr(comp.data[idx*comp.stride]))
                        comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{val}") }})

                else
                    comps |> emplace <| JV({{ "name" => JV(comp.name); "value" => JV("{comp.info.basicType} {comp.data[idx*comp.stride]}") }})

            ent["data"] = JV(comps)
            res |> emplace <| JV(ent)
    send_response(JV(res), id)
    delete res


def tick_rpc()
    tick_agent()