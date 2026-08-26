#ifndef QFUNCTIONTASK_H
#define QFUNCTIONTASK_H

#include "qtask.h"
#include <functional>

/**
 * @brief Задача для выполнения произвольной функции/лямбды
 */
class QTASK_EXPORT QFunctionTask : public QTask
{
    Q_OBJECT

public:
    using Func = std::function<QVariant()>;
    
    static std::shared_ptr<QFunctionTask> create(Func func);
    
    template<typename Lambda>
    static std::shared_ptr<QFunctionTask> createLambda(Lambda&& lambda) {
        return create(std::forward<Lambda>(lambda));
    }

protected:
    QVariant run() override;

private:
    Func m_func;
};

#endif // QFUNCTIONTASK_H