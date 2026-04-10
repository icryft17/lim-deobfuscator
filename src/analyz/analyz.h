#ifndef ANALYZ_H
#define ANALYZ_H

#include "pattern/switch.h"
#include "type/control_flow_graph.h"

#include "context.h"

namespace analyz
{
	type::ControlFlowGraph *AnalyzBinary(type::Context *ctx);

}
#endif
