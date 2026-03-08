# if !defined( __libmorph_rus_prob_tools_tuner_sample_hpp__ )
# define __libmorph_rus_prob_tools_tuner_sample_hpp__
# include <cstdint>
# include <cstddef>

struct  sample
{
  unsigned  uclass;     // идентификатор класса
  size_t    ccstem;     // длина основы
  size_t    ccflex;     // длина окончания
  uint8_t   partSp;     // часть речи
  unsigned  upower;     // производительность модели
  unsigned  uoccur;     // частота появления примера
  bool      bmatch;     // позитивность или негативность этого примера
};

# endif // __libmorph_rus_prob_tools_tuner_sample_hpp__
