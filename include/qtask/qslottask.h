#ifndef QSLOTTASK_H
#define QSLOTTASK_H

#include "qtask.h"
#include <functional>

/**
 * @brief Задача для выполнения слота объекта
 */
class QTASK_EXPORT QSlotTask : public QTask
{
    Q_OBJECT

public:
    using SlotFunc = std::function<QVariant()>;
    
    /**
     * @brief Создает задачу из слота объекта
     */
    static std::shared_ptr<QSlotTask> create(SlotFunc func);
    
    /**
     * @brief Создает задачу из слота с захватом объекта
     */
    template<typename T, typename Slot, typename... Args>
    static std::shared_ptr<QSlotTask> createSlot(T* obj, Slot slot, Args... args) {
        return create([obj, slot, args...]() -> QVariant {
            return QVariant::fromValue((obj->*slot)(args...));
        });
    }
    
    /**
     * @brief Создает задачу из метода с результатом
     */
    template<typename T, typename Method, typename... Args>
    static std::shared_ptr<QSlotTask> createMethod(T* obj, Method method, Args... args) {
        return create([obj, method, args...]() -> QVariant {
            return QVariant::fromValue((obj->*method)(args...));
        });
    }

protected:
    QVariant run() override;

private:
    SlotFunc m_func;
};

#endif // QSLOTTASK_H