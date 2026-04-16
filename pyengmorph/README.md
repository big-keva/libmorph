# pyengmorph

Python bindings for libmorph::eng - morphological analyses library for English.

## Installation

```bash
pip install pyengmorph
```

## Usage

```python
import pyengmorph

imorph = pyengmorph.mlmaenWc()

lemmas = imorph.lemmatize('windows', 0)
if len(lemmas) != 0:
    for lemm in lemmas:
        print(f"  {lemm.lexid:<6d}  {lemm.lemma}")

        for gram in lemm.grams:
            print(f"    {gram.wdInfo & 0x3f}  {gram.idForm:2d}")
else:
    print("unknown word")
```

## Requirements

Python 3.8+
Linux x86_64

## License

MIT