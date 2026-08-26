/**
 * @file 09_data_processing.cpp
 * @brief Реальный пример: обработка данных
 * 
 * Демонстрирует использование QTask в реальном сценарии
 */

#include <qtask/qtask.h>
#include <qtask/qfunctiontask.h>
#include <qtask/qslottask.h>
#include <qsignalwaiter/qsignalwaiter.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QVector>
#include <QElapsedTimer>
#include <random>

/**
 * @brief Генератор данных
 */
class DataGenerator : public QObject
{
public:
    QVector<int> generateData(int count) {
        qDebug() << "DataGenerator: Генерация" << count << "чисел";
        
        QVector<int> data;
        data.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);
        
        for (int i = 0; i < count; ++i) {
            data.append(dis(gen));
        }
        
        return data;
    }
};

/**
 * @brief Обработчик данных
 */
class DataProcessor : public QObject
{
public:
    // Обработка данных (требует времени)
    QVector<int> processData(const QVector<int>& input) {
        qDebug() << "DataProcessor: Обработка" << input.size() << "элементов";
        
        QVector<int> result;
        result.reserve(input.size());
        
        for (int value : input) {
            // Имитация сложной обработки
            QThread::msleep(1);
            result.append(value * 2);
        }
        
        return result;
    }
    
    // Фильтрация данных
    QVector<int> filterData(const QVector<int>& input, int threshold) {
        qDebug() << "DataProcessor: Фильтрация данных с порогом" << threshold;
        
        QVector<int> result;
        result.reserve(input.size());
        
        for (int value : input) {
            if (value > threshold) {
                result.append(value);
            }
        }
        
        return result;
    }
    
    // Статистика данных
    QString calculateStats(const QVector<int>& data) {
        qDebug() << "DataProcessor: Расчет статистики";
        
        if (data.isEmpty()) {
            return "Нет данных";
        }
        
        qint64 sum = 0;
        int min = data[0];
        int max = data[0];
        
        for (int value : data) {
            sum += value;
            if (value < min) min = value;
            if (value > max) max = value;
        }
        
        double avg = static_cast<double>(sum) / data.size();
        
        return QString("Статистика:\n"
                      "  Количество: %1\n"
                      "  Сумма: %2\n"
                      "  Среднее: %3\n"
                      "  Минимум: %4\n"
                      "  Максимум: %5")
                .arg(data.size())
                .arg(sum)
                .arg(avg, 0, 'f', 2)
                .arg(min)
                .arg(max);
    }
};

/**
 * @brief Задача обработки данных с прогрессом
 */
class DataProcessingTask : public QTask<QVector<int>>
{
public:
    DataProcessingTask(const QVector<int>& data) : m_data(data) {}
    
protected:
    QVector<int> run() override {
        qDebug() << "DataProcessingTask: Начинаем обработку" << m_data.size() << "элементов";
        
        QVector<int> result;
        result.reserve(m_data.size());
        
        for (int i = 0; i < m_data.size(); ++i) {
            if (shouldCancel()) {
                qDebug() << "DataProcessingTask: Отменено!";
                return result;
            }
            
            // Сложная обработка
            QThread::msleep(2);
            result.append(m_data[i] * 2);
            
            setProgress((i * 100) / m_data.size());
        }
        
        qDebug() << "DataProcessingTask: Завершено!";
        return result;
    }
    
private:
    QVector<int> m_data;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Пример 9: Обработка данных ===";
    
    QElapsedTimer timer;
    timer.start();
    
    // 1. Генерация данных
    qDebug() << "\n--- Шаг 1: Генерация данных ---";
    DataGenerator generator;
    DataProcessor processor;
    
    // Генерируем данные в отдельном потоке
    auto genTask = QSlotTask::createMethod(&generator, &DataGenerator::generateData, 500);
    genTask->start();
    QVector<int> data = genTask->waitForResult().value<QVector<int>>();
    
    qDebug() << "Сгенерировано" << data.size() << "чисел";
    qDebug() << "Первые 5 чисел:" << data.mid(0, 5);
    
