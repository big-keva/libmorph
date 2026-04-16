import time
import re
import pyrusmorph

def benchmark_libmorph(file_path, encoding='utf-8'):
    morph = pyrusmorph.mlmaruWc()
    fuzzy = pyrusmorph.mlfaruWc()
    
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
        return "Слов нет, какой пустой файл..."

    print(f"--- Старт обработки {total_words} слов...")

    # 3. Замер времени
    known_count = 0
    start_time = time.perf_counter()

    for word in words:
        if len(word) > 1:
            result = morph.lemmatize(word, pyrusmorph.SF_IGNORE_CAPITALS)
    
            if len(result) == 0:
                result = fuzzy.lemmatize(word, pyrusmorph.SF_IGNORE_CAPITALS)
    
            if len(result) != 0:
               known_count += 1

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
