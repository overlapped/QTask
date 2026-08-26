/**
 * @file 05_progress_and_cancel.cpp
 * @brief Демонстрация отслеживания прогресса и отмены задачи
 */

#include <qtask/qtask.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

/**
 * @brief Задача с длительной работой и прогрессом
 */
class LongTask : public QTask
{
public:
    LongTask(int stepMs = 100) : m_stepMs(stepMs) {}
    
protected:
    QVariant run() override {
        qDebug() << "LongTask: Начинаем длительную работу (шаг" << m_stepMs << "мс)...";
        
        int result = 0;
        for (int i = 0; i <= 100; ++i) {
            // Проверяем отмену на каждом шаге
            if (shouldCancel()) {
                qDebug() << "❌ Задача отменена на" << i << "%";
                return result;
            }
            
            // Имитация работы
            QThread::msleep(m_stepMs);
            result += i;
            
            // Обновляем прогресс
            setProgress(i);
        }
        
        qDebug() << "✅ Задача завершена! Результат:" << result;
        return result;
    }
    
private:
    int m_stepMs;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 5: Прогресс и отмена ===";
    qDebug() << "ВНИМАНИЕ: Используются синхронные задержки для демонстрации отмены\n";
    
    // 1. Демонстрация отмены с синхронной задержкой
    qDebug() << "--- Отмена задачи через 2000мс (синхронно) ---";
    auto task = std::make_shared<LongTask>(200);
    
    QObject::connect(task.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [](int percent) {
        if (percent % 10 == 0) {
            qDebug() << "📊 Прогресс:" << percent << "%";
        }
    });
    
    QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished), 
                     [](const QVariant& result) {
        qDebug() << "🎯 Завершено с результатом:" << result.toInt();
    });
    
    QObject::connect(task.get(), &QTask::cancelled, []() {
        qDebug() << "⛔ Задача отменена пользователем";
    });
    
    // Запускаем задачу
    task->start();
    
    // Синхронная задержка в главном потоке
    qDebug() << "Ждем 2000мс перед отменой...";
    QThread::msleep(2000);
    
    qDebug() << "\n🛑 Отмена задачи...";
    task->cancel();
    
    try {
        QVariant result = task->waitForResult();
        qDebug() << "Итоговый результат:" << result.toInt();
    } catch (const std::exception& e) {
        qDebug() << "Исключение:" << e.what();
    }
    qDebug() << "---\n";
    
    // 2. Отмена через 500мс
    qDebug() << "--- Отмена через 500мс (синхронно) ---";
    auto task2 = std::make_shared<LongTask>(200);
    
    QObject::connect(task2.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [](int percent) {
        if (percent % 10 == 0) {
            qDebug() << "📊 Задача 2 - прогресс:" << percent << "%";
        }
    });
    
    task2->start();
    
    QThread::msleep(500);
    qDebug() << "🛑 Отмена задачи 2 через 500мс...";
    task2->cancel();
    
    try {
        QVariant result2 = task2->waitForResult();
        qDebug() << "Результат задачи 2:" << result2.toInt();
    } catch (const std::exception& e) {
        qDebug() << "Задача 2 - исключение:" << e.what();
    }
    qDebug() << "---\n";
    
    // 3. Задача без отмены
    qDebug() << "--- Задача без отмены (10мс на шаг) ---";
    auto task3 = std::make_shared<LongTask>(10);
    
    QObject::connect(task3.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [](int percent) {
        if (percent % 20 == 0) {
            qDebug() << "📊 Задача 3 - прогресс:" << percent << "%";
        }
    });
    
    task3->start();
    QVariant result3 = task3->waitForResult();
    qDebug() << "Результат задачи 3:" << result3.toInt();
    qDebug() << "---\n";
    
    // 4. Отмена до запуска
    qDebug() << "--- Отмена до запуска ---";
    auto task4 = std::make_shared<LongTask>(200);
    task4->cancel();
    
    QObject::connect(task4.get(), &QTask::cancelled, []() {
        qDebug() << "⛔ Задача 4 отменена до запуска";
    });
    
    task4->start();
    
    try {
        QVariant result4 = task4->waitForResult();
        qDebug() << "Результат задачи 4:" << result4.toInt();
    } catch (const std::exception& e) {
        qDebug() << "Задача 4 - исключение:" << e.what();
    }
    qDebug() << "---\n";
    
    // 5. Демонстрация: несколько попыток отмены
    qDebug() << "--- Несколько попыток отмены ---";
    auto task5 = std::make_shared<LongTask>(150);
    
    task5->start();
    
    // Отменяем несколько раз с синхронными задержками
    QThread::msleep(100);
    qDebug() << "🛑 Попытка отмены 1 (100мс)";
    task5->cancel();
    
    QThread::msleep(100);
    qDebug() << "🛑 Попытка отмены 2 (200мс)";
    task5->cancel();
    
    QThread::msleep(100);
    qDebug() << "🛑 Попытка отмены 3 (300мс)";
    task5->cancel();
    
    try {
        QVariant result5 = task5->waitForResult();
        qDebug() << "Результат задачи 5:" << result5.toInt();
    } catch (const std::exception& e) {
        qDebug() << "Задача 5 - исключение:" << e.what();
    }
    qDebug() << "---\n";
    
    // 6. Демонстрация: отмена на разных этапах
    qDebug() << "--- Отмена на разных этапах ---";
    
    auto task6 = std::make_shared<LongTask>(100);
    task6->start();
    
    // Ждем немного и отменяем
    for (int delay : {200, 400, 600}) {
        QThread::msleep(200);
        qDebug() << "🛑 Отмена на этапе" << delay << "мс";
        task6->cancel();
        if (task6->isCancelled()) {
            break;
        }
    }
    
    try {
        QVariant result6 = task6->waitForResult();
        qDebug() << "Результат задачи 6:" << result6.toInt();
    } catch (const std::exception& e) {
        qDebug() << "Задача 6 - исключение:" << e.what();
    }
    qDebug() << "---\n";
    
    // 7. Демонстрация shouldCancel() с проверкой состояния
    qDebug() << "--- Демонстрация shouldCancel() ---";
    
    class LoopTask : public QTask
    {
    protected:
        QVariant run() override {
            qDebug() << "LoopTask: Начинаем работу...";
            
            int result = 0;
            for (int i = 0; i <= 100; ++i) {
                if (shouldCancel()) {
                    qDebug() << "❌ LoopTask отменена на шаге" << i;
                    return result;
                }
                
                QThread::msleep(100);
                result += i;
                setProgress(i);
            }
            
            return result;
        }
    };
    
    auto task7 = std::make_shared<LoopTask>();
    task7->start();
    
    QThread::msleep(1200);
    qDebug() << "🛑 Отмена LoopTask через 1200мс...";
    task7->cancel();
    
    try {
        QVariant result7 = task7->waitForResult();
        qDebug() << "Результат LoopTask:" << result7.toInt();
    } catch (const std::exception& e) {
        qDebug() << "LoopTask - исключение:" << e.what();
    }
    
    qDebug() << "\n✅ Пример завершен!";
    
    return 0;
}