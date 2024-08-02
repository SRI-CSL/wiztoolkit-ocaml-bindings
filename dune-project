(lang dune 3.14)

(name wiztoolkit)

(generate_opam_files true)

(source
 (github SRI-CSL/wiztoolkit_ocaml_bindings))

(authors "Vitor Pereira and Stéphane Graham-Lengrand")

(maintainers "Vitor Pereira <vitor.pereira@sri.com>")

(license "MIT License")

; (documentation https://url/to/documentation)

(package
 (name wiztoolkit)
 (synopsis "OCaml bindings for the Wiztoolkit toolset")
 (description "Provides OCaml bindings for the SIEVE IR circuit parsing infrastructure provided by the Wiztoolkit toolset (https://github.com/stealthsoftwareinc/wiztoolkit)")
 (depends ocaml dune ctypes containers)
)

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/dune-files.html#dune-project
