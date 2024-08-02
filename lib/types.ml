open Ctypes
open Unsigned
open Ctypes_zarith

module BaseTypes = struct

  type gate_t = [ `gate_s ] structure
                               
  type gate_type =
    [ `assign
    | `copy
    | `add
    | `mul
    | `addc
    | `mulc
    | `publicIn
    | `privateIn
    | `assertZero ]

end


module type API = sig

  open BaseTypes
  
  val gate_type   : gate_t -> gate_type
  val gate_out    : gate_t -> ulong
  val gate_left   : gate_t -> ulong
  val gate_right  : gate_t -> ulong
  val gate_value  : gate_t -> Z.t
  val load_file   : string -> string -> string -> bool
  val get_outputs : uint64 ptr -> ulong ptr
  val get_private_inputs : uint64 ptr -> MPZ.t Ctypes_static.abstract ptr
  val get_public_inputs : uint64 ptr -> MPZ.t Ctypes_static.abstract ptr
  val get_primes : uint64 ptr -> MPZ.t Ctypes_static.abstract ptr
  val get_driver  : ulong -> gate_t ptr
  val get_gates   : uint64 ptr -> gate_t ptr
  val get_plugins  : uint64 ptr -> string ptr

end
