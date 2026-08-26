/**
 * @file 06_error_handling.cpp
 * @brief Демонстрация обработки ошибок в задачах
 */

#include <qtask/qtask.h>
#include <qtask/qfunctiontask.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <random>

/**
 * @brief Задача с возможной ошибкой
 */
class RandomTask : public QTask<int>
{
protected:
    int run() override {
        qDebug() << "RandomTask: Выполняем работу...";
        QThread::msleep(100);
        
        // Случайным образом генерируем ошибку
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        int errorCode = dis(gen);
        if (errorCode == 0) {
            qDebug() << "✅ Успех!";
            return 100;
        } else if (errorCode == 1) {
            qDebug() << "❌ Ошибка: Недостаточно данных";
            setError("Недостаточно данных для обработки");
            return 0;
        } else {
            qDebug() << "❌ Ошибка: Таймаут соединения";
            setError("Таймаут соединения с сервером");
            return 0;
        }
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 6: Обработка ошибок ===";
    
    // 1. Обработка ошибок через try-catch
    qDebug() << "\n--- Метод 1: try-catch ---";
    auto task1 = std::make_shared<RandomTask>();
    
    QObject::connect(task1.get(), &QTask::taskError, [](const QString& error) {
        qDebug() << "🔴 Сигнал ошибки:" << error;
    });
    
    task1->start();
    
    try {
        int result = task1->waitForResult();
        qDebug() << "Результат:" << result;
    } catch (const std::runtime_error& e) {
        qDebug() << "🔴 Исключение:" << e.what();
    }
    
    // 2. Обработка ошибок через проверку error()
    qDebug() << "\n--- Метод 2: Проверка error() ---";
    
    // Создаем задачу, которая точно завершится с ошибкой
    auto task2 = QFunctionTask::createLambda([]() -> QVariant {
        qDebug() << "Генерируем ошибку...";
        throw std::runtime_error("Критическая ошибка в лямбде");
        return QVariant();
    });
    
    task2->start();
    
    try {
        QVariant result = task2->waitForResult();
        qDebug() << "Результат:" << result.toString();
    } catch (const std::runtime_error& e) {
        qDebug() << "🔴 Перехвачено исключение:" << e.what();
        qDebug() << "Текст ошибки из задачи:" << task2->error();
    }
    
    // 3. Обработка ошибок с QSignalWaiter
    qDebug() << "\n--- Метод 3: QSignalWaiter ---";
    auto task3 = std::make_shared<RandomTask>();
    task3->start();
    
    bool hasError = false;
    QObject::connect(task3.get(), &QTask::taskError, [&hasError](const QString&) {
        hasError = true;
    });
    
    try {
        int result = task3->waitForResult();
        if (!hasError) {
            qDebug() << "✅ Успех! Результат:" << result;
        }
    } catch (const std::exception& e) {
        qDebug() << "🔴 Ошибка:" << e.what();
    }
    
    // 4. Демонстрация разных типов ошибок
    qDebug() << "\n--- Демонстрация разных ошибок ---";
    
    std::vector<std::shared_ptr<RandomTask>> tasks;
    for (int i = 0; i < 5; ++i) {
        auto task = std::make_shared<RandomTask>();
        tasks.push_back(task);
        
        QObject::connect(task.get(), &QTask::taskError, [i](const QString& error) {
            qDebug() << "Задача" << i << "ошибка:" << error;
        });
        
        task->start();
    }
    
    for (auto& task : tasks) {
        try {
            task->waitForResult();
        } catch (const std::exception& e) {
            // Игнорируем
        }
    }
    
    return 0;
}