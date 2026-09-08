/**
 * @file 10_task_chain.cpp
 * @brief Демонстрация цепочки последовательных задач
 */

#include <qtask/qtaskchain.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 10: Цепочка задач ===";
    
    // 1. Простая цепочка
    qDebug() << "\n--- Простая цепочка ---";
    
    auto chain = QTaskChain::create();
    chain->then([]() -> QVariant {
        qDebug() << "Шаг 1: Генерация данных...";
        QThread::msleep(100);
        return 100;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        int value = input.toInt();
        qDebug() << "Шаг 2: Умножение" << value << "* 2 = " << value * 2;
        QThread::msleep(100);
        return value * 2;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        int value = input.toInt();
        qDebug() << "Шаг 3: Добавление 10:" << value << "+ 10 = " << value + 10;
        QThread::msleep(100);
        return value + 10;
    });
    
    chain->start();
    int result = chain->waitForResult().toInt();
    qDebug() << "✅ Результат цепочки:" << result;
    
    // 2. Цепочка с прогрессом
    qDebug() << "\n--- Цепочка с прогрессом ---";
    
    auto chain2 = QTaskChain::create();
    chain2->then([]() -> QVariant {
        qDebug() << "Шаг A: Подготовка...";
        QThread::msleep(100);
        return "Hello";
    })->thenWithInput([](const QVariant& input) -> QVariant {
        QString str = input.toString();
        qDebug() << "Шаг B: Преобразование" << str << "->" << str.toUpper();
        QThread::msleep(100);
        return str.toUpper();
    })->thenWithInput([](const QVariant& input) -> QVariant {
        QString str = input.toString();
        qDebug() << "Шаг C: Добавление суффикса:" << str << "->" << str + " WORLD!";
        QThread::msleep(100);
        return str + " WORLD!";
    });
    
    QObject::connect(chain2.get(), static_cast<void(QTaskChain::*)(int)>(&QTaskChain::progress), 
                     [](int percent) {
        qDebug() << "📊 Прогресс цепочки:" << percent << "%";
    });
    
    QObject::connect(chain2.get(), &QTaskChain::stepCompleted, [](int step, int total) {
        qDebug() << "✅ Шаг" << step << "из" << total << "завершен";
    });
    
    chain2->start();
    QString result2 = chain2->waitForResult().toString();
    qDebug() << "✅ Результат цепочки 2:" << result2;
    
    // 3. Цепочка с обработкой ошибок
    qDebug() << "\n--- Цепочка с ошибкой ---";
    
    auto chain3 = QTaskChain::create();
    chain3->then([]() -> QVariant {
        qDebug() << "Шаг 1: Генерация числа...";
        return 42;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        int value = input.toInt();
        qDebug() << "Шаг 2: Проверка числа" << value;
        if (value > 40) {
            qDebug() << "❌ Ошибка: число слишком большое!";
            throw std::runtime_error("Number too large");
        }
        return value;
    });
    
    chain3->start();
    try {
        int result3 = chain3->waitForResult().toInt();
        qDebug() << "Результат цепочки 3:" << result3;
    } catch (const std::exception& e) {
        qDebug() << "❌ Ошибка в цепочке:" << e.what();
    }
    
    // 4. Отмена цепочки с длинными шагами
    qDebug() << "\n--- Отмена цепочки (длинные шаги) ---";
    
    auto chain4 = QTaskChain::create();
    chain4->then([]() -> QVariant {
        qDebug() << "Шаг 1: Длительная операция (3 сек)...";
        for (int i = 0; i < 30; ++i) {
            QThread::msleep(100);
        }
        return 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 2: Еще длительнее (3 сек)...";
        for (int i = 0; i < 30; ++i) {
            QThread::msleep(100);
        }
        return input.toInt() + 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 3: Завершаем (3 сек)...";
        for (int i = 0; i < 30; ++i) {
            QThread::msleep(100);
        }
        return input.toInt() + 1;
    });
    
    QObject::connect(chain4.get(), static_cast<void(QTaskChain::*)(int)>(&QTaskChain::progress), 
                     [](int percent) {
        if (percent % 20 == 0) {
            qDebug() << "📊 Прогресс цепочки 4:" << percent << "%";
        }
    });
    
    // Убираем Qt::UniqueConnection - используем обычное подключение
    QObject::connect(chain4.get(), &QTaskChain::cancelled, []() {
        qDebug() << "⛔ Цепочка 4 отменена!";
    });
    
    chain4->start();
    
    // Отменяем через 1.5 секунды
    QTimer::singleShot(1500, [chain4]() {
        qDebug() << "🛑 Отмена цепочки 4 через 1.5 сек...";
        chain4->cancel();
    });
    
    try {
        QVariant result4 = chain4->waitForResult();
        qDebug() << "Результат цепочки 4:" << result4.toInt();
    } catch (const std::exception& e) {
        qDebug() << "❌ Цепочка 4:" << e.what();
    }
    
    // 5. Отмена цепочки с короткими шагами (быстрая реакция)
    qDebug() << "\n--- Отмена цепочки (короткие шаги) ---";
    
    auto chain5 = QTaskChain::create();
    chain5->then([]() -> QVariant {
        qDebug() << "Шаг 1: Быстрая операция...";
        QThread::msleep(50);
        return 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 2: Быстрая операция...";
        QThread::msleep(50);
        return input.toInt() + 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 3: Быстрая операция...";
        QThread::msleep(50);
        return input.toInt() + 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 4: Быстрая операция...";
        QThread::msleep(50);
        return input.toInt() + 1;
    })->thenWithInput([](const QVariant& input) -> QVariant {
        qDebug() << "Шаг 5: Быстрая операция...";
        QThread::msleep(50);
        return input.toInt() + 1;
    });
    
    // Убираем Qt::UniqueConnection
    QObject::connect(chain5.get(), &QTaskChain::cancelled, []() {
        qDebug() << "⛔ Цепочка 5 отменена!";
    });
    
    chain5->start();
    
    // Отменяем через 100 мс
    QTimer::singleShot(100, [chain5]() {
        qDebug() << "🛑 Отмена цепочки 5 через 100 мс...";
        chain5->cancel();
    });
    
    try {
        QVariant result5 = chain5->waitForResult();
        qDebug() << "Результат цепочки 5:" << result5.toInt();
    } catch (const std::exception& e) {
        qDebug() << "❌ Цепочка 5:" << e.what();
    }
    
    qDebug() << "\n✅ Пример завершен!";
    
    return 0;
}