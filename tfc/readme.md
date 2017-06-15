# libmorph/tfc
Компилятор таблиц окончаний / inflexion tables compiler

---------------------
## Использование
	Usage: tfc [options] inputname binaryname symbolsname  
	Options are:  
		-w            - assume source tables use 1251 Windows Cyrillic instead of 866;  
		-lang:rus/ukr - set the tables description syntax, russian or ukrainian; the detault is russian.  

## Комментарии
Внимание: настоятельно не рекомендуются игрища и прочие упражнения с кодировкой исходников! Там используется 1251,
и перекодирование приведёт к непредсказуемым результатам.  

Предполагается также, что компилятор не будет слишком экзотическим и не впадёт в построение предположений
о том, что же имелось ввиду при использовании символов верхней половины базовой кодовой страницы.
