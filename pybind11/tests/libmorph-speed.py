import time
import re
import libmorphrus

def benchmark_libmorph(file_path, encoding='utf-8'):
    morph = libmorphrus.mlmaruWc()
    
    # 1. Загрузка текста
    try:
        with open(file_path, 'r', encoding=encoding) as f:
            raw_text = f.read()
    except FileNotFoundError:
        return "Файл не найден."

    # 2. Очистка текста от пунктуации и разбивка на слова
    # Используем регулярку, чтобы оставить только слова
    words = re.findall(r'[а-яА-ЯёЁ]+', raw_text)
    total_words = len(words)
    
    if total_words == 0:
        return "В файле не найдено слов для анализа."

    print(f"--- Старт теста ---")
    print(f"Обработка {total_words} слов...")

    # 3. Замер времени
    known_count = 0
    start_time = time.perf_counter()

    for word in words:
        result = morph.lemmatize(word, libmorphrus.SF_IGNORE_CAPITALS)
        # Проверяем, опознано ли слово (есть ли оно в словаре)
        # Если score высокий и нет пометки 'UNKN', слово считается известным
        if len(result) != 0:
            known_count += 1
        else:
            print(word)

    end_time = time.perf_counter()
    
    # 4. Расчеты
    duration = end_time - start_time
    words_per_sec = total_words / duration

    # Вывод результатов
    print("-" * 30)
    print(f"Затраченное время:      {duration:.4f} сек")
    print(f"Общее количество слов:  {total_words}")
    print(f"Опознано слов:          {known_count} ({(known_count/total_words)*100:.1f}%)")
    print(f"Скорость:               {words_per_sec:.0f} слов/сек")
    print("-" * 30)

benchmark_libmorph('sample.txt')
