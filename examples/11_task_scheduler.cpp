/**
 * @file 11_task_scheduler.cpp
 * @brief Демонстрация планировщика последовательных задач
 */

#include <qtask/qtaskscheduler.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>

class DataGenerator : public QObject
{
public:
    QVariant generate1() {
        qDebug() << "Генератор 1: создаем данные...";
        QThread::msleep(100);
        return 100;
    }
    
    QVariant generate2() {
        qDebug() << "Генератор 2: создаем данные...";
        QThread::msleep(100);
        return 200;
    }
    
    QVariant processData() {
        qDebug() << "Обработка данных...";
        QThread::msleep(100);
        return 300;
    }
    
    QVariant saveResult(const QVariant& data) {
        qDebug() << "Сохранение результата:" << data.toInt();
        QThread::msleep(100);
        return data.toInt() + 1000;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 11: Планировщик задач ===";
    
    // 1. Простой планировщик с лямбдами
    qDebug() << "\n--- Простой планировщик ---";
    
    auto scheduler = QTaskScheduler::create();
    scheduler->add([]() -> QVariant {
        qDebug() << "Задача 1: Вычисление 1+2...";
        QThread::msleep(50);
        return 3;
    })->add([]() -> QVariant {
        qDebug() << "Задача 2: Вычисление 3*4...";
        QThread::msleep(50);
        return 12;
    })->add([]() -> QVariant {
        qDebug() << "Задача 3: Вычисление 5+6...";
        QThread::msleep(50);
        return 11;
    });
    
    // Используем scheduler.get() для получения сырого указателя
    QObject::connect(scheduler.get(), static_cast<void(QTaskScheduler::*)(int)>(&QTaskScheduler::progress), 
                     [](int percent) {
        qDebug() << "📊 Прогресс планировщика:" << percent << "%";
    });
    
    QObject::connect(scheduler.get(), &QTaskScheduler::taskCompleted, [](int index, int total) {
        qDebug() << "✅ Задача" << index << "из" << total << "завершена";
    });
    
    scheduler->start();
    QVariant result = scheduler->waitForResult();
    qDebug() << "✅ Результат последней задачи:" << result.toInt();
    
    // 2. Планировщик со слотами
    qDebug() << "\n--- Планировщик со слотами ---";
    
    DataGenerator generator;
    
    auto scheduler2 = QTaskScheduler::create();
    scheduler2->addSlot(&generator, &DataGenerator::generate1)
              ->addSlot(&generator, &DataGenerator::generate2)
              ->addSlot(&generator, &DataGenerator::processData)
              ->addSlot(&generator, &DataGenerator::saveResult, QVariant(42));
    
    scheduler2->start();
    QVariant result2 = scheduler2->waitForResult();
    qDebug() << "✅ Результат последней задачи:" << result2.toInt();
    
    // 3. Смешанный планировщик
    qDebug() << "\n--- Смешанный планировщик ---";
    
    auto scheduler3 = QTaskScheduler::create();
    scheduler3->add([]() -> QVariant {
        qDebug() << "Лямбда 1: Привет!";
        return "Hello";
    })->addSlot(&generator, &DataGenerator::generate1)
      ->add([]() -> QVariant {
        qDebug() << "Лямбда 2: Мир!";
        return "World";
      })->addSlot(&generator, &DataGenerator::saveResult, QVariant(99));
    
    scheduler3->start();
    QVariant result3 = scheduler3->waitForResult();
    qDebug() << "✅ Результат последней задачи:" << result3.toInt();
    
    // 4. Планировщик с отменой
    qDebug() << "\n--- Планировщик с отменой ---";
    
    auto scheduler4 = QTaskScheduler::create();
    scheduler4->add([]() -> QVariant {
        qDebug() << "Задача 1: Длительная операция...";
        QThread::msleep(300);
        return 1;
    })->add([]() -> QVariant {
        qDebug() << "Задача 2: Еще длительнее...";
        QThread::msleep(300);
        return 2;
    })->add([]() -> QVariant {
        qDebug() << "Задача 3: Последняя...";
        QThread::msleep(300);
        return 3;
    });
    
    scheduler4->start();
    
    // Отменяем через 500мс
    QTimer::singleShot(500, [scheduler4]() {
        qDebug() << "🛑 Отмена планировщика...";
        scheduler4->cancel();
    });
    
    try {
        QVariant result4 = scheduler4->waitForResult();
        qDebug() << "Результат планировщика 4:" << result4.toInt();
    } catch (const std::exception& e) {
        qDebug() << "❌ Планировщик 4:" << e.what();
    }
    
    // 5. Динамическое добавление задач
    qDebug() << "\n--- Динамическое добавление задач ---";
    
    auto scheduler5 = QTaskScheduler::create();
    scheduler5->add([]() -> QVariant {
        qDebug() << "Первая задача";
        return 1;
    });
    
    scheduler5->start();
    
    // Можно добавлять задачи даже после старта
    scheduler5->add([]() -> QVariant {
        qDebug() << "Вторая задача (добавлена позже)";
        return 2;
    });
    scheduler5->add([]() -> QVariant {
        qDebug() << "Третья задача (добавлена позже)";
        return 3;
    });
    
    try {
        QVariant result5 = scheduler5->waitForResult();
        qDebug() << "✅ Результат планировщика 5:" << result5.toInt();
    } catch (const std::exception& e) {
        qDebug() << "❌ Планировщик 5:" << e.what();
    }
    
    qDebug() << "\n✅ Пример завершен!";
    
    return 0;
}