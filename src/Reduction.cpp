#include "IR.h"
#include "Reduction.h"

namespace Halide {
namespace Internal {

struct ReductionDomainContents : public RefCounted {
    std::vector<ReductionVariable> domain;
};

ReductionDomain::ReductionDomain(const std::vector<ReductionVariable> &domain) :
    contents(new ReductionDomainContents) {
    contents->domain = domain;
}

const std::vector<ReductionVariable> &ReductionDomain::domain() const {
    return contents->domain;
}

}
}
