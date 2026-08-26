/**
 * @file 08_task_with_waiter.cpp
 * @brief Интеграция QTask и QSignalWaiter
 * 
 * Демонстрирует использование QSignalWaiter для ожидания сигналов задачи
 */

#include <qtask/qtask.h>
#include <qtask/qfunctiontask.h>
#include <qsignalwaiter/qsignalwaiter.h>
#include <qsignalwaiter/qmultisignalwaiter.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>

/**
 * @brief Задача с имитацией длительной работы
 */
class LongTask : public QTask
{
public:
    LongTask(int durationMs) : m_durationMs(durationMs) {}
    
protected:
    QVariant run() override {
        qDebug() << "LongTask: Начинаем работу на" << m_durationMs << "мс";
        
        int result = 0;
        for (int i = 0; i <= 100; ++i) {
            if (shouldCancel()) {
                qDebug() << "LongTask: Отменено!";
                return result;
            }
            
            QThread::msleep(m_durationMs / 100);
            result += i;
            setProgress(i);
        }
        
        qDebug() << "LongTask: Завершено! Результат:" << result;
        return result;
    }
    
private:
    int m_durationMs;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 8: Интеграция с QSignalWaiter ===";
    
    // 1. Ожидание сигнала finished с таймаутом
    qDebug() << "\n--- Ожидание с таймаутом ---";
    
    auto task1 = std::make_shared<LongTask>(2000); // 2 секунды
    task1->start();
    
    // Используем QSignalWaiter для ожидания с таймаутом
    QSignalWaiter waiter1(task1.get(), static_cast<void(QTask::*)()>(&QTask::finished));
    
    if (waiter1.wait(1000)) {
        qDebug() << "✅ Задача завершена вовремя!";
        QVariant result = task1->waitForResult();
        qDebug() << "Результат:" << result.toInt();
    } else {
        qDebug() << "❌ Таймаут! Отменяем задачу...";
        task1->cancel();
    }
    
    // 2. Ожидание с прогрессом
    qDebug() << "\n--- Ожидание с прогрессом ---";
    
    auto task2 = std::make_shared<LongTask>(300);
    task2->start();
    
    // Ожидаем с проверкой прогресса
    while (!task2->isFinished()) {
        QSignalWaiter waiter2(task2.get(), static_cast<void(QTask::*)(int)>(&QTask::progress));
        
        if (waiter2.wait(50)) {
            qDebug() << "📊 Прогресс:" << task2->progress() << "%";
        }
    }
    
    QVariant result2 = task2->waitForResult();
    qDebug() << "✅ Завершено! Результат:" << result2.toInt();
    
    // 3. Ожидание нескольких сигналов
    qDebug() << "\n--- Ожидание нескольких сигналов ---";
    
    auto task3 = std::make_shared<LongTask>(200);
    task3->start();
    
    QMultiSignalWaiter multiWaiter;
    multiWaiter.add(task3.get(), static_cast<void(QTask::*)()>(&QTask::finished), "finished");
    multiWaiter.add(task3.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), "progress");
    
    int idx = multiWaiter.wait(500);
    
    if (idx >= 0) {
        qDebug() << "Получен сигнал:" << multiWaiter.capturedName();
        if (multiWaiter.capturedName() == "finished") {
            QVariant result3 = task3->waitForResult();
            qDebug() << "Результат:" << result3.toInt();
        } else {
            qDebug() << "Прогресс:" << task3->progress() << "%";
        }
    } else {
        qDebug() << "❌ Таймаут!";
    }
    
    // 4. Комплексный пример: несколько задач с ожиданием
    qDebug() << "\n--- Несколько задач с QSignalWaiter ---";
    
    auto t1 = std::make_shared<LongTask>(150);
    auto t2 = std::make_shared<LongTask>(200);
    auto t3 = std::make_shared<LongTask>(100);
    
    t1->start();
    t2->start();
    t3->start();
    
    QMultiSignalWaiter waiter3;
    waiter3.add(t1.get(), static_cast<void(QTask::*)()>(&QTask::finished), "task1");
    waiter3.add(t2.get(), static_cast<void(QTask::*)()>(&QTask::finished), "task2");
    waiter3.add(t3.get(), static_cast<void(QTask::*)()>(&QTask::finished), "task3");
    
    // Ждем первую завершенную задачу
    idx = waiter3.wait(1000);
    if (idx >= 0) {
        qDebug() << "✅ Первая завершенная задача:" << waiter3.capturedName();
    }
    
    // Ждем все задачи
    while (!t1->isFinished() || !t2->isFinished() || !t3->isFinished()) {
        QThread::msleep(10);
    }
    
    qDebug() << "✅ Все задачи завершены!";
    qDebug() << "Результаты:" 
             << t1->waitForResult().toInt() << ", "
             << t2->waitForResult().toInt() << ", "
             << t3->waitForResult().toInt();
    
    return 0;
}