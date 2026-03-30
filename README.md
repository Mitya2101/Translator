# MyLang

Проект: лексер → синтаксер → генерация POLIZ → исполнение (VM).

## Сборка (CMake)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Запуск

По умолчанию берётся `test.txt` (ищется рядом с местом запуска и на 1–2 уровня выше):

```bash
./MyLang
```

Или можно явно указать файл:

```bash
./MyLang path/to/program.txt
```


## Примечание

Исполнение идет сверху вниз
