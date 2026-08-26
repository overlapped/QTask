#include <qtask/qfunctiontask.h>

std::shared_ptr<QFunctionTask> QFunctionTask::create(Func func) {
    auto task = std::make_shared<QFunctionTask>();
    task->m_func = func;
    return task;
}

QVariant QFunctionTask::run() {
    if (shouldCancel()) {
        return QVariant();
    }
    return m_func();
}