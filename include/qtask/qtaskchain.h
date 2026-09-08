#ifndef QTASKCHAIN_H
#define QTASKCHAIN_H

#include "qtask_global.h"
#include "qtask.h"
#include <QObject>
#include <vector>
#include <memory>
#include <functional>

/**
 * @brief Цепочка последовательных задач
 * 
 * Позволяет выполнять задачи одну за другой, передавая результат
 * предыдущей задачи в следующую.
 * 
 * @code
 * // Цепочка: генерация -> обработка -> сохранение
 * auto chain = QTaskChain::create()
 *     ->then([]() -> QVariant { return generateData(); })
 *     ->then([](const QVariant& data) -> QVariant { return processData(data); })
 *     ->then([](const QVariant& processed) -> QVariant { return saveData(processed); })
 *     ->start();
 * 
 * QVariant result = chain->waitForResult();
 * @endcode
 */
class QTASK_EXPORT QTaskChain : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(QTaskChain)

public:
  using Ptr = std::shared_ptr<QTaskChain>;

  static Ptr create() {
    return Ptr(new QTaskChain());
   }

  template<typename Func>
  QTaskChain* then(Func func) {
    m_steps.push_back([func](const QVariant&) -> QVariant {
      return func();
     });
    return this;
   }

  template<typename Func>
  QTaskChain* thenWithInput(Func func) {
    m_steps.push_back(func);
    return this;
   }

  template<typename T>
  QTaskChain* thenSlot(T* obj, QVariant (T::*slot)()) {
    m_steps.push_back([obj, slot](const QVariant&) -> QVariant {
      return QVariant::fromValue((obj->*slot)());
     });
    return this;
   }

  template<typename T>
  QTaskChain* thenSlotWithInput(T* obj, QVariant (T::*slot)(const QVariant&)) {
    m_steps.push_back([obj, slot](const QVariant& input) -> QVariant {
      return QVariant::fromValue((obj->*slot)(input));
     });
    return this;
   }

  void start();
  QVariant waitForResult();
  void cancel();

  bool isFinished() const;
  bool isCancelled() const;
  int progress() const;
  QString error() const;

signals:
  void finished();
  void finished(const QVariant& result);
  void cancelled();
  void taskError(const QString& message);
  void progress(int percent);
  void stepCompleted(int step, int total);

private:
  QTaskChain() = default;

  using StepFunc = std::function<QVariant(const QVariant&)>;
  std::vector<StepFunc> m_steps;

  class ChainTask;
  std::shared_ptr<ChainTask> m_currentTask;
  int m_currentStep = 0;
  bool m_finished = false;
  bool m_cancelled = false;
  QString m_error;
  QVariant m_result;

  void executeNextStep(const QVariant& input);
};

#endif // QTASKCHAIN_H
