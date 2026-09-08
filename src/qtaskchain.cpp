#include <qtask/qtaskchain.h>
#include <QThread>
#include <QEventLoop>
#include <QDebug>
#include <QMetaMethod>

class QTaskChain::ChainTask : public QTask
{
public:
  ChainTask(StepFunc func, const QVariant& input)
 : m_func(func), m_input(input) {}

protected:
  QVariant run() override {
    if (shouldCancel()) {
      qDebug() << "ChainTask: Отменено до выполнения!";
      return QVariant();
     }

    try {
      return m_func(m_input);
     } catch (const std::exception& e) {
       setError(QString("Chain step error: %1").arg(e.what()));
       return QVariant();
      }
   }

private:
  StepFunc m_func;
  QVariant m_input;
};

void QTaskChain::start() {
  if (m_steps.empty()) {
    qWarning() << "QTaskChain: No steps defined";
    return;
   }

  m_currentStep = 0;
  m_finished = false;
  m_cancelled = false;
  m_error.clear();
  m_result = QVariant();

  executeNextStep(QVariant());
 }

QVariant QTaskChain::waitForResult() {
  // Если уже завершено или отменено, возвращаем результат
  if (m_finished) {
    if (m_cancelled) {
      throw std::runtime_error("Task chain cancelled");
     }
    if (!m_error.isEmpty()) {
      throw std::runtime_error(m_error.toStdString());
     }
    return m_result;
   }

  // Ждем завершения
  QEventLoop loop;
  QObject::connect(this, static_cast<void(QTaskChain::*)()>(&QTaskChain::finished),
    &loop, &QEventLoop::quit);
  QObject::connect(this, &QTaskChain::taskError,
    &loop, &QEventLoop::quit);
  QObject::connect(this, &QTaskChain::cancelled,
    &loop, &QEventLoop::quit);
  loop.exec();

  // Проверяем результат после ожидания
  if (m_cancelled) {
    throw std::runtime_error("Task chain cancelled");
   }

  if (!m_error.isEmpty()) {
    throw std::runtime_error(m_error.toStdString());
   }

  return m_result;
 }

void QTaskChain::cancel() {
  if (m_finished || m_cancelled) {
    return;
   }

  qDebug() << "QTaskChain: Отмена цепочки на шаге" << m_currentStep;
  m_cancelled = true;

  if (m_currentTask) {
    m_currentTask->cancel();
   }

  emit cancelled();
 }

bool QTaskChain::isFinished() const {
  return m_finished;
 }

bool QTaskChain::isCancelled() const {
  return m_cancelled;
 }

int QTaskChain::progress() const {
  if (m_steps.empty()) return 0;
  if (m_finished) return 100;
  return (m_currentStep * 100) / static_cast<int>(m_steps.size());
 }

QString QTaskChain::error() const {
  return m_error;
 }

void QTaskChain::executeNextStep(const QVariant& input) {
  if (m_cancelled) {
    if (!m_finished) {
      m_finished = true;
     // Сигнал cancelled уже отправлен в cancel()
     }
    return;
   }

  if (m_currentStep >= static_cast<int>(m_steps.size())) {
    m_finished = true;
    m_result = input;
    emit finished();
    emit finished(input);
    emit progress(100);
    return;
   }

  auto task = std::make_shared<ChainTask>(m_steps[m_currentStep], input);
  m_currentTask = task;

  QTaskChain* chain = this;
  int step = m_currentStep;
  int total = static_cast<int>(m_steps.size());

  QObject::connect(task.get(), static_cast<void(QTask::*)(const QVariant&)>(&QTask::finished),
    [chain, step, total](const QVariant& result) {
      chain->m_currentStep = step + 1;
      emit chain->stepCompleted(step + 1, total);
      emit chain->progress((chain->m_currentStep * 100) / total);
      chain->executeNextStep(result);
     });

  QObject::connect(task.get(), &QTask::taskError, [chain](const QString& error) {
    chain->m_error = error;
    chain->m_finished = true;
    emit chain->taskError(error);
   });

  QObject::connect(task.get(), &QTask::cancelled,
    [chain]() {
      if (!chain->m_cancelled) {
        chain->m_cancelled = true;
        chain->m_finished = true;
        emit chain->cancelled();
       }
     });

  task->start();
 }
