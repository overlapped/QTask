/**
 * @file 01_simple_task.cpp
 * @brief Базовое использование QTask
 * 
 * Демонстрирует создание пользовательской задачи
 */

#include <qtask/qtask.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

/**
 * @brief Задача для вычисления суммы чисел
 */
class SumTask : public QTask
{
protected:
    QVariant run() override {
        qDebug() << "SumTask: Начинаем вычисления в потоке" << QThread::currentThreadId();
        
        int sum = 0;
        for (int i = 1; i <= 100; ++i) {
            if (shouldCancel()) {
                qDebug() << "SumTask: Отменено!";
                return sum;
            }
            sum += i;
            // Имитация работы
            QThread::msleep(2);
        }
        
        qDebug() << "SumTask: Завершено! Результат:" << sum;
        return sum;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 1: Простая задача ===";
    qDebug() << "Главный поток:" << QThread::currentThreadId();
    
    // 1. Создаем задачу
    auto task = std::make_shared<SumTask>();
    
    // 2. Подключаем сигналы - используем static_cast для разрешения перегрузки
    QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished), 
                     [](const QVariant& result) {
        qDebug() << "✅ Задача завершена! Результат:" << result.toInt();
    });
    
    QObject::connect(task.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [](int percent) {
        qDebug() << "📊 Прогресс:" << percent << "%";
    });
    
    // 3. Запускаем задачу
    qDebug() << "Запускаем задачу...";
    auto future = task->start();
    
    // 4. Ожидаем результат (блокирующее ожидание)
    QVariant result = task->waitForResult();
    qDebug() << "Результат вычислений:" << result.toInt();
    
    return 0;
}