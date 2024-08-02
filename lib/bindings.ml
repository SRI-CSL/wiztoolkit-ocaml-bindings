open Ctypes_zarith
open Unsigned

(* let __int8_t = Ctypes.typedef Ctypes.schar "__int8_t" *)
(* let __uint8_t = Ctypes.typedef Ctypes.uchar "__uint8_t" *)
(* let __int16_t = Ctypes.typedef Ctypes.short "__int16_t" *)
(* let __uint16_t = Ctypes.typedef Ctypes.ushort "__uint16_t" *)
(* let __int32_t = Ctypes.typedef Ctypes.sint "__int32_t" *)
(* let __uint32_t = Ctypes.typedef Ctypes.uint "__uint32_t" *)
(* let __int64_t = Ctypes.typedef Ctypes.long "__int64_t" *)
let __uint64_t = Ctypes.typedef Ctypes.ulong "__uint64_t"

(* let int8_t = Ctypes.typedef __int8_t "int8_t" *)
(* let int16_t = Ctypes.typedef __int16_t "int16_t" *)
(* let int32_t = Ctypes.typedef __int32_t "int32_t" *)
(* let int64_t = Ctypes.typedef __int64_t "int64_t" *)
(* let uint8_t = Ctypes.typedef __uint8_t "uint8_t" *)
(* let uint16_t = Ctypes.typedef __uint16_t "uint16_t" *)
(* let uint32_t = Ctypes.typedef __uint32_t "uint32_t" *)
let uint64_t = Ctypes.typedef __uint64_t "uint64_t"

let wire_t  = Ctypes.typedef uint64_t "WireType"
let value_t = Ctypes.typedef MPZ.t "CValueType"

let gate_type =
  let (to_int, of_int) =
    ((function
      | `assign -> 0L
      | `copy   -> 1L
      | `add    -> 2L
      | `mul    -> 3L
      | `addc   -> 4L
      | `mulc   -> 5L
      | `publicIn -> 6L
      | `privateIn -> 7L
      | `assertZero -> 8L),
     (function
      | 0L -> `assign
      | 1L -> `copy
      | 2L -> `add
      | 3L -> `mul
      | 4L -> `addc
      | 5L -> `mulc
      | 6L -> `publicIn
      | 7L -> `privateIn
      | 8L -> `assertZero
      | _ -> failwith "enum to_int")) in
  object
    method ctype = Ctypes.uint
    method to_int = to_int
    method of_int = of_int
  end

let gate_type_t = Ctypes.typedef gate_type#ctype "GateType"

let union_s_0  : [ `wire_or_value ] Ctypes.union Ctypes.typ = Ctypes.union "other_arg"

let union_s =
  let case_0 = Ctypes.field union_s_0 "right" wire_t in
  let case_1 = Ctypes.field union_s_0 "value" value_t in
  let () = Ctypes.seal union_s_0 in
  object
    method ctype = union_s_0
    method members =
      object
        method right = case_0
        method value = case_1
      end
  end

let union_t = Ctypes.typedef union_s_0 "Other"

let gate_s_0 =
  let (_ctype : [ `gate_s ] Ctypes.structure Ctypes.typ) =
    Ctypes.structure "gate_s" in
  _ctype

let gate_s =
  let field_0 = Ctypes.field gate_s_0 "gateType" gate_type_t in
  let field_1 = Ctypes.field gate_s_0 "out" wire_t in
  let field_2 = Ctypes.field gate_s_0 "left" wire_t in
  let field_3 = Ctypes.field gate_s_0 "other" union_t in
  let () = Ctypes.seal gate_s_0 in
  object
    method ctype = gate_s_0
    method members =
      object
        method gate_type = field_0
        method out = field_1
        method left  = field_2
        method other = field_3
      end
  end

let gate_t = Ctypes.typedef gate_s_0 "Gate"

open Types.BaseTypes

let gate_type (x : gate_t) : gate_type =
  Ctypes.getf x (gate_s#members#gate_type) |> Unsigned.UInt.to_int64 |> gate_type#of_int

let gate_out (x : gate_t) : ulong = Ctypes.getf x (gate_s#members#out)

let gate_left (x : gate_t) : ulong = Ctypes.getf x (gate_s#members#left)

let gate_right (x : gate_t) : ulong =
  let h = Ctypes.getf x (gate_s#members#other) in
  Ctypes.getf h (union_s#members#right)

let gate_value (x : gate_t) : Z.t =
  let h = Ctypes.getf x (gate_s#members#other) in
  Ctypes.getf h (union_s#members#value)
  |> Ctypes.addr
  |> MPZ.to_z

let load_file relation instance witness =
  let aux =
    Foreign.foreign "loadFile"
      Ctypes.(ptr char @-> ptr char @-> ptr char @-> returning bool)
  in
  aux
    Ctypes.(coerce string (ptr char) relation)
    Ctypes.(coerce string (ptr char) instance)
    Ctypes.(coerce string (ptr char) witness)

let get_outputs =
  Foreign.foreign "getOutputs"
    Ctypes.(ptr uint64_t @-> returning (ptr wire_t))

let get_private_inputs =
  Foreign.foreign "getPrivateInputs"
    Ctypes.(ptr uint64_t @-> returning (ptr value_t))

let get_public_inputs =
  Foreign.foreign "getPublicInputs"
    Ctypes.(ptr uint64_t @-> returning (ptr value_t))

let get_primes =
  Foreign.foreign "getPrimes"
    Ctypes.(ptr uint64_t @-> returning (ptr value_t))

let get_driver =
  Foreign.foreign "getDriver"
    Ctypes.(wire_t @-> returning (ptr gate_t))

let get_gates =
  Foreign.foreign "getGates"
    Ctypes.(ptr uint64_t @-> returning (ptr gate_t))

let get_plugins =
  Foreign.foreign "getPlugins"
    Ctypes.(ptr uint64_t @-> returning (ptr (string)))
