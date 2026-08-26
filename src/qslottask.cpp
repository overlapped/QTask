#include <qtask/qslottask.h>

std::shared_ptr<QSlotTask> QSlotTask::create(SlotFunc func) {
    auto task = std::make_shared<QSlotTask>();
    task->m_func = func;
    return task;
}

QVariant QSlotTask::run() {
    if (shouldCancel()) {
        return QVariant();
    }
    return m_func();
}