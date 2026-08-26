#ifndef QTASK_H
#define QTASK_H

#include "qtask_global.h"
#include <QObject>
#include <QThread>
#include <QFuture>
#include <QFutureInterface>
#include <QAtomicInt>
#include <QPointer>
#include <QEventLoop>
#include <QVariant>
#include <exception>

/**
 * @brief Базовый класс для асинхронных задач
 * 
 * Использует QVariant для хранения результата
 * 
 * @code
 * class ComputeTask : public QTask {
 * protected:
 *     QVariant run() override {
 *         int result = 0;
 *         for (int i = 0; i <= 100; ++i) {
 *             if (shouldCancel()) return result;
 *             result += i;
 *             setProgress(i);
 *             QThread::msleep(10);
 *         }
 *         return result;
 *     }
 * };
 * 
 * auto task = std::make_shared<ComputeTask>();
 * task->start();
 * int result = task->waitForResult().toInt();
 * @endcode
 */
class QTASK_EXPORT QTask : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QTask)

public:
    using Ptr = std::shared_ptr<QTask>;
    using Future = QFuture<QVariant>;

    explicit QTask(QObject* parent = nullptr);
    virtual ~QTask();

    Future exec(QThread* thread);
    Future start();
    
    QVariant waitForResult();
    void wait();
    void cancel();
    
    bool isFinished() const noexcept;
    bool isCancelled() const noexcept;
    int progress() const noexcept;
    QString error() const noexcept;

signals:
    void finished();
    void finished(const QVariant& result);
    void cancelled();
    void taskError(const QString& message);  // Переименовано, чтобы избежать конфликта
    void progress(int percent);

protected:
    virtual QVariant run() = 0;
    virtual void onCancel();
    
    void setProgress(int percent);
    bool shouldCancel() const noexcept;
    void setResult(const QVariant& result);
    void setError(const QString& errorText);

private Q_SLOTS:
    void runTask();
    void finish();
    void finish(const QVariant& result);

private:
    QFutureInterface<QVariant> m_futureInterface;
    QVariant m_result;
    QString m_error;
    QAtomicInt m_cancelled;
    bool m_finished;
    int m_progress;
    QPointer<QThread> m_targetThread;
    QPointer<QThread> m_originalThread;
};

#endif // QTASK_H