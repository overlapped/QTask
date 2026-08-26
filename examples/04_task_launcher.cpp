/**
 * @file 04_task_launcher.cpp
 * @brief Использование QTaskLauncher для быстрого запуска задач
 */

#include <qtask/qtasklauncher.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

class Worker : public QObject
{
public:
    QString processData(const QString& input) {
        qDebug() << "Worker: Обработка" << input;
        QThread::msleep(50);
        return input.toUpper();
    }
    
    int multiply(int a, int b) {
        qDebug() << "Worker: Умножение" << a << "*" << b;
        QThread::msleep(30);
        return a * b;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 4: QTaskLauncher ===";
    
    // 1. Запуск лямбды
    qDebug() << "\n--- Запуск лямбды ---";
    auto future1 = QTaskLauncher::run([]() -> QVariant {
        int sum = 0;
        for (int i = 1; i <= 100; ++i) {
            sum += i;
        }
        return sum;
    });
    
    int result1 = future1.result().toInt();
    qDebug() << "Сумма чисел:" << result1;
    
    // 2. Запуск слота
    qDebug() << "\n--- Запуск слота ---";
    Worker worker;
    auto future2 = QTaskLauncher::runSlot(&worker, &Worker::processData, "hello world");
    QString result2 = future2.result().toString();
    qDebug() << "Результат:" << result2;
    
    // 3. Запуск слота с несколькими аргументами
    qDebug() << "\n--- Запуск слота с аргументами ---";
    auto future3 = QTaskLauncher::runSlot(&worker, &Worker::multiply, 6, 7);
    int result3 = future3.result().toInt();
    qDebug() << "6 * 7 =" << result3;
    
    // 4. Параллельные запуски
    qDebug() << "\n--- Параллельные запуски ---";
    auto f1 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(100);
        return "Первый";
    });
    auto f2 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(50);
        return "Второй";
    });
    auto f3 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(150);
        return "Третий";
    });
    
    qDebug() << "Результат 1:" << f1.result().toString();
    qDebug() << "Результат 2:" << f2.result().toString();
    qDebug() << "Результат 3:" << f3.result().toString();
    
    return 0;
}