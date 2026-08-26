# QTask

[![CI](https://github.com/yourusername/qtask/workflows/CI/badge.svg)](https://github.com/yourusername/qtask/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Qt6](https://img.shields.io/badge/Qt-6.0+-green.svg)](https://www.qt.io)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

Современная библиотека для асинхронных задач в Qt6 с поддержкой C++17.

## 📦 Особенности

- ✅ **Современный C++17** - шаблоны, умные указатели, лямбды
- ✅ **Qt6 совместимость** - QFuture, QThread, сигналы/слоты
- ✅ **Интеграция с QSignalWaiter** - удобное ожидание сигналов
- ✅ **Отмена задач** - безопасная остановка выполнения
- ✅ **Прогресс** - обратная связь о выполнении
- ✅ **Обработка ошибок** - исключения + сигналы ошибок
- ✅ **Гибкость** - можно использовать с любыми функциями и слотами
- ✅ **Полное тестовое покрытие** - стабильность гарантирована

## 🚀 Установка

### Через CMake

```cmake
find_package(QTask REQUIRED)
find_package(QSignalWaiter REQUIRED)

target_link_libraries(your_app
    PRIVATE
        QTask::QTask
        QSignalWaiter::QSignalWaiter
)
```

## Использование

### Создание пользовательской задачи

```cpp
#include <qtask/qtask.h>

class SumTask : public QTask {
protected:
    QVariant run() override {
        int sum = 0;
        for (int i = 0; i <= 100; ++i) {
            if (shouldCancel()) return sum;
            sum += i;
            setProgress(i);
            QThread::msleep(10);
        }
        return sum;
    }
};

auto task = std::make_shared<SumTask>();
task->start();
int result = task->waitForResult().toInt(); // 5050
```

### Быстрый запуск лямбды

```cpp
#include <qtask/qfunctiontask.h>

auto task = QFunctionTask::createLambda([]() -> QVariant {
    QThread::sleep(2);
    return 42;
});

task->start();
int result = task->waitForResult().toInt(); // 42
```

### Выполнение слота

```cpp
#include <qtask/qslottask.h>

class Worker : public QObject {
public:
    QString processData(const QString& input) {
        return input.toUpper();
    }
};

Worker worker;
auto task = QSlotTask::createMethod(&worker, &Worker::processData, "hello");
task->start();
QString result = task->waitForResult().toString(); // "HELLO"
```

### Использование QTaskLauncher

```cpp
#include <qtask/qtasklauncher.h>

// Запуск лямбды
auto future = QTaskLauncher::run([]() -> QVariant {
    return 42;
});

// Запуск слота
Worker worker;
auto future2 = QTaskLauncher::runSlot(&worker, &Worker::processData, "hello");
```

### Интеграция с QSignalWaiter

```cpp
#include <qsignalwaiter/qsignalwaiter.h>

auto task = std::make_shared<LongTask>();
task->start();

QSignalWaiter waiter(task.get(), &QTask::finished);

if (waiter.wait(5000)) {
    int result = task->waitForResult().toInt();
    qDebug() << "Task finished with result:" << result;
} else {
    qDebug() << "Timeout!";
    task->cancel();
}
```

### Обработка ошибок

```cpp
auto task = QFunctionTask::createLambda([]() -> QVariant {
    throw std::runtime_error("Something went wrong");
    return QVariant();
});

task->start();

try {
    QVariant result = task->waitForResult();
} catch (const std::runtime_error& e) {
    qDebug() << "Error:" << e.what(); // "Something went wrong"
}
```

### Отслеживание прогресса и отмена

```cpp
auto task = std::make_shared<LongTask>();

// Подключаем прогресс
QObject::connect(task.get(), &QTask::progress, [](int percent) {
    qDebug() << "Progress:" << percent << "%";
});

// Подключаем завершение
QObject::connect(task.get(), &QTask::finished, [](const QVariant& result) {
    qDebug() << "Result:" << result.toInt();
});

task->start();

// Отмена через 1 секунду
QTimer::singleShot(1000, [task]() {
    task->cancel();
});

int result = task->waitForResult().toInt();
```