    // 2. Обработка данных с прогрессом
    qDebug() << "\n--- Шаг 2: Обработка данных ---";
    
    auto processTask = std::make_shared<DataProcessingTask>(data);
    
    // Подключаем прогресс
    QObject::connect(processTask.get(), &QTask<QVector<int>>::progress, [](int percent) {
        if (percent % 10 == 0) {
            qDebug() << "📊 Прогресс обработки:" << percent << "%";
        }
    });
    
    processTask->start();
    QVector<int> processedData = processTask->waitForResult();
    
    qDebug() << "Обработано" << processedData.size() << "чисел";
    qDebug() << "Первые 5 результатов:" << processedData.mid(0, 5);
    
    // 3. Фильтрация данных
    qDebug() << "\n--- Шаг 3: Фильтрация данных ---";
    
    int threshold = 500;
    auto filterTask = QSlotTask::createMethod(&processor, &DataProcessor::filterData, processedData, threshold);
    filterTask->start();
    QVector<int> filteredData = filterTask->waitForResult().value<QVector<int>>();
    
    qDebug() << "Отфильтровано данных (порог" << threshold << "):" << filteredData.size();
    qDebug() << "Первые 5 отфильтрованных:" << filteredData.mid(0, 5);
    
    // 4. Статистика
    qDebug() << "\n--- Шаг 4: Статистика ---";
    
    auto statsTask = QSlotTask::createMethod(&processor, &DataProcessor::calculateStats, filteredData);
    statsTask->start();
    QString stats = statsTask->waitForResult().toString();
    
    qDebug() << "\n" << stats;
    
    // 5. Параллельная обработка разных частей данных
    qDebug() << "\n--- Шаг 5: Параллельная обработка ---";
    
    timer.restart();
    
    // Разбиваем данные на части
    int chunkSize = data.size() / 4;
    std::vector<QVector<int>> chunks;
    
    for (int i = 0; i < data.size(); i += chunkSize) {
        chunks.push_back(data.mid(i, chunkSize));
    }
    
    // Запускаем обработку каждой части параллельно
    std::vector<std::shared_ptr<DataProcessingTask>> tasks;
    std::vector<QFuture<QVector<int>>> futures;
    
    for (const auto& chunk : chunks) {
        auto task = std::make_shared<DataProcessingTask>(chunk);
        tasks.push_back(task);
        futures.push_back(task->start());
    }
    
    // Собираем результаты
    QVector<int> combinedResult;
    for (auto& future : futures) {
        QVector<int> chunkResult = future.result();
        combinedResult.append(chunkResult);
    }
    
    qDebug() << "Параллельная обработка завершена за" << timer.elapsed() << "мс";
    qDebug() << "Всего обработано:" << combinedResult.size() << "чисел";
    
    // 6. Использование QSignalWaiter для ожидания всех задач
    qDebug() << "\n--- Шаг 6: Ожидание всех задач ---";
    
    std::vector<std::shared_ptr<DataProcessingTask>> tasks2;
    for (int i = 0; i < 3; ++i) {
        auto task = std::make_shared<DataProcessingTask>(data.mid(0, 50));
        tasks2.push_back(task);
        task->start();
    }
    
    // Ждем первую завершенную задачу
    QMultiSignalWaiter waiter;
    for (size_t i = 0; i < tasks2.size(); ++i) {
        waiter.add(tasks2[i].get(), &QTask<QVector<int>>::finished, 
                   QString("task_%1").arg(i));
    }
    
    int firstIdx = waiter.wait(5000);
    if (firstIdx >= 0) {
        qDebug() << "✅ Первая завершенная задача:" << waiter.capturedName();
        qDebug() << "Результат (первые 5 элементов):" 
                 << tasks2[firstIdx]->waitForResult().mid(0, 5);
    }
    
    qDebug() << "\n✅ Все операции завершены! Общее время:" << timer.elapsed() << "мс";
    
    return 0;
}