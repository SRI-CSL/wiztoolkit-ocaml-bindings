#include <cstddef>
#include <cstdio>
#include <vector>

// Parsers
#include <wtk/Parser.h>                // top level API
#include <wtk/circuit/Parser.h>        // circuit IR API
#include <wtk/circuit/Data.h>          // structs used by the circuit parser API
#include <wtk/irregular/Parser.h>      // text parser implementation

#include <wtk/utils/ParserOrganizer.h> // "owner"/"organizer" for parsers

// Backend API
#include <wtk/TypeBackend.h>           // per-field/type backend callback API
#include <wtk/Converter.h>             // field switching API

// NAILS Interpreter
#include <wtk/nails/Interpreter.h>     // main actor of the NAILS API
#include <wtk/nails/Handler.h>         // bridge between NAILS and the Parser
#include <wtk/nails/Functions.h>       // helpers for NAILS functions

// Plugin API
#include <wtk/plugins/Plugin.h>        // The general Plugin API
#include <wtk/plugins/Multiplexer.h>   // Specific to each feature...
#include <wtk/plugins/ArithmeticRAM.h>
#include <wtk/plugins/Vectors.h>       // WizToolKit bonus plugin
#include <wtk/nails/IterPlugin.h>

// An unbounded (or big enough not to overflow) numeric type.
#include <gmpxx.h>
using ValueType = mpz_class;
// using ValueType = uint64_t; 

#include <wtk/utils/NumUtils.gmp.h> // GMP specific hacks
#include <iostream>

#include "bindings.h"

std::vector<WireType> outputs;
std::vector<Gate> graph;

std::vector<ValueType> primes;
std::vector<ValueType> privateStream;
std::vector<ValueType> publicStream;
std::vector<const char*> plugins;

//int nmuls = 0;
//int ncmuls = 0;
//int ngates = 0;

const WireType*
getOutputs(uint64_t* const nrOutputs)
{
  *nrOutputs = outputs.size();
  return &outputs[0];
}

const ValueType*
getPrivateInputs(uint64_t* const nrPrivateInputs)
{
  *nrPrivateInputs = privateStream.size();
  return &privateStream[0];
}

const ValueType*
getPublicInputs(uint64_t* const nrPublicInputs)
{
  *nrPublicInputs = publicStream.size();
  return &publicStream[0];
}

const ValueType*
getPrimes(uint64_t* const nrPrimes)
{
  *nrPrimes = primes.size();
  return &primes[0];
}

Gate const*
getDriver(WireType const output)
{
  return &graph[output];
}

Gate const*
getGates(uint64_t* const nrGates)
{
  *nrGates = graph.size();
  return &graph[0];
}

const char** getPlugins(uint64_t* const nrPlugins) {
  *nrPlugins = plugins.size();
  return &plugins[0];
}

// Struct containing the data carried between gates ("wires")

// Backend implementation of callbacks
struct GraphBuilder : wtk::TypeBackend<ValueType, WireType>
{
  bool assertFailure = false;

  // TypeSpec is a wrapper for the IR's types (field/ring/...)
  GraphBuilder(wtk::circuit::TypeSpec<ValueType> const* t)
    : wtk::TypeBackend<ValueType, WireType>(t) { }

  // $0 <- <0>;
  void assign(WireType* output, ValueType&& input_value) override
  {
    *output = graph.size();
    //printf("C: assign gate(%llu)\n", *element);
    graph.push_back({GateType::assign, *output, 0});
    mpz_set(graph.back().value, input_value.get_mpz_t());
    //ngates = ngates + 1;
  }

  // $1 <- $0;
  void copy(WireType* output, WireType const* input_wire) override
  {
    *output = graph.size();
    //printf("C: copy gate(%llu)\n", *output);
    graph.push_back({GateType::copy, *output, *input_wire, 0});
  }

  // $2 <- @add($0, $1);
  void addGate(WireType* output,
      WireType const* left_input, WireType const* right_input) override
  {
    *output = graph.size();
    //printf("C: add gate(%llu, %llu, %llu)\n", *out, *left, *right);
    graph.push_back({GateType::add, *output, *left_input, *right_input});
    //ngates = ngates + 1;
  }

