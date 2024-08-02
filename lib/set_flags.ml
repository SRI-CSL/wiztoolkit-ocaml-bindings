open Containers

let args = ref []
let description = "Create flags for linking";;

Arg.parse [] (fun a->args := a::!args) description;;

match !args |> List.rev with
| os::tail ->
   if String.equal os "macosx"
   then
     print_endline
       ("(-cc g++
         -cclib -Wl,-lgmpxx
         -cclib -Wl,-lgmp
         -cclib -Wl,-v
         -cclib -Wl,-force_load
         -cclib %{env:wiztoolkit_ocaml_build_prefix=}/%{lib:wiztoolkit:libbindings.a})")
   else
     let path =
       match tail with
       | path::_ -> path
       | [] -> "/usr/local/lib" (*"/%{ocaml_where}/../wiztoolkit"*)

     in
(* See
   https://github.com/yallop/ocaml-ctypes/issues/541
   https://stackoverflow.com/questions/6578484/telling-gcc-directly-to-link-a-library-statically
   less useful links (approach didn't work)
   https://github.com/ocaml/dune/issues/1904
   https://stackoverflow.com/questions/4156055/static-linking-only-some-libraries
   https://stackoverflow.com/questions/6578484/telling-gcc-directly-to-link-a-library-statically
   https://stackoverflow.com/questions/58564219/how-to-statically-link-all-libraries-except-glibc-using-make-gcc
   https://stackoverflow.com/questions/3698321/g-linker-force-static-linking-if-static-library-exists
   https://stackoverflow.com/questions/6578484/telling-gcc-directly-to-link-a-library-statically *)
     print_endline ("(-cc g++
                      -cclib -Wl,-E -cclib -L"^path^"
                      -cclib -Wl,-lgmpxx
                      -cclib -Wl,-lgmp
                      -cclib -Wl,--verbose
                      -cclib -Wl,-whole-archive
                      -cclib -l:libbindings.a
                      -cclib -Wl,-no-whole-archive)")
| _ -> failwith "Wrong number of arguments in the command.";;
