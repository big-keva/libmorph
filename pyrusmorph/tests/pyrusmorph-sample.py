import time
import re
import pyrusmorph

partOfSpeech = [
  '?',
  'нсв',
  'нсв_нп',
  'св',
  'св_нп',
  'св-нсв',
  'св-нсв_нп',
  'м',
  'мо',
  'м//мо',
  'м//с',
  'мо',
  'мо',
  'ж',
  'жо',
  'ж//жо',
  'с',     # 16
  'со',
  'с//со',
  'м//ж',
  'мо-жо',
  'м//с',
  'мо//со',
  'ж//с',
  'мн. м//ж',
  'п',     # 25
  'п',
  'п',
  'мс-п',
  'мс',    # 29
  'мсм',   # 30
  'мсж',
  'мсс',
  'числ',  #    33
  'числ._2',
  'числ._с',
  'числ.-п',
  'и',    # 37, Имена собственные
  'им',   # 38, Имена мужского рода
  'иж',   # 39, Имена женского рода
  'ом',   # 40, Отчества муж. рода
  'ож',   # 41, Отчества жен. рода
  'ф',    # 42, Фамилии
  'г',    # 43, География
  'гм',
  'гж',
  'гс',
  'мн._от_г',  # 47

  'вводн.',
  'межд.',
  'предик.',
  'предл.',
  'союз',      # 52
  'союз_соч.',
  'част.',
  'н',
  'сокр._сущ.',
  'сокр._прил.',
  'сокр._вводн.',
  'сравн.',
  'АБ',        # 59
  'аб',
  '#1',
  '#2',
  '??'
]

morph = pyrusmorph.mlmaruWc()
fuzzy = pyrusmorph.mlfaruWc()

def print_lemmatize(word, flags = 0):
    lemmas = morph.lemmatize(word, 0)
 
    print(f"{word}")
    if len(lemmas) != 0:
        for lemm in lemmas:
            print(f"  {lemm.lexid:<6d}  {lemm.lemma}")

            for gram in lemm.grams:
                print(f"    {partOfSpeech[gram.wdInfo & 0x3f]}  {gram.idForm:2d}")
    else:
        print("unknown word")

def print_buildform(lexeme, formid):
    formlist = morph.build_form(lexeme, formid)
    print(f"{lexeme:<6d}")
    for nextform in formlist:
        print(f"  {nextform}")

def print_lemmforms(lexeme, formlist):
    print(f"{lexeme:<6d}")
    for nextform in formlist:
        print(f"  {nextform.form:<2d}  {nextform.text}")
    return 0

def print_findmatch(mask):
    morph.find_match(mask, print_lemmforms)

print("======= lemmatize() =======")
print_lemmatize('программирование')
print_lemmatize('киев')
print_lemmatize('Киев')
print_lemmatize('простой')

print("======= build_form() =======")
print_buildform(182067, 5)

print("======= find_match() =======")
print_findmatch("до?ал?д")

print_findmatch("ко*ро*ва")

def print_stematize(str, flags):
    stemlist = fuzzy.lemmatize(str, 0)
 
    print(f"{str}")
    if len(stemlist) != 0:
        for stem in stemlist:
            stemform = stem.lemma[:stem.lStem] + "|" + stem.lemma[stem.lStem:]
            print(f"  {stem.fProb:6.4}\t{stemform}\t{stem.clsId}")

            for gram in stem.grams:
                print(f"    {partOfSpeech[gram.wdInfo & 0x3f]}  {gram.idForm:2d}")
    else:
        print("unknown word")

print_stematize("командировочка", 0)

def print_stemforms(stem, clsid, idform):
    formlist = fuzzy.build_form(stem, clsid, idform)
 
    if len(formlist) != 0:
        for form in formlist:
            print(f"  {form}")
    else:
        print("unknown word")

print_stemforms("командировоч", 14, 5)

def print_samples(clsid):
    samples = fuzzy.get_sample(clsid)

    if len(samples) != 0:
        for sample in samples:
            print(f"\t{sample}");

print_samples(14)

# help(pyrusmorph)
