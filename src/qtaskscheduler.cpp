#include "qtaskscheduler.h"
#include <QEventLoop>

QTaskScheduler* QTaskScheduler::addTask(std::shared_ptr<QTask> task) {
    m_tasks.push([task]() -> QVariant {
        return task->waitForResult();
      });
    return this;
  }

void QTaskScheduler::start() {
    if (m_tasks.empty()) {
        qWarning() << "QTaskScheduler: No tasks to execute";
        return;
      }

    m_totalTasks = static_cast<int>(m_tasks.size());
    m_completedTasks = 0;
    m_finished = false;
    m_cancelled = false;
    m_error.clear();
    m_lastResult = QVariant();

    executeNext();
  }

QVariant QTaskScheduler::waitForResult() {
    if (m_finished) {
        if (m_cancelled) {
            throw std::runtime_error("Task scheduler cancelled");
          }
        if (!m_error.isEmpty()) {
            throw std::runtime_error(m_error.toStdString());
          }
        return m_lastResult;
      }

    QEventLoop loop;
    QObject::connect(this, static_cast<void(QTaskScheduler::*)()>(&QTaskScheduler::finished),
        &loop, &QEventLoop::quit);
    QObject::connect(this, &QTaskScheduler::taskError,
        &loop, &QEventLoop::quit);
    QObject::connect(this, &QTaskScheduler::cancelled,
        &loop, &QEventLoop::quit);
    loop.exec();

    if (m_cancelled) {
        throw std::runtime_error("Task scheduler cancelled");
      }

    if (!m_error.isEmpty()) {
        throw std::runtime_error(m_error.toStdString());
      }

    return m_lastResult;
  }

void QTaskScheduler::cancel() {
    if (m_finished || m_cancelled) {
        return;
      }

    m_cancelled = true;
    if (m_currentTask) {
        m_currentTask->cancel();
      }

    emit cancelled();
  }

bool QTaskScheduler::isFinished() const {
    return m_finished;
  }

bool QTaskScheduler::isCancelled() const {
    return m_cancelled;
  }

int QTaskScheduler::progress() const {
    if (m_totalTasks == 0) return 0;
    if (m_finished) return 100;
    return (m_completedTasks * 100) / m_totalTasks;
  }

size_t QTaskScheduler::count() const {
    return m_tasks.size();
  }

void QTaskScheduler::clear() {
    while (!m_tasks.empty()) {
        m_tasks.pop();
      }
  }

void QTaskScheduler::executeNext() {
    if (m_cancelled) {
        if (!m_finished) {
            m_finished = true;
            emit cancelled();
          }
        return;
      }

    if (m_tasks.empty()) {
        m_finished = true;
        emit finished();
        emit finished(m_lastResult);
        emit progress(100);
        return;
      }

    auto func = m_tasks.front();
    m_tasks.pop();

    auto task = QFunctionTask::create(func);
    m_currentTask = task;

    QTaskScheduler* scheduler = this;

    QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished),
    [scheduler](const QVariant& result) {
        scheduler->m_lastResult = result;
        scheduler->m_completedTasks++;
        emit scheduler->taskCompleted(scheduler->m_completedTasks, scheduler->m_totalTasks);
        emit scheduler->progress((scheduler->m_completedTasks * 100) / scheduler->m_totalTasks);
        scheduler->executeNext();
      });

    QObject::connect(task.get(), &QTask::taskError, [scheduler](const QString& error) {
        scheduler->m_error = error;
        scheduler->m_finished = true;
        emit scheduler->taskError(error);
      });

    QObject::connect(task.get(), &QTask::cancelled,
    [scheduler]() {
        if (!scheduler->m_cancelled) {
            scheduler->m_cancelled = true;
            scheduler->m_finished = true;
            emit scheduler->cancelled();
          }
      });

    task->start();
  }
