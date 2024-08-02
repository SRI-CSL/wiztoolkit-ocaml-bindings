# Wiztoolkit OCaml Bindings: OCaml bindings to the `wiztoolkit` toolset to read SIEVE IR circuits

This repository provides an OCaml library containing bindings for the [wiztoolkit](https://github.com/stealthsoftwareinc/wiztoolkit) toolset. Wiztoolkit is a collection of tools for working with the SIEVE IR circuit format with an extensive C++ API for manipulating the SIEVE IR.

## Contents

The main module of the OCaml library is called `Wiztoolkit.Bindings`, and provides the following functions
- `load_file` - uses `wiztoolkit` to parse the relation, instance and witness files, creating the corresponding OCaml datatypes
- `get_gates` - returns an array with the circuit gates
- `get_private_inputs` - returns an array with the private inputs values
- `get_public_inputs` - returns an array with the public input values
- `get_primes` - returns an array with the primes used to define the circuit 
- `get_plugins` - returns the plugins used in the circuit description

## Instalation requirements

Our `wiztoolkit` OCaml library uses the following third-party tools/libraries:
- OCaml (>= 4.14.0) - available at [https://ocaml.org/](https://ocaml.org/)
- Dune (>= 3.14) - available at [https://github.com/ocaml/dune](https://github.com/ocaml/dune)
- `wiztoolkit` - available at [https://github.com/stealthsoftwareinc/wiztoolkit](https://github.com/stealthsoftwareinc/wiztoolkit)

To install `wiztoolkit`, please follow the instructions provided at [https://github.com/stealthsoftwareinc/wiztoolkit/blob/main/docs/7_install.adoc](https://github.com/stealthsoftwareinc/wiztoolkit/blob/main/docs/7_install.adoc). The other dependencies can be installed using `opam` by typing

```
$> opam install dune
```

## Installing/Compiling EVOCrypt

If installing from source, running

```
$> make
$> make install
```

builds and install the `wiztoolkit` OCaml library (under root module named `Wiztoolkit.Bindings`) assuming that all dependencies have been successfully installed. 

The bindings can also be installed via `opam`, by running

```
$> opam install wiztoolkit_ocaml
```

## Examples

Examples of how to use EVOCrypt can be found in the `test` directory.

## Acknowledgments

This material is based upon work supported by DARPA under Contract No. HR001120C0086. Any opinions, findings and conclusions or recommendations expressed in this material are those the author(s) and do not necessarily reflect the views of the United States Government or DARPA.

Distribution Statement ‘A’ (Approved for Public Release, Distribution Unlimited)