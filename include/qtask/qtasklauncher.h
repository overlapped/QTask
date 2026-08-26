#ifndef QTASKLAUNCHER_H
#define QTASKLAUNCHER_H

#include "qtask_global.h"
#include <QtConcurrent>
#include <QFuture>
#include <QVariant>
#include <functional>

/**
 * @brief Утилитный класс для быстрого запуска задач
 * 
 * Использует QtConcurrent::run для максимальной простоты
 */
class QTASK_EXPORT QTaskLauncher {
public:
    /**
     * @brief Запускает функцию асинхронно
     */
    template<typename Func>
    static QFuture<QVariant> run(Func func) {
        return QtConcurrent::run([func]() -> QVariant {
            return func();
        });
    }

    /**
     * @brief Запускает слот с аргументами асинхронно
     */
    template<typename T, typename Slot, typename... Args>
    static QFuture<QVariant> runSlot(T* obj, Slot slot, Args... args) {
        return QtConcurrent::run([obj, slot, args...]() -> QVariant {
            return QVariant::fromValue((obj->*slot)(args...));
        });
    }

    /**
     * @brief Запускает слот без аргументов асинхронно
     */
    template<typename T, typename Slot>
    static QFuture<QVariant> runSlot(T* obj, Slot slot) {
        return QtConcurrent::run([obj, slot]() -> QVariant {
            return QVariant::fromValue((obj->*slot)());
        });
    }
};

#endif // QTASKLAUNCHER_H