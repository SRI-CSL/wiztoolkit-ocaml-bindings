#include "gmp.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include "stdint.h"
#include "stdbool.h"
#endif

  enum GateType
    {
     assign,
     copy,
     add,
     mul,
     addc,
     mulc,
     publicIn,
     privateIn,
     assertZero
    };

  typedef uint64_t WireType;
  typedef mpz_t CValueType;

  struct Gate
  {
    enum GateType gateType;
    //
    // Gate ID
    //
    WireType out;
    //
    //	Defined for gateType: copy, add, mul, addc, mulc
    //
    WireType left;
    union
    {
      //
      //	Defined for gateType: add, mul
      //
      WireType right;
      //
      //	Defined for gateType: assign, addc, mulc, publicIn, privateIn
      //
      CValueType value;
    };
  };

  //
  //	Loads the file containing a graph; returns true on success and false on failure
  //
  bool
  loadFile(const char* relation_file, const char* instance_file, const char* witness_file);
  //
  //	Returns pointer to array of output wires and sets nrOutputs to array size
  //
  const WireType*
  getOutputs(uint64_t* const nrOutputs);
  //
  //	Returns a pointer to a Gate struct whose output is wire
  //
  struct Gate const*
  getDriver(WireType const wire);

struct Gate const*
getGates(uint64_t* const nrGates);

const ValueType*
getPrivateInputs(uint64_t* const nrPrivateInputs);

const ValueType*
getPublicInputs(uint64_t* const nrPublicInputs);

const ValueType*
getPrimes(uint64_t* const nrPrimes);

const char **
getPlugins(uint64_t* const nrPlugins);

#ifdef __cplusplus
}
#endif
