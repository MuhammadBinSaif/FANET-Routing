
#ifdef NS3_MODULE_COMPILATION
# error "Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts."
#endif

#ifndef NS3_MODULE_TRAFFIC_CONTROL
    

// Module headers:
#include "cobalt-queue-disc.h"
#include "codel-queue-disc.h"
#include "fifo-queue-disc.h"
#include "fq-codel-queue-disc.h"
#include "mq-queue-disc.h"
#include "packet-filter.h"
#include "pfifo-fast-queue-disc.h"
#include "pie-queue-disc.h"
#include "prio-queue-disc.h"
#include "queue-disc-container.h"
#include "queue-disc.h"
#include "red-queue-disc.h"
#include "tbf-queue-disc.h"
#include "traffic-control-helper.h"
#include "traffic-control-layer.h"
#endif
