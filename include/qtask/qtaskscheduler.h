#ifndef QTASKSCHEDULER_H
#define QTASKSCHEDULER_H

#include "qtask_global.h"
#include "qtask.h"
#include "qfunctiontask.h"
#include "qslottask.h"
#include <QObject>
#include <queue>
#include <memory>
#include <functional>

class QTASK_EXPORT QTaskScheduler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QTaskScheduler)

public:
    using Ptr = std::shared_ptr<QTaskScheduler>;

    static Ptr create() {
        return Ptr(new QTaskScheduler());
      }

    template<typename Func>
    QTaskScheduler* add(Func func) {
        m_tasks.push([func]() -> QVariant {
            return func();
          });
        return this;
      }

    template<typename Lambda>
    QTaskScheduler* addLambda(Lambda&& lambda) {
        m_tasks.push([lambda]() -> QVariant {
            return lambda();
          });
        return this;
      }

    template<typename T>
    QTaskScheduler* addSlot(T* obj, QVariant (T::*slot)()) {
        m_tasks.push([obj, slot]() -> QVariant {
            return QVariant::fromValue((obj->*slot)());
          });
        return this;
      }

    template<typename T, typename... Args>
    QTaskScheduler* addSlot(T* obj, QVariant (T::*slot)(Args...), Args... args) {
        m_tasks.push([obj, slot, args...]() -> QVariant {
            return QVariant::fromValue((obj->*slot)(args...));
          });
        return this;
      }

    QTaskScheduler* addTask(std::shared_ptr<QTask> task);

    void start();
    QVariant waitForResult();
    void cancel();

    bool isFinished() const;
    bool isCancelled() const;
    int progress() const;
    size_t count() const;
    void clear();

signals:
    void finished();
    void finished(const QVariant& result);
    void cancelled();
    void taskError(const QString& message);
    void progress(int percent);
    void taskCompleted(int index, int total);

private:
    QTaskScheduler() = default;

    using TaskFunc = std::function<QVariant()>;
    std::queue<TaskFunc> m_tasks;

    std::shared_ptr<QTask> m_currentTask;
    int m_totalTasks = 0;
    int m_completedTasks = 0;
    bool m_finished = false;
    bool m_cancelled = false;
    QString m_error;
    QVariant m_lastResult;

    void executeNext();
};

#endif // QTASKSCHEDULER_H
