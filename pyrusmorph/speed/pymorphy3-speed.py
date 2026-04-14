import time
import re
import pymorphy3
from pymorphy3.units.by_lookup import DictionaryAnalyzer

def benchmark_pymorphy3(file_path, encoding='utf-8'):
    morph = pymorphy3.MorphAnalyzer(units=[[DictionaryAnalyzer()]])
    
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
        result = morph.parse(word)

        if result and not any(tag.startswith('UNKN') for tag in result[0].tag._grammemes_tuple):
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

benchmark_pymorphy3('sample.txt')
