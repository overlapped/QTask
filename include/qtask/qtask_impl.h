#ifndef QTASK_IMPL_H
#define QTASK_IMPL_H

#include "qtask.h"
#include <QCoreApplication>

inline QTask::QTask(QObject* parent)
    : QObject(parent)
    , m_cancelled(false)
    , m_finished(false)
    , m_progress(0)
{
    m_futureInterface.reportStarted();
}

inline QTask::~QTask() {
    cancel();
    wait();
}

inline QTask::Future QTask::exec(QThread* thread) {
    if (!thread) {
        setError("Invalid thread");
        return m_futureInterface.future();
    }
    
    if (m_finished) {
        setError("Task already finished");
        return m_futureInterface.future();
    }
    
    m_targetThread = thread;
    m_originalThread = QThread::currentThread();
    
    moveToThread(thread);
    QMetaObject::invokeMethod(this, "runTask", Qt::QueuedConnection);
    
    return m_futureInterface.future();
}

inline QTask::Future QTask::start() {
    auto thread = new QThread();
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    
    connect(thread, &QThread::started, this, &QTask::runTask);
    connect(this, &QTask::finished, thread, &QThread::quit);
    
    thread->start();
    
    return m_futureInterface.future();
}

inline QVariant QTask::waitForResult() {
    if (!m_finished) {
        QEventLoop loop;
        connect(this, &QTask::finished, &loop, &QEventLoop::quit);
        connect(this, &QTask::error, &loop, &QEventLoop::quit);
        loop.exec();
    }
    
    if (m_cancelled) {
        throw std::runtime_error("Task cancelled");
    }
    
    if (!m_error.isEmpty()) {
        throw std::runtime_error(m_error.toStdString());
    }
    
    return m_result;
}

inline void QTask::wait() {
    waitForResult();
}

inline void QTask::cancel() {
    if (m_finished) {
        return;
    }
    
    m_cancelled = true;
    onCancel();
    
    if (!m_finished) {
        m_futureInterface.reportCanceled();
        emit cancelled();
        finish();
    }
}

inline bool QTask::isFinished() const noexcept {
    return m_finished;
}

inline bool QTask::isCancelled() const noexcept {
    return m_cancelled;
}

inline int QTask::progress() const noexcept {
    return m_progress;
}

inline QString QTask::error() const noexcept {
    return m_error;
}

inline void QTask::onCancel() {
    // Переопределяется в наследниках
}

inline void QTask::setProgress(int percent) {
    m_progress = qBound(0, percent, 100);
    emit progress(m_progress);
}

inline bool QTask::shouldCancel() const noexcept {
    return m_cancelled;
}

inline void QTask::setResult(const QVariant& result) {
    m_result = result;
}

inline void QTask::setError(const QString& error) {
    m_error = error;
    m_futureInterface.reportException(std::runtime_error(error.toStdString()));
    emit error(error);
}

inline void QTask::runTask() {
    try {
        if (m_cancelled) {
            emit cancelled();
            finish();
            return;
        }
        
        auto result = run();
        m_result = result;
        finish(result);
    } catch (const std::exception& e) {
        setError(QString("Exception: %1").arg(e.what()));
        finish();
    } catch (...) {
        setError("Unknown exception");
        finish();
    }
}

inline void QTask::finish() {
    m_finished = true;
    m_futureInterface.reportFinished();
    emit finished();
    
    if (m_originalThread && thread() != m_originalThread) {
        moveToThread(m_originalThread);
    }
}

inline void QTask::finish(const QVariant& result) {
    m_finished = true;
    m_futureInterface.reportFinished(&result);
    emit finished();
    emit finished(result);
    
    if (m_originalThread && thread() != m_originalThread) {
        moveToThread(m_originalThread);
    }
}

#endif // QTASK_IMPL_H