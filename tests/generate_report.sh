#!/bin/bash

echo "==========================================="
echo "    ОТЧЕТ О ТЕСТОВОЙ ИНФРАСТРУКТУРЕ"
echo "==========================================="
echo "Время генерации: $(date)"
echo ""

./tests/run_all_tests.sh > tests/tmp/full_test_report.txt 2>&1

if grep -q "ОБЩИЙ РЕЗУЛЬТАТ: PASSED" tests/tmp/full_test_report.txt; then
    FINAL_RESULT="PASSED"
    RESULT_ICON="✅"
else
    FINAL_RESULT="FAILED" 
    RESULT_ICON="❌"
fi

echo "📊 СВОДКА РЕЗУЛЬТАТОВ ТЕСТИРОВАНИЯ"
echo "-------------------------------------------"

echo "Выполненные тесты:"
while IFS= read -r line; do
    if [[ "$line" == *"PASSED"* ]] || [[ "$line" == *"FAILED"* ]]; then
        echo "  $line"
    fi
done < tests/tmp/full_test_report.txt

echo ""
echo "📁 СТРУКТУРА ТЕСТОВОЙ ИНФРАСТРУКТУРЫ"
echo "-------------------------------------------"
echo "Скрипты тестирования:"
ls -1 tests/*.sh | while read file; do
    echo "  📄 $(basename "$file")"
done

echo ""
echo "Каталоги с тестовыми данными:"
find tests/tmp/ -type d | sort | while read dir; do
    if [ "$dir" != "tests/tmp/" ]; then
        echo "  📁 ${dir#tests/tmp/}"
    fi
done

echo ""
echo "📈 СТАТИСТИКА"
echo "-------------------------------------------"
echo "Всего тестов: $(grep -cE "✅|❌" tests/tmp/full_test_report.txt)"
echo "Успешных: $(grep -c "✅" tests/tmp/full_test_report.txt)"
echo "Неудачных: $(grep -c "❌" tests/tmp/full_test_report.txt)"

echo ""
echo "🎯 ФИНАЛЬНЫЙ РЕЗУЛЬТАТ"
echo "-------------------------------------------"
echo "$RESULT_ICON Инфраструктура тестирования: $FINAL_RESULT"
echo ""

if [ "$FINAL_RESULT" = "PASSED" ]; then
    echo "✨ Тестовая инфраструктура готова к использованию!"
    echo "Команда может приступать к разработке клиента и сервера."
else
    echo "⚠️  Есть проблемы, требующие внимания."
fi

echo ""
echo "Подробный лог: tests/tmp/full_test_report.txt"
