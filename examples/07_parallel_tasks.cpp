/**
 * @file 07_parallel_tasks.cpp
 * @brief Демонстрация параллельного выполнения задач
 */

#include <qtask/qtask.h>
#include <qtask/qfunctiontask.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <vector>

/**
 * @brief Задача для вычисления факториала
 */
class FactorialTask : public QTask
{
public:
    FactorialTask(int n) : m_n(n) {}
    
protected:
    QVariant run() override {
        qDebug() << "FactorialTask: Вычисление" << m_n << "! в потоке" << QThread::currentThreadId();
        
        int result = 1;
        for (int i = 2; i <= m_n; ++i) {
            if (shouldCancel()) {
                return result;
            }
            result *= i;
            setProgress((i * 100) / m_n);
            QThread::msleep(10);
        }
        
        return result;
    }
    
private:
    int m_n;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 7: Параллельные задачи ===";
    
    QElapsedTimer timer;
    timer.start();
    
    // 1. Запуск нескольких задач параллельно
    qDebug() << "\n--- Параллельное выполнение ---";
    
    std::vector<int> numbers = {5, 7, 8, 10, 12};
    std::vector<std::shared_ptr<FactorialTask>> tasks;
    std::vector<QFuture<QVariant>> futures;
    
    for (int n : numbers) {
        auto task = std::make_shared<FactorialTask>(n);
        tasks.push_back(task);
        futures.push_back(task->start());
        
        // Подключаем сигнал завершения
        QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished), 
                         [n](const QVariant& result) {
            qDebug() << "✅" << n << "! =" << result.toInt();
        });
    }
    
    // Ждем завершения всех задач
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    qDebug() << "Все задачи завершены за" << timer.elapsed() << "мс";
    
    // 2. Сравнение последовательного и параллельного выполнения
    qDebug() << "\n--- Сравнение производительности ---";
    
    // Последовательное выполнение
    timer.restart();
    qDebug() << "Последовательное выполнение:";
    for (int n : {8, 10, 12}) {
        auto task = std::make_shared<FactorialTask>(n);
        task->start();
        int result = task->waitForResult().toInt();
        qDebug() << n << "! =" << result;
    }
    qDebug() << "Последовательно:" << timer.elapsed() << "мс";
    
    // Параллельное выполнение
    timer.restart();
    qDebug() << "Параллельное выполнение:";
    
    std::vector<std::shared_ptr<FactorialTask>> parallelTasks;
    for (int n : {8, 10, 12}) {
        auto task = std::make_shared<FactorialTask>(n);
        parallelTasks.push_back(task);
        task->start();
    }
    
    for (auto& task : parallelTasks) {
        int result = task->waitForResult().toInt();
        qDebug() << result << "(из параллельной задачи)";
    }
    
    qDebug() << "Параллельно:" << timer.elapsed() << "мс";
    
    // 3. Использование QTaskLauncher для параллельных запусков
    qDebug() << "\n--- QTaskLauncher параллельно ---";
    timer.restart();
    
    auto f1 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(100);
        return "A";
    });
    
    auto f2 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(50);
        return "B";
    });
    
    auto f3 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(75);
        return "C";
    });
    
    qDebug() << "Результаты:" << f1.result().toString() 
             << f2.result().toString() 
             << f3.result().toString();
    
    qDebug() << "Время выполнения:" << timer.elapsed() << "мс";
    
    return 0;
}