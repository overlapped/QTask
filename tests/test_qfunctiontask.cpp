#include <gtest/gtest.h>
#include <qtask/qfunctiontask.h>
#include <qtask/qtasklauncher.h>
#include <QThread>
#include <QCoreApplication>
#include <QObject>

// Вспомогательный класс для тестирования прогресса
class ProgressFunctionTask : public QFunctionTask {
public:
    static std::shared_ptr<ProgressFunctionTask> create() {
        return std::make_shared<ProgressFunctionTask>();
    }
    
protected:
    QVariant run() override {
        int sum = 0;
        for (int i = 0; i <= 100; ++i) {
            sum += i;
            setProgress(i);
            QThread::msleep(1);
        }
        return sum;
    }
};

class TestQFunctionTask : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char* argv[] = {nullptr};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }
    }
    
    void TearDown() override {
        if (m_app) {
            m_app.reset();
        }
    }
    
    std::unique_ptr<QCoreApplication> m_app;
};

TEST_F(TestQFunctionTask, LambdaExecution) {
    auto task = QFunctionTask::createLambda([]() -> QVariant {
        int sum = 0;
        for (int i = 0; i <= 100; ++i) {
            sum += i;
        }
        return sum;
    });
    
    task->start();
    QVariant result = task->waitForResult();
    
    EXPECT_EQ(result.toInt(), 5050);
}

TEST_F(TestQFunctionTask, LambdaWithCapturedParameters) {
    int a = 10;
    int b = 20;
    
    auto task = QFunctionTask::createLambda([a, b]() -> QVariant {
        return a + b;
    });
    
    task->start();
    QVariant result = task->waitForResult();
    
    EXPECT_EQ(result.toInt(), 30);
}

TEST_F(TestQFunctionTask, LambdaWithBind) {
    auto sumFunc = std::bind([](int x, int y) { return x + y; }, 15, 25);
    
    auto task = QFunctionTask::create([sumFunc]() -> QVariant {
        return sumFunc();
    });
    
    task->start();
    QVariant result = task->waitForResult();
    
    EXPECT_EQ(result.toInt(), 40);
}

TEST_F(TestQFunctionTask, Cancel) {
    auto task = QFunctionTask::createLambda([]() -> QVariant {
        for (int i = 0; i < 100; ++i) {
            QThread::msleep(10);
        }
        return 42;
    });
    
    task->start();
    QThread::msleep(50);
    task->cancel();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_TRUE(task->isCancelled());
    EXPECT_TRUE(task->isFinished());
}

TEST_F(TestQFunctionTask, Error) {
    auto task = QFunctionTask::createLambda([]() -> QVariant {
        throw std::runtime_error("Test exception");
        return QVariant();
    });
    
    task->start();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_FALSE(task->error().isEmpty());
}

TEST_F(TestQFunctionTask, MultipleTasks) {
    auto task1 = QFunctionTask::createLambda([]() -> QVariant { 
        QThread::msleep(50); 
        return 100; 
    });
    
    auto task2 = QFunctionTask::createLambda([]() -> QVariant { 
        QThread::msleep(100); 
        return 200; 
    });
    
    task1->start();
    task2->start();
    
    QVariant result1 = task1->waitForResult();
    QVariant result2 = task2->waitForResult();
    
    EXPECT_EQ(result1.toInt(), 100);
    EXPECT_EQ(result2.toInt(), 200);
}

TEST_F(TestQFunctionTask, Progress) {
    auto task = ProgressFunctionTask::create();
    
    int lastProgress = -1;
    QObject::connect(task.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [&](int percent) {
        EXPECT_GE(percent, 0);
        EXPECT_LE(percent, 100);
        EXPECT_GE(percent, lastProgress);
        lastProgress = percent;
    });
    
    task->start();
    QVariant result = task->waitForResult();
    
    EXPECT_EQ(result.toInt(), 5050);
    EXPECT_GE(lastProgress, 0);
}

TEST_F(TestQFunctionTask, QuickLaunch) {
    auto future = QTaskLauncher::run([]() -> QVariant {
        int sum = 0;
        for (int i = 0; i <= 100; ++i) {
            sum += i;
        }
        return sum;
    });
    
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 5050);
}