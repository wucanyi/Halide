#ifndef HALIDE_INTERNAL_REGION_COSTS_H
#define HALIDE_INTERNAL_REGION_COSTS_H

#include<set>

#include "IR.h"
#include "IRVisitor.h"
#include "Expr.h"
#include "Function.h"
#include "Interval.h"
#include "Bounds.h"
#include "Reduction.h"
#include "Definition.h"
#include "Inline.h"
#include "Simplify.h"
#include "FindCalls.h"
#include "RealizationOrder.h"

namespace Halide {
namespace Internal {

using std::string;
using std::map;
using std::pair;
using std::set;
using std::vector;
using std::make_pair;

typedef map<string, Interval> DimBounds;

struct FindAllCalls : public IRVisitor {
    set<string> funcs_called;
    vector<pair<string, vector<Expr>>> call_args;
    using IRVisitor::visit;

    void visit(const Call *call) {
        // See if images need to be included
        if (call->call_type == Call::Halide || call->call_type == Call::Image) {
            funcs_called.insert(call->name);
            pair<string, vector<Expr>> arg_exprs =
                make_pair(call->name, call->args);
            call_args.push_back(arg_exprs);
        }
        for (size_t i = 0; (i < call->args.size()); i++) {
            call->args[i].accept(this);
        }
    }
};

struct FindImageInputs : public IRVisitor {
    map<string, Type> input_type;
    using IRVisitor::visit;

    void visit(const Call *call) {
        if (call->call_type == Call::Image) {
            input_type[call->name] = call->type;
        }
        for (size_t i = 0; (i < call->args.size()); i++) {
            call->args[i].accept(this);
        }
    }
};

struct RegionCosts {
    const map<string, Function> &env;
    map<string, vector<pair<int64_t, int64_t>>> func_cost;
    map<string, Type> inputs;


    Expr perform_inline(Expr e, const set<string> &inlines = set<string>());

    pair<int64_t, int64_t>
        stage_region_cost(string func, int stage, DimBounds &bounds,
                          const set<string> &inlines = set<string>());

    pair<int64_t, int64_t>
        stage_region_cost(string func, int stage, Box &region,
                          const set<string> &inlines = set<string>());

    pair<int64_t, int64_t>
        region_cost(string func, Box &region,
                    const set<string> &inlines = set<string>());

    pair<int64_t, int64_t>
        region_cost(map<string, Box> &regions,
                    const set<string> &inlines = set<string>());

    vector<pair<int64_t, int64_t>>
        get_func_cost(const Function &f,
                      const set<string> &inlines = set<string>());
    map<string, int64_t>
        stage_detailed_load_costs(string func, int stage,
                                  DimBounds &bounds,
                                  const set<string> &inlines = set<string>());
    map<string, int64_t>
        stage_detailed_load_costs(string func, int stage,
                                  const set<string> &inlines = set<string>());
    map<string, int64_t>
        detailed_load_costs(string func, const Box &region,
                            const set<string> &inlines = set<string>());
    map<string, int64_t>
        detailed_load_costs(const map<string, Box> &regions,
                            const set<string> &inlines = set<string>());

    int64_t region_size(string func, const Box &region);

    int64_t region_footprint(const map<string, Box> &regions,
                             const set<string> &inlined = set<string>());

    int64_t input_region_size(string input, const Box &region);

    int64_t input_region_size(const map<string, Box> &input_regions);

    void disp_func_costs();

    RegionCosts(const map<string, Function> &_env);
};

// Utility functions
int get_extent(const Interval &i);

int64_t box_area(const Box &b);

void disp_regions(const map<string, Box> &regions);

Definition get_stage_definition(const Function &f, int stage_num);

void combine_load_costs(map<string, int64_t> &result,
                        const map<string, int64_t> &partial);

DimBounds get_stage_bounds(Function f, int stage_num,
                           const DimBounds &pure_bounds);

vector<DimBounds> get_stage_bounds(Function f, const DimBounds &pure_bounds);

}
}

#endif