#include <gtest/gtest.h>
#include <qtask/qtasklauncher.h>
#include <QThread>
#include <QCoreApplication>

class TestObject : public QObject {
public:
    int multiply(int a, int b) {
        QThread::msleep(50);
        return a * b;
    }
};

class TestQTaskLauncher : public ::testing::Test {
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

TEST_F(TestQTaskLauncher, RunLambda) {
    auto future = QTaskLauncher::run([]() -> QVariant {
        return 42;
    });
    
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 42);
}

TEST_F(TestQTaskLauncher, RunSlot) {
    TestObject obj;
    auto future = QTaskLauncher::runSlot(&obj, &TestObject::multiply, 5, 3);
    
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 15);
}

TEST_F(TestQTaskLauncher, RunSlotWithString) {
    // Тестируем с разными типами
    auto future = QTaskLauncher::run([]() -> QVariant {
        return QString("Hello");
    });
    
    QVariant result = future.result();
    EXPECT_EQ(result.toString(), "Hello");
}

TEST_F(TestQTaskLauncher, MultipleLaunches) {
    auto future1 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(50);
        return 1;
    });
    
    auto future2 = QTaskLauncher::run([]() -> QVariant {
        QThread::msleep(50);
        return 2;
    });
    
    QVariant result1 = future1.result();
    QVariant result2 = future2.result();
    
    EXPECT_EQ(result1.toInt(), 1);
    EXPECT_EQ(result2.toInt(), 2);
}