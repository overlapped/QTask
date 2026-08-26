#include <gtest/gtest.h>
#include <qtask/qtask.h>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QEventLoop>

class TestTask : public QTask {
protected:
    QVariant run() override {
        int sum = 0;
        for (int i = 0; i <= 100; ++i) {
            if (shouldCancel()) {
                return sum;
            }
            sum += i;
            setProgress(i);
            QThread::msleep(1);
        }
        return sum;
    }
};

class TestQTask : public ::testing::Test {
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

TEST_F(TestQTask, BasicExecution) {
    auto task = std::make_shared<TestTask>();
    auto future = task->start();
    
    QVariant result = task->waitForResult();
    EXPECT_EQ(result.toInt(), 5050);
    EXPECT_TRUE(task->isFinished());
}

TEST_F(TestQTask, Cancel) {
    auto task = std::make_shared<TestTask>();
    auto future = task->start();
    
    QThread::msleep(50);
    task->cancel();
    
    // Используем EXPECT_THROW для перехвата исключения
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_TRUE(task->isCancelled());
    EXPECT_TRUE(task->isFinished());
}

TEST_F(TestQTask, Progress) {
    auto task = std::make_shared<TestTask>();
    
    int lastProgress = -1;
    QObject::connect(task.get(), static_cast<void(QTask::*)(int)>(&QTask::progress), 
                     [&](int percent) {
        EXPECT_GE(percent, 0);
        EXPECT_LE(percent, 100);
        EXPECT_GE(percent, lastProgress);
        lastProgress = percent;
    });
    
    task->start();
    task->waitForResult();
    EXPECT_GE(lastProgress, 0);
}

TEST_F(TestQTask, Error) {
    class ErrorTask : public QTask {
    protected:
        QVariant run() override {
            setError("Test error");
            return QVariant();
        }
    };
    
    auto task = std::make_shared<ErrorTask>();
    task->start();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_FALSE(task->error().isEmpty());
}

TEST_F(TestQTask, ExecOnThread) {
    auto task = std::make_shared<TestTask>();
    auto thread = new QThread();
    thread->start();
    
    auto future = task->exec(thread);
    QVariant result = task->waitForResult();
    
    EXPECT_EQ(result.toInt(), 5050);
    
    thread->quit();
    thread->wait();
    delete thread;
}

TEST_F(TestQTask, MultipleTasks) {
    auto task1 = std::make_shared<TestTask>();
    auto task2 = std::make_shared<TestTask>();
    
    auto future1 = task1->start();
    auto future2 = task2->start();
    
    QVariant result1 = task1->waitForResult();
    QVariant result2 = task2->waitForResult();
    
    EXPECT_EQ(result1.toInt(), 5050);
    EXPECT_EQ(result2.toInt(), 5050);
}

TEST_F(TestQTask, CancelBeforeStart) {
    auto task = std::make_shared<TestTask>();
    task->cancel();
    
    auto future = task->start();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_TRUE(task->isCancelled());
    EXPECT_TRUE(task->isFinished());
}

TEST_F(TestQTask, WaitForResultWithTimeout) {
    auto task = std::make_shared<TestTask>();
    auto future = task->start();
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    bool finished = false;
    QObject::connect(task.get(), static_cast<void(QTask::*)()>(&QTask::finished), [&]() {
        finished = true;
        loop.quit();
    });
    
    timer.start(2000);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    loop.exec();
    
    if (finished) {
        QVariant result = task->waitForResult();
        EXPECT_EQ(result.toInt(), 5050);
    } else {
        task->cancel();
        EXPECT_TRUE(task->isCancelled());
    }
}

TEST_F(TestQTask, TaskErrorSignal) {
    class ErrorTask : public QTask {
    protected:
        QVariant run() override {
            setError("Test error signal");
            return QVariant();
        }
    };
    
    auto task = std::make_shared<ErrorTask>();
    
    bool errorReceived = false;
    QObject::connect(task.get(), &QTask::taskError, [&](const QString& msg) {
        EXPECT_EQ(msg, "Test error signal");
        errorReceived = true;
    });
    
    task->start();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_TRUE(errorReceived);
}