  // $3 <- @add($1, $2);
  void mulGate(WireType* output,
      WireType const* left_input, WireType const* right_input) override
  {
    *output = graph.size();
    //printf("C: mul gate(%llu, %llu, %llu)\n", *out, *left, *right);
    graph.push_back({GateType::mul, *output, *left_input, *right_input});
    //nmuls = nmuls + 1;
    //ngates = ngates + 1;
  }

  // $4 <- @addc($3, <1>);
  void addcGate(WireType* output,
      WireType const* left_input, ValueType&& right_input) override
  {
    *output = graph.size();
    //printf("C: addc gate(%llu)\n", *out);
    graph.push_back({GateType::addc, *output, *left_input});
    mpz_set(graph.back().value, right_input.get_mpz_t());
    //ncmuls = ncmuls + 1;
    //ngates = ngates + 1;
  }

  // $4 <- @addc($3, <2>);
  void mulcGate(WireType* output,
      WireType const* left_input, ValueType&& right_input) override
  {
    *output = graph.size();
    //printf("C: mulc gate(%llu)\n", *out);
    graph.push_back({GateType::mulc, *output, *left_input});
    mpz_set(graph.back().value, right_input.get_mpz_t());
    //ngates = ngates + 1;
  }

  // @assert_zero($4);
  // Failures may occur, but should get cached until the end when "check()"
  // is called
  void assertZero(WireType const* input_wire) override
  {
    graph.push_back({GateType::assertZero, 0, *input_wire, 0});
    outputs.push_back(*input_wire);
  }

  // $5 <- @public_in();
  void publicIn(WireType* output, ValueType&& input_value) override
  {
    *output = graph.size();
    //printf("C: pub input(%llu)\n", *output);
    graph.push_back({GateType::publicIn, *output, 0});
    mpz_set(graph.back().value, input_value.get_mpz_t());
    publicStream.push_back(input_value);
  }

  // $5 <- @private_in();
  void privateIn(WireType* output, ValueType&& input_value) override
  {
    *output = graph.size();
    //printf("C: sec input(%llu)\n", *output);
    graph.push_back({GateType::privateIn, *output, 0});
    mpz_set(graph.back().value, input_value.get_mpz_t());
    //printf("%llu\n", *output);

    privateStream.push_back(input_value);
  }

  // indicates if any failures occured.
  // Normally true, false on failure.
  bool check() override
  {
    //return !this->assertFailure;
    return true;
  }

  void finish() override
  {
    // optional cleanup tasks.
  }
};

// Subclass the Converter type to implement conversion.
struct MyConverter : public wtk::Converter<WireType, WireType>
{
  // Maybe you need the output and input primes
  ValueType outPrime;
  ValueType inPrime;

  // The parent constructor requires the length of output and input which
  // this Converter will handle.
  MyConverter(size_t out_len, ValueType op, size_t in_len, ValueType ip)
    : wtk::Converter<WireType, WireType>(out_len, in_len),
      outPrime(op), inPrime(ip) { }

  void convert(WireType* const out_wires,
      WireType const* const in_wires, bool modulus) override
  {
    // Your Code Here!

    // out_wires has length this->outLength
    (void) out_wires;
    // in_wires has the length this->inLength
    (void) in_wires;


    if(modulus)
    {
      // An overflowing conversion should behave modularly
    }
    else
    {
      // An overflowing conversion should fail
      if(false/* an overflow occurred */) { this->success = false; }
    }
  }

  // The success or failure is cached until the user calls check at the end.
  bool success = false;
  bool check() override { return success; }
};

