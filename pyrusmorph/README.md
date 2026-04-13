# pyrusmorph

Python bindings for libmorph::rus - russian morphological analyses library.

## Installation

```bash
pip install pyrusmorph
```

## Usage

```python
import pyrusmorph

imorph = pyrusmorph.mlmaruWc()

lemmas = imorph.lemmatize('простой', 0)
if len(lemmas) != 0:
    for lemm in lemmas:
        print(f"  {lemm.lexid:<6d}  {lemm.lemma}")

        for gram in lemm.grams:
            print(f"    {partOfSpeech[gram.wdInfo & 0x3f]}  {gram.idForm:2d}")
else:
    print("unknown word")
```

## Requirements

Python 3.8+
Linux x86_64

## License

MIT