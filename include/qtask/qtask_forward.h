#ifndef QTASK_FORWARD_H
#define QTASK_FORWARD_H

#include <memory>

namespace QTask {

template<typename T = void>
class QTask;

class QFunctionTask;

template<typename T>
class QSlotTask;

class QTaskLauncher;

class QFuture;

} // namespace QTask

#endif // QTASK_FORWARD_H