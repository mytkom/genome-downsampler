#ifndef MIN_COST_FLOW_SOLVER_COMMON_H
#define MIN_COST_FLOW_SOLVER_COMMON_H

#include <cstddef>
#include <cstdint>

namespace min_cost_flow_solver {

// vertex index data type
typedef std::size_t EdgeIndex;
typedef std::size_t Index;
typedef std::size_t Height;
typedef int32_t Flow;

struct EdgeNode {
    Index to;
    EdgeIndex next;
    Flow res_cap;
};

struct VertexProperties {
    EdgeIndex head;
    Height height;
    Flow excess;
};

}  // namespace min_cost_flow_solver

#endif
