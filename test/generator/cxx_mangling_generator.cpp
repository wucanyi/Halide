#include "Halide.h"

// TODO: Add HalideExtern support for C++ mangling, hopefully using automatic argument type deduction
Halide::Expr extract_value_global(Halide::Expr arg) {
    return Halide::Internal::Call::make(Halide::type_of<int>(),
                                        "extract_value_global", {arg},
                                        Halide::Internal::Call::ExternCPlusPlus);
}

Halide::Expr extract_value_ns(Halide::Expr arg) {
    return Halide::Internal::Call::make(Halide::type_of<int>(),
                                        "HalideTest::extract_value_ns", {arg},
                                        Halide::Internal::Call::ExternCPlusPlus);
}

namespace my_namespace {
class my_class {public: int foo;};
namespace my_subnamespace {
struct my_struct {int foo;};
}
}
union my_union {
    float a;
    int b;
};

HALIDE_DECLARE_EXTERN_CLASS_TYPE(my_namespace::my_class);
HALIDE_DECLARE_EXTERN_STRUCT_TYPE(my_namespace::my_subnamespace::my_struct);
HALIDE_DECLARE_EXTERN_UNION_TYPE(my_union);

class CPlusPlusNameManglingGenerator : public Halide::Generator<CPlusPlusNameManglingGenerator> {
public:
    // Use all the parameter types to make sure mangling works for each of them.
    // TODO: verify this provides full coverage.
    ImageParam input{UInt(8), 1, "input"};
    Param<int8_t> offset_i8{"offset_i8", 0};
    Param<uint8_t> offset_u8{"offset_u8", 0};
    Param<int16_t> offset_i16{"offset_i16", 0};
    Param<uint16_t> offset_u16{"offset_u16", 0};
    Param<int32_t> offset_i32{"offset_i32", 0};
    Param<uint32_t> offset_u32{"offset_u32", 0};
    Param<int64_t> offset_i64{"offset_i64", 0};
    Param<uint64_t> offset_u64{"offset_u64", 0};

    Param<bool> scale_direction{"scale_direction", 1};
    Param<float> scale_f{"scale_f", 0};
    Param<double> scale_d{"scale_d", 0};
    Param<int32_t *> ptr{"ptr", 0};
    Param<int32_t const *> const_ptr{"const_ptr", 0};
    Param<void *> void_ptr{"void_ptr", 0};
    Param<void const *> const_void_ptr{"const_void_ptr", 0};
    // 'string' is just a convenient struct-like thing that isn't special
    // cased by Halide; it will be generated as a void* (but const-ness
    // should be preserved).
    Param<std::string *> string_ptr{"string_ptr", 0};
    Param<std::string const *> const_string_ptr{"const_string_ptr", 0};

    // Test some manually-registered types. These won't be void *.
    Param<const my_namespace::my_class *> const_my_class_ptr{"const_my_class_ptr", 0};
    Param<const my_namespace::my_subnamespace::my_struct *> const_my_struct_ptr{"const_my_struct_ptr", 0};
    Param<const my_union *> const_my_union_ptr{"const_my_union_ptr", 0};

    Func build() {
        assert(get_target().has_feature(Target::CPlusPlusMangling));
        Var x("x");
        Func f("f");

        Expr offset = offset_i8 + offset_u8 + offset_i16 + offset_u16 +
                      offset_i32 + offset_u32 + offset_i64 + offset_u64 +
                      extract_value_global(ptr) + extract_value_ns(const_ptr);

        // No significance to the calculation here.
        f(x) = select(scale_direction, (input(x) * scale_f + offset) / scale_d,
                                       (input(x) * scale_d + offset) / scale_f);

        return f;
    }
};

Halide::RegisterGenerator<CPlusPlusNameManglingGenerator> register_my_gen{"cxx_mangling"};
