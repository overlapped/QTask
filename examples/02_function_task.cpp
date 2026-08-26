/**
 * @file 02_function_task.cpp
 * @brief Использование QFunctionTask для быстрого запуска лямбд
 */

#include <qtask/qfunctiontask.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 2: Функциональная задача ===";
    
    // 1. Создаем задачу из лямбды
    auto task = QFunctionTask::createLambda([]() -> QVariant {
        qDebug() << "Лямбда выполняется в потоке:" << QThread::currentThreadId();
        
        // Имитация длительной работы
        int sum = 0;
        for (int i = 1; i <= 100; ++i) {
            sum += i;
            QThread::msleep(1);
        }
        
        return sum;
    });
    
    // 2. Подключаем сигналы - используем static_cast
    QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished), 
                     [](const QVariant& result) {
        qDebug() << "✅ Лямбда завершена! Результат:" << result.toInt();
    });
    
    // 3. Запускаем
    task->start();
    
    // 4. Получаем результат
    int result = task->waitForResult().toInt();
    qDebug() << "Результат:" << result;
    
    // 5. Задача с захватом параметров
    qDebug() << "\n--- Задача с захватом параметров ---";
    
    int a = 10;
    int b = 20;
    
    auto task2 = QFunctionTask::createLambda([a, b]() -> QVariant {
        return a * b;
    });
    
    task2->start();
    int result2 = task2->waitForResult().toInt();
    qDebug() << "Результат умножения" << a << "*" << b << "=" << result2;
    
    return 0;
}