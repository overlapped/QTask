#include <gtest/gtest.h>
#include <qtask/qslottask.h>
#include <QThread>
#include <QCoreApplication>

class TestObject : public QObject {
public:
    int compute(int a, int b) {
        QThread::msleep(50);
        return a + b;
    }
    
    QString process(const QString& input) {
        QThread::msleep(50);
        return input.toUpper();
    }
    
    int throwError() {
        throw std::runtime_error("Test error from slot");
        return 0;
    }
};

class TestQSlotTask : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char* argv[] = {nullptr};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }
        obj = std::make_unique<TestObject>();
    }
    
    void TearDown() override {
        if (m_app) {
            m_app.reset();
        }
    }
    
    std::unique_ptr<QCoreApplication> m_app;
    std::unique_ptr<TestObject> obj;
};

TEST_F(TestQSlotTask, MethodExecution) {
    auto task = QSlotTask::createMethod(obj.get(), &TestObject::compute, 10, 20);
    task->start();
    
    QVariant result = task->waitForResult();
    EXPECT_EQ(result.toInt(), 30);
}

TEST_F(TestQSlotTask, MethodWithString) {
    auto task = QSlotTask::createMethod(obj.get(), &TestObject::process, QString("hello"));
    task->start();
    
    QVariant result = task->waitForResult();
    EXPECT_EQ(result.toString(), "HELLO");
}

TEST_F(TestQSlotTask, Cancel) {
    auto task = QSlotTask::createMethod(obj.get(), &TestObject::compute, 10, 20);
    task->start();
    task->cancel();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_TRUE(task->isCancelled());
}

TEST_F(TestQSlotTask, SlotTask) {
    auto task = QSlotTask::createSlot(obj.get(), &TestObject::compute, 5, 5);
    task->start();
    
    QVariant result = task->waitForResult();
    EXPECT_EQ(result.toInt(), 10);
}

TEST_F(TestQSlotTask, MultipleSlots) {
    auto task1 = QSlotTask::createMethod(obj.get(), &TestObject::compute, 1, 2);
    auto task2 = QSlotTask::createMethod(obj.get(), &TestObject::compute, 3, 4);
    
    task1->start();
    task2->start();
    
    QVariant result1 = task1->waitForResult();
    QVariant result2 = task2->waitForResult();
    
    EXPECT_EQ(result1.toInt(), 3);
    EXPECT_EQ(result2.toInt(), 7);
}

TEST_F(TestQSlotTask, ErrorHandling) {
    auto task = QSlotTask::createMethod(obj.get(), &TestObject::throwError);
    task->start();
    
    EXPECT_THROW({
        task->waitForResult();
    }, std::runtime_error);
    
    EXPECT_FALSE(task->error().isEmpty());
}