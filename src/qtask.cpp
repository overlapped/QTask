#include <qtask/qtask.h>
#include <QCoreApplication>
#include <QException>

QTask::QTask(QObject* parent)
    : QObject(parent)
    , m_cancelled(false)
    , m_finished(false)
    , m_progress(0)
{
    m_futureInterface.reportStarted();
}

QTask::~QTask() {
    cancel();
    wait();
}

QTask::Future QTask::exec(QThread* thread) {
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

QTask::Future QTask::start() {
    auto thread = new QThread();
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::started, this, &QTask::runTask);
    connect(this, static_cast<void(QTask::*)()>(&QTask::finished), thread, &QThread::quit);

    thread->start();
    return m_futureInterface.future();
  }


QVariant QTask::waitForResult() {
    wait();
    if (m_cancelled) {
        throw std::runtime_error("Task cancelled");
      }
    if (!m_error.isEmpty()) {
        throw std::runtime_error(m_error.toStdString());
      }
    return m_result;
  }

void QTask::wait() {
    if (!m_finished) {
        QEventLoop loop;
        connect(this, static_cast<void(QTask::*)()>(&QTask::finished), &loop, &QEventLoop::quit);
        connect(this, &QTask::taskError, &loop, &QEventLoop::quit);
        loop.exec();
      }
  }

void QTask::cancel() {
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

bool QTask::isFinished() const noexcept {
    return m_finished;
}

bool QTask::isCancelled() const noexcept {
    return m_cancelled;
}

int QTask::progress() const noexcept {
    return m_progress;
}

QString QTask::error() const noexcept {
    return m_error;
}

void QTask::onCancel() {
    // Переопределяется в наследниках
}

void QTask::setProgress(int percent) {
    m_progress = qBound(0, percent, 100);
    emit progress(m_progress);
}

bool QTask::shouldCancel() const noexcept {
    return m_cancelled;
}

void QTask::setResult(const QVariant& result) {
    m_result = result;
}

void QTask::setError(const QString& errorText) {
    m_error = errorText;
    // Используем QException вместо std::runtime_error
    QException exception;
    m_futureInterface.reportException(exception);
    emit taskError(errorText);
}

void QTask::runTask() {
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

void QTask::finish() {
    m_finished = true;
    m_futureInterface.reportFinished();
    emit finished();
    
    if (m_originalThread && thread() != m_originalThread) {
        moveToThread(m_originalThread);
    }
}

void QTask::finish(const QVariant& result) {
    m_finished = true;
    m_futureInterface.reportFinished(&result);
    emit finished();
    emit finished(result);
    
    if (m_originalThread && thread() != m_originalThread) {
        moveToThread(m_originalThread);
    }
}
