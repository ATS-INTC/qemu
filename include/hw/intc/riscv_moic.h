/*
 * RISC-V MOIC (Multiple-Object Interaction Interrupt Controller) interface
 */

#ifndef HW_RISCV_MOIC_H
#define HW_RISCV_MOIC_H

#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/queue.h"

#define MAX_IRQ                 0x100
#define RISCV_MOIC_MMIO_SIZE    0x1000000
#define MAX_PRIORITY            32
#define SIZEOF_PERHART          0x1000

#define ADD_OP                          0x00
#define FETCH_OP                        0x08
#define SWITCH_PROC_OP                  0x10
#define SWITCH_OS_OP                    0x18
#define REGISTER_RECV_TASK_OP           0x20
#define REGISTER_RECV_TARGET_OS_OP      0x28
#define REGISTER_RECV_TARGET_PROC_OP    0x30
#define REGISTER_RECV_TARGET_TASK_OP    0x38
#define REGISTER_SEND_TASK_OP           0x40
#define REGISTER_SEND_TARGET_OS_OP      0x48
#define REGISTER_SEND_TARGET_PROC_OP    0x50
#define REGISTER_SEND_TARGET_TASK_OP    0x58
#define SEND_INTR_OS_OP                 0x60                
#define SEND_INTR_PROC_OP               0x68                
#define SEND_INTR_TASK_OP               0x70                

#define TYPE_RISCV_MOIC "riscv.moic"

typedef struct RISCVMOICState RISCVMOICState;
DECLARE_INSTANCE_CHECKER(RISCVMOICState, RISCV_MOIC, TYPE_RISCV_MOIC)

typedef QSIMPLEQ_HEAD(, QueueEntry) QueueHead;

struct QueueEntry {
    uint64_t data;
    QSIMPLEQ_ENTRY(QueueEntry) next;
};

typedef struct {
    QueueHead head;
} Queue;

void queue_init(Queue* queue);
void queue_push(Queue* queue, uint64_t data);
uint64_t queue_pop(Queue* queue);

typedef struct {
    Queue *task_queues;
} PriorityQueue;

void pq_init(PriorityQueue* pq);
void pq_push(PriorityQueue* pq, uint64_t priority, uint64_t data);
uint64_t pq_pop(PriorityQueue* pq);

typedef struct {
    uint64_t os_id;
    uint64_t proc_id;
    uint64_t task_id;
} TotalIdentity;

typedef struct {
    uint64_t task_id;
    TotalIdentity target;
} Capability;

typedef QSIMPLEQ_HEAD(, CapQueueEntry) CapQueueHead;

struct CapQueueEntry {
    Capability cap;
    QSIMPLEQ_ENTRY(CapQueueEntry) next;
};

typedef struct {
    CapQueueHead head;
} CapQueue;

void cap_queue_init(CapQueue* cap_queue);
void cap_queue_insert(CapQueue* cap_queue, uint64_t task_id, 
    uint64_t target_os_id, 
    uint64_t target_proc_id, 
    uint64_t target_task_id);

struct CapQueueEntry* cap_queue_remove(CapQueue* cap_queue, 
    uint64_t task_id, 
    uint64_t target_os_id, 
    uint64_t target_proc_id, 
    uint64_t target_task_id);

bool is_device_cap(Capability* cap);

typedef struct {
    TotalIdentity current;
    PriorityQueue ready_queue;
    Capability* device_cap;
    CapQueue send_cap;
    CapQueue recv_cap;
    TotalIdentity send_intr_transaction;
    Capability register_receiver_transaction;
    Capability register_sender_transaction;
} MoicHart;

struct RISCVMOICState {
    /*< private >*/
    SysBusDevice parent_obj;
    qemu_irq *external_irqs;

    /*< public >*/
    MemoryRegion mmio;

    /* properties */
    uint32_t hart_count;
    uint32_t external_irq_count;


    /* config */
    MoicHart* moicharts;
};

DeviceState *riscv_moic_create(hwaddr addr, uint32_t hart_count, uint32_t external_irq_count);

#endif
