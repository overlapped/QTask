find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
# find_package(QSignalWaiter REQUIRED)
# find_package(QTask REQUIRED)

# Функция для создания примера
function(add_qtask_example NAME)
    add_executable(${NAME} ${NAME}.cpp)
    target_link_libraries(${NAME}
        PRIVATE
            QTask
#            QSignalWaiter::QSignalWaiter
            Qt6::Core
            Qt6::Concurrent
    )
    target_include_directories(${NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    # Включаем MOC для Q_OBJECT
    set_target_properties(${NAME} PROPERTIES AUTOMOC ON)
endfunction()

# Создаем все примеры
add_qtask_example(01_simple_task)
add_qtask_example(02_function_task)
# add_qtask_example(03_slot_task)
add_qtask_example(04_task_launcher)
add_qtask_example(05_progress_and_cancel)
# add_qtask_example(06_error_handling)
# add_qtask_example(07_parallel_tasks)
# add_qtask_example(08_task_with_waiter)
# add_qtask_example(09_data_processing)