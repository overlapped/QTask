/**
 * @file 03_slot_task.cpp
 * @brief Использование QSlotTask для выполнения слотов объектов
 */

#include <qtask/qslottask.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

/**
 * @brief Тестовый объект с методами
 */
class DataProcessor : public QObject
{
public:
    QString processData(const QString& input) {
        qDebug() << "DataProcessor: Обработка" << input << "в потоке" << QThread::currentThreadId();
        QThread::msleep(100);
        return input.toUpper();
    }
    
    int calculate(int a, int b) {
        qDebug() << "DataProcessor: Вычисление" << a << "+" << b;
        QThread::msleep(50);
        return a + b;
    }
    
    void saveResult(const QString& data) {
        qDebug() << "DataProcessor: Сохранение результата:" << data;
        QThread::msleep(50);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 3: Слот-задачи ===";
    
    DataProcessor processor;
    
    // 1. Выполнение метода с результатом
    qDebug() << "\n--- Метод с результатом ---";
    auto task1 = QSlotTask::createMethod(&processor, &DataProcessor::processData, "hello world");
    
    QObject::connect(task1.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished), 
                     [](const QVariant& result) {
        qDebug() << "✅ Результат обработки:" << result.toString();
    });
    
    task1->start();
    QString result1 = task1->waitForResult().toString();
    qDebug() << "Получено:" << result1;
    
    // 2. Выполнение метода с несколькими аргументами
    qDebug() << "\n--- Метод с несколькими аргументами ---";
    auto task2 = QSlotTask::createMethod(&processor, &DataProcessor::calculate, 15, 27);
    task2->start();
    int result2 = task2->waitForResult().toInt();
    qDebug() << "Результат вычисления:" << result2;
    
    // 3. Void метод (без результата)
    qDebug() << "\n--- Void метод ---";
    auto task3 = QSlotTask::createMethod(&processor, &DataProcessor::saveResult, "Final data");
    task3->start();
    task3->wait();
    qDebug() << "Метод сохранения выполнен";
    
    // 4. Создание задачи из слота
    qDebug() << "\n--- Слот-задача ---";
    auto task4 = QSlotTask::createSlot(&processor, &DataProcessor::calculate, 100, 200);
    task4->start();
    int result4 = task4->waitForResult().toInt();
    qDebug() << "Результат слота:" << result4;
    
    return 0;
}