bool loadFile(const char* relation_file, const char* instance_file, const char* witness_file)
{
  //printf("1\n");
  wtk::utils::ParserOrganizer<wtk::irregular::Parser<ValueType>, ValueType> organizer;
  //printf("2\n");

  // Open and organize files
  if(!organizer.open(relation_file))
    return false;

  if(!organizer.open(instance_file))
    return false;
  
  if (witness_file[0] != '\0')
    if(!organizer.open(witness_file))
      return false;
  //printf("3\n");

  wtk::utils::Setting setting = organizer.organize();
  if(setting == wtk::utils::Setting::failure)
  {
    printf("failed to organize inputs\n");
    return false;
  }
  //printf("4\n");

  // Create a nails interpreter
  wtk::nails::Interpreter<ValueType> interpreter(organizer.circuitName);
  //printf("5\n");

  // Create a plugins manager, which requires templates for all possible wires
  wtk::plugins::PluginsManager<ValueType,
      WireType,                                 // most plugins
      wtk::plugins::FallbackRAMBuffer<WireType> // RAM has special buffer wires
    > plugins_manager;
  //printf("6\n");

  // Hoist the iteration plugin's allocation, for memory lifetime purposes.
  wtk::nails::MapOperation<ValueType> map_operation(&interpreter);
  //printf("7\n");

  // Scan the plugins required in the circuit's header.
  for(size_t i = 0; i < organizer.circuitBodyParser->plugins.size(); i++)
  {
      //printf("8\n");
    // Recognize each plugin by name
    if("mux_v0" == organizer.circuitBodyParser->plugins[i])
    {
      // Allocate a multiplexer plugin, and hand it off to the plugins_manager.
      std::unique_ptr<wtk::plugins::Plugin<ValueType, WireType>> mux_plugin(
          new wtk::plugins::FallbackMultiplexerPlugin<ValueType, WireType>());
      plugins_manager.addPlugin("mux_v0", std::move(mux_plugin));
    }
    else if("wizkit_vectors" == organizer.circuitBodyParser->plugins[i])
    {
      // Allocate a vectors plugin, and hand it off to the plugins_manager.
      std::unique_ptr<wtk::plugins::Plugin<ValueType, WireType>> vector_plugin(
          new wtk::plugins::FallbackVectorPlugin<ValueType, WireType>());
      plugins_manager.addPlugin("wizkit_vectors", std::move(vector_plugin));
    }
    else if("iter_v0" == organizer.circuitBodyParser->plugins[i])
    {
      // The iter plugin is supplied by the NAILS interpreter as a single
      // instantiable operation, contrary to most other plugins.
      plugins_manager.addPlugin("iter_v0", map_operation.makePlugin<WireType>());
    }
    else if("ram_arith_v0" == organizer.circuitBodyParser->plugins[i]
        || "ram_arith_v1" == organizer.circuitBodyParser->plugins[i])
    {
      char const* const ram_name =
        organizer.circuitBodyParser->plugins[i].c_str();
      // When allocating the RAM plugin, notice how the parent pointer's type
      // changes to have a buffer template, while the allocation still has
      // the ordinary wire type. This is due to how the RAM plugin specializes
      // its parent class.
      //
      // Note, the fallback for RAM is currently a naive linear scan.
      std::unique_ptr<wtk::plugins::Plugin<ValueType,
        wtk::plugins::FallbackRAMBuffer<WireType>>> RAM_plugin(
            new wtk::plugins::FallbackRAMPlugin<ValueType, WireType>());
      plugins_manager.addPlugin(ram_name, std::move(RAM_plugin));
    }
    else
    {
      printf("unrecognized plugin \"%s\"\n",
          organizer.circuitBodyParser->plugins[i].c_str());
      return false;
    }
  }
  //printf("9\n");
  // Create storage for backends, one backend for each type/field
  std::vector<GraphBuilder> backends;
  //printf("10\n");
  // Reserve is necessary otherwise multiple fields (vector growths)
  // invalidate prior pointers.
  backends.reserve(organizer.circuitBodyParser->types.size());
  //printf("11\n");

  // Create storage for RAM backends, Note, its possible to have multiple
  // RAM backends, if multiple types and corresponding RAM types are declared.
  std::vector<wtk::plugins::FallbackRAMBackend<ValueType, WireType>> ram_backends;
  ram_backends.reserve(organizer.circuitBodyParser->types.size() / 2);

  // To instantiate RAM backends, we need to know where the corresponding
  // element's backend is.
  std::vector<bool> is_ram;
  std::vector<size_t> backend_place;
  //printf("12\n");

  // Create each type/field in the same order as required by the relation
  // Notice that the counter, i, may be used as the type index.
  for(size_t i = 0; i < organizer.circuitBodyParser->types.size(); i++)
  {
    //printf("13\n");
    wtk::circuit::TypeSpec<ValueType>* const type =
      &organizer.circuitBodyParser->types[i];

    // Check if it's a RAM or regular field type.
    if(type->variety == wtk::circuit::TypeSpec<ValueType>::plugin
        && (type->binding.name == "ram_arith_v0"
          || type->binding.name == "ram_arith_v1")
        && type->binding.operation == "ram")
    {
      // The plugin must supply the following values, although not all are used
      wtk::type_idx type_index = 0;
      wtk::wire_idx num_allocs = 0;
      wtk::wire_idx total_allocs = 0;
      wtk::wire_idx max_alloc = 0;
      bool has_alloc_hints = type->binding.name == "ram_arith_v0";

      // WizToolKit helper for getting the hintsfrom the type's plugin binding,
      // But only the v0 plugin has the hints.
      if(has_alloc_hints && !wtk::plugins::checkRAMv0Type(
            type, &type_index, &num_allocs, &total_allocs, &max_alloc))
      {
        return false;
      }

      // these ones don't get used by the fallback.
      if(has_alloc_hints)
      {
        (void) num_allocs;
        (void) total_allocs;
        (void) max_alloc;
      }

      // check that the type index refers to a field wire.
      if((size_t) type_index >= is_ram.size()
          || is_ram[(size_t) type_index])
      {
        printf("type %zu is unsuitable as a RAM element", (size_t) type_index);
        return false;
      }

      // finally allocate and supply the backend
      ram_backends.emplace_back(
          type, type_index, &backends[backend_place[(size_t) type_index]]);
      interpreter.addType(&ram_backends.back(), nullptr, nullptr);
      plugins_manager.addBackend((wtk::type_idx) i, &ram_backends.back());

      // record the RAM backend's position
      is_ram.push_back(true);
      backend_place.push_back(ram_backends.size() - 1);
    }
    else if(type->variety != wtk::circuit::TypeSpec<ValueType>::field)
    {
      printf("Type %zu is a plugin (not yet supported)\n", i);
      return false;
    }
    else
    {
      // construct another backend with this prime
      backends.emplace_back(type);
      // add the backend and its streams to the interpreter
      interpreter.addType(&backends.back(),
          organizer.circuitStreams[i].publicStream,
          organizer.circuitStreams[i].privateStream);
      plugins_manager.addBackend((wtk::type_idx) i, &backends.back());
      primes.push_back(type->prime);
      // record the backend's position incase it's used as a RAM element
      is_ram.push_back(false);
      backend_place.push_back(backends.size() - 1);
    }
  }
  //printf("14\n");

  // Create a storage vector for converters, one for each ConversionSpec
  std::vector<MyConverter> converters;
  // Reserve necessary space, otherwise vector growth will invalidate pointers
  converters.reserve(organizer.circuitBodyParser->conversions.size());
  //printf("15\n");

  for(size_t i = 0; i < organizer.circuitBodyParser->conversions.size(); i++)
  {
      //printf("16\n");
    // pointer to the conversion spec (outType, outLength, inType, inLength)
    wtk::circuit::ConversionSpec* const spec =
      &organizer.circuitBodyParser->conversions[i];

    // get the TypeSpecs corresponding to the out and input types.
    wtk::circuit::TypeSpec<ValueType>* const out_type =
      &organizer.circuitBodyParser->types[(size_t) spec->outType];
    wtk::circuit::TypeSpec<ValueType>* const in_type =
      &organizer.circuitBodyParser->types[(size_t) spec->inType];

    // call the constructor for our converter
    converters.emplace_back(
        spec->outLength, out_type->prime, spec->inLength, in_type->prime);
    // add the converter to the interpreter
    interpreter.addConversion(spec, &converters.back());
  }
  //printf("17\n");
  // Create the nails handler (accepts/directs gates from the parser)
  wtk::nails::GatesFunctionFactory<ValueType> func_factory;        // boilerplate
  wtk::nails::Handler<ValueType> handler(
      &interpreter, &func_factory, &plugins_manager);

  // parse the relation and pass each gate off through NAILS and into
  // the backends.
  //printf("18\n");
  if(!organizer.circuitBodyParser->parse(&handler))
  {
    printf("parser failure\n");
    return false;
  }
  //printf("19\n");
  // Check that all the fields succeeded
  bool ret = true;
  for(size_t i = 0; i < backends.size(); i++)
  {
    //printf("20\n");
    if(!backends[i].check())
    {
      printf("failure in field %zu\n", i);
      ret = false;
    }
    //printf("21\n");
    backends[i].finish();
  }
  //printf("22\n");
  // Check that all the conversions succeeded
  for(size_t i = 0; i < converters.size(); i++)
  {
    //printf("23\n");
    if(!converters[i].check())
    {
      printf("failure during conversion\n");
      ret = false;
    }
  }

  return true;